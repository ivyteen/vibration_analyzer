#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "adxl345.h"

#define SENSOR_DATA



#if 0
static const char codeVersion[3] = "0.2";  // code version number
static const int timeDefault = 5;  // default duration of data stream, seconds
static const int freqDefault = 5;  // default sampling rate of data stream, Hz
static const int freqMax = 3200;  // maximal allowed cmdline arg sampling rate of data stream, Hz
static const int coldStartSamples = 2;  // number of samples to be read before outputting data to console (cold start delays)
static const double coldStartDelay = 0.1;  // time delay between cold start reads
#endif


#define CAL_OFFSET(val)  (((val) < 0)?((char)(((abs(val))*0.0039)/0.0156)):(-((char)(((abs(val))*0.0039)/0.0156))))

static const int speedSPI = 2000000;  // SPI communication speed, bps
static const int freqMaxSPI = 100000;  // maximal possible communication sampling rate through SPI, Hz (assumption)
static const double accConversion = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution
static const double tStatusReport = 1;  // time period of status report if data read to file, seconds



// SPI sensor setup
static int pigpio_ver = 0;

static int vibSensor_ReadBytes(int handle, char *data, int count) {
    data[0] |= READ_BIT;
    if (count > 1) data[0] |= MULTI_BIT;
    return spiXfer(handle, data, data, count);
}

static int vibSensor_WriteBytes(int handle, char *data, int count) {
    if (count > 1) data[0] |= MULTI_BIT;
    return spiWrite(handle, data, count);
}

int vibSensor_ReadData(SENSOR_DEV_t* dev){

    int statusReportedTimes = 0;
    double tCurrent, tClosest, tError, tErrorPrev, tLeft;
    int j, jClosest;
    int samplesRead;
    char data[7];

    int bytes;
    int16_t x, y, z;
    double tStart, tDuration, t;

    int i;

    size_t buf_size;

    buf_size = dev->samples_max_spi * sizeof(double);
    bzero(dev->rx,buf_size);
    bzero(dev->ry,buf_size);
    bzero(dev->rz,buf_size);
    bzero(dev->rt,buf_size);

    buf_size = dev->samples * sizeof(double);
    //bzero(at,buf_size);
    bzero(dev->outbuf_x,buf_size);
    bzero(dev->outbuf_y,buf_size);
    bzero(dev->outbuf_z,buf_size);

    tStart = time_time();

#ifdef SENSOR_DATA
    for (i = 0; i < dev->samples_max_spi; i++) {
        data[0] = DATAX0;
        bytes = vibSensor_ReadBytes(dev->handler, data, 7);
        if (bytes == 7) {
            x = (data[2]<<8)|data[1];
            y = (data[4]<<8)|data[3];
            z = (data[6]<<8)|data[5];
            t = time_time();
            //printf("%d %d %d\n",x,y,z);
            dev->rx[i] = x * accConversion;
            dev->ry[i] = y * accConversion;
            dev->rz[i] = z * accConversion;
            dev->rt[i] = t - tStart;

            //printf("%02f %02f %02f\n",dev->rx[i],dev->ry[i],dev->rz[i]);
        }
        else {
            printf("Bytes read : %d\n", bytes);
            gpioTerminate();
            printf("Error occurred!");
            exit(1);
        }
        tDuration = t - tStart;
        if (tDuration > tStatusReport * ((float)statusReportedTimes + 1.0)) {
            statusReportedTimes++;
            tLeft = dev->sample_time - tDuration;
            if (tLeft < 0) {
                tLeft = 0.0;
            }
           // printf("%.2f seconds left...\n", tLeft);
        }
        if (tDuration > dev->sample_time) {  // enough data read
            samplesRead = i;
            break;
        }
    }
#endif

    
//    printf("Writing to the output file...\n");
#ifdef SENSOR_DATA
    //printf("Sampling Sensor\n");
#else
    printf("Sine Wave\n");
#endif

    for (i = 0; i < dev->samples; i++) {
#ifdef SENSOR_DATA        
        if (i == 0) {  // always get the first reading from position 0
            tCurrent = 0.0;
            jClosest = 0;
            tClosest = dev->rt[jClosest];
        }
        else {
            tCurrent = (float)i * dev->delay;
            tError = fabs(tClosest - tCurrent);
            tErrorPrev = tError;
            for (j = jClosest; j < samplesRead; j++) {  // lookup starting from previous j value
                if (fabs(dev->rt[j] - tCurrent) < tError) {  // in order to save some iterations
                    jClosest = j;
                    tClosest = dev->rt[jClosest];
                    tErrorPrev = tError;
                    tError = fabs(tClosest - tCurrent);
                }
                else {
                    if (tError > tErrorPrev) {  // if the error starts growing
	                	break;                  // break the loop since the minimal error point passed
                    }
                }
            }  // when this loop is ended, jClosest and tClosest keep coordinates of the closest (j, t) point
        }
        dev->outbuf_x[i] = dev->rx[jClosest];
        dev->outbuf_y[i] = dev->ry[jClosest];
        dev->outbuf_z[i] = dev->rz[jClosest];

        //at[i] = tCurrent;
#else
/*
        ax[i] =(2*sin((double)i/(double)samples*2.0*M_PI*2)) + sin((double)i/(double)samples*2.0*M_PI*4)+ sin((double)i/(double)samples*2.0*M_PI*6)+ 3;
        ay[i] =(3*sin((double)i/(double)samples*2.0*M_PI*3)) + sin((double)i/(double)samples*2.0*M_PI*6)+ sin((double)i/(double)samples*2.0*M_PI*9);
        az[i] =(4*sin((double)i/(double)samples*2.0*M_PI*5)) + sin((double)i/(double)samples*2.0*M_PI*2)+ sin((double)i/(double)samples*2.0*M_PI*7);
*/

        ax[i] =sin((double)i/(double)samples*2.0*M_PI)+1;
        ay[i] =sin((double)i/(double)samples*2.0*M_PI)+2;
        az[i] =sin((double)i/(double)samples*2.0*M_PI)+3;

#endif
    }


    return 0;
}

static int vibSensor_SensorCalibration(SENSOR_DEV_t* dev){

    int i;
    char data[7];
    int bytes;
    int16_t x,y,z;
    int16_t *x_arr, *y_arr, *z_arr;

	int x_sum=0, y_sum=0, z_sum=0;
	int x_avg=0, y_avg=0, z_avg=0;
	/*
    int x_dev, y_dev, z_dev;

    x_arr = (int16_t*)malloc(sizeof(int16_t)*dev->samples_max_spi);
    y_arr = (int16_t*)malloc(sizeof(int16_t)*dev->samples_max_spi);
    z_arr = (int16_t*)malloc(sizeof(int16_t)*dev->samples_max_spi);
*/


    for (i = 0; i < dev->samples_max_spi; i++) {
        data[0] = DATAX0;
        bytes = vibSensor_ReadBytes(dev->handler, data, 7);
        if (bytes == 7) {
            x = (data[2]<<8)|data[1];
            //x_arr[i] = x;
            //x_sum += x_arr[i];
            x_sum += x;

            y = (data[4]<<8)|data[3];
            //y_arr[i] = y;
            //y_sum += y_arr[i];
            y_sum += y;


            z = (data[6]<<8)|data[5];
            //z_arr[i] = z;
            //z_sum += z_arr[i];
            z_sum += z;


            //printf("%02f %02f %02f\n",dev->rx[i],dev->ry[i],dev->rz[i]);
        }
        else {
            printf("[%s] Bytes read : %d\n",__FUNCTION__,bytes);
            gpioTerminate();
            printf("[%s] Error occurred!\n",__FUNCTION__);
            exit(1);
        }

    }

    /* Get Avg */
	x_avg = (int)(x_sum / dev->samples_max_spi);
	y_avg = (int)(y_sum / dev->samples_max_spi);
	z_avg = (int)(z_sum / dev->samples_max_spi);
/*
	for(i = 0; i<dev->samples_max_spi; i++){
		x_sum += pow(x_avg-x_arr[i],2);
		y_sum += pow(y_avg-y_arr[i],2);
		z_sum += pow(z_avg-z_arr[i],2);
	}

	x_dev = (int)sqrt(x_sum/dev->samples_max_spi);
	y_dev = (int)sqrt(y_sum/dev->samples_max_spi);
	z_dev = (int)sqrt(z_sum/dev->samples_max_spi);
*/

    data[0] = OFS_X;
    data[1] = CAL_OFFSET(x_avg);
    printf("x offset : %d %X\n",x_avg,data[1]);
    vibSensor_WriteBytes(dev->handler, data, 2);

    data[0] = OFS_Y;
    data[1] = CAL_OFFSET(y_avg);
    printf("y offset : %d %X\n",y_avg,data[1]);

    vibSensor_WriteBytes(dev->handler, data, 2);

    data[0] = OFS_Z;
    data[1] = CAL_OFFSET(z_avg);
    printf("z offset : %d %X\n",z_avg,data[1]);

    vibSensor_WriteBytes(dev->handler, data, 2);


    //free(x_arr);
    //free(y_arr);
    //free(z_arr);

    return 0;
}


int vibSensor_PiGpioInit(void){

    int i=0;
    int ret = 0;
    for(i = 0; i<3; i++){
        if((ret=gpioInitialise()) < 0)
        {
            gpioTerminate();
            printf("GPIO Init retry");
            usleep(100000);
            //exit(EXIT_FAILURE);
        }
        else
        {
            break;
        }
        
    }

    if(ret < 0){
        printf("Failed to init GPIO\n");
        exit(EXIT_FAILURE);
    }


    return 0;
}

int vibSensor_SpiInit(SENSOR_DEV_t* dev)
{
    //SENSOR_DEV_t* dev = &vib_analyzer->dev;

    double vTime;
    double vFreq;
    int samplesMaxSPI;

    char data[7];

    vTime = dev->sample_time;
    vFreq = dev->sample_rate;
    samplesMaxSPI = freqMaxSPI * vTime;
   
   // SPI sensor setup
    dev->samples = vFreq * vTime;
    dev->samples_max_spi = samplesMaxSPI;

    dev->rx = malloc(samplesMaxSPI * sizeof(double));
    dev->ry = malloc(samplesMaxSPI * sizeof(double));
    dev->rz = malloc(samplesMaxSPI * sizeof(double));
    dev->rt = malloc(samplesMaxSPI * sizeof(double));


    if((dev->rt==NULL)||(dev->rx==NULL)||(dev->ry==NULL)||(dev->rz==NULL)){
        perror("Raw Buffer Allocation");
        exit(1);
    }
 //   printf("%s - %08x %08x %08x %08x\n",__FUNCTION__,rx,ry,rz,rt);

    // SPI Open and Init Sensor
    if(dev->bus_num == 0)
        dev->handler = spiOpen(dev->cs_num, speedSPI,PI_SPI_FLAGS_MODE(3));
    else
    {
        dev->handler = spiOpen(dev->cs_num, speedSPI,PI_SPI_FLAGS_AUX_SPI(1));       
    }
    
    //h = spiOpen(1, speedSPI, 3);

    data[0] = BW_RATE;
    data[1] = 0x0E;
    vibSensor_WriteBytes(dev->handler, data, 2);
    data[0] = DATA_FORMAT;
    data[1] = DATA_FORMAT_B;
    vibSensor_WriteBytes(dev->handler, data, 2);
    data[0] = POWER_CTL;
    data[1] = 0x08;
    vibSensor_WriteBytes(dev->handler, data, 2);

    data[0] = OFS_X;
    data[1] = 0;
    vibSensor_WriteBytes(dev->handler, data, 2);

    data[0] = OFS_Y;
    data[1] = 0;
    vibSensor_WriteBytes(dev->handler, data, 2);

    data[0] = OFS_Z;
    data[1] = 0;
    vibSensor_WriteBytes(dev->handler, data, 2);


    vibSensor_SensorCalibration(dev);


    dev->delay = 1.0 / vFreq;  // delay between reads in seconds

    return 0;
}




void vibSensor_PiGpioCleanUp(void){

    gpioTerminate();

    return;
}

void vibSensor_SpiCleanUp(SENSOR_DEV_t* dev){

    spiClose(dev->handler);

    free(dev->rt);
    free(dev->rx);
    free(dev->ry);
    free(dev->rz);

    return;
}