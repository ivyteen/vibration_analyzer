#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define SENSOR_DATA

#define DATA_FORMAT   0x31  // data format register address
#define DATA_FORMAT_B 0x0B  // data format bytes: +/- 16g range, 13-bit resolution (p. 26 of ADXL345 datasheet)
#define READ_BIT      0x80
#define MULTI_BIT     0x40
#define BW_RATE       0x2C
#define POWER_CTL     0x2D
#define DATAX0        0x32

//static const char codeVersion[3] = "0.2";  // code version number
//static const int timeDefault = 5;  // default duration of data stream, seconds
//static const int freqDefault = 5;  // default sampling rate of data stream, Hz
//static const int freqMax = 3200;  // maximal allowed cmdline arg sampling rate of data stream, Hz
static const int speedSPI = 2000000;  // SPI communication speed, bps
static const int freqMaxSPI = 100000;  // maximal possible communication sampling rate through SPI, Hz (assumption)
//static const int coldStartSamples = 2;  // number of samples to be read before outputting data to console (cold start delays)
//static const double coldStartDelay = 0.1;  // time delay between cold start reads
static const double accConversion = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution
static const double tStatusReport = 1;  // time period of status report if data read to file, seconds


static double vTime;
static double vFreq;

static double *rt, *rx, *ry, *rz; // Buffer for Raw Data
static double *ax, *ay, *az; //*at Buffer for Output 


static pthread_t vib_sensor_th;


static int bSave = 0;
static char vSave[256] = "";


// SPI sensor setup
static int samples;
static int samplesMaxSPI;
static int success = 1;
static int h, bytes;
//static char data[7];
static int16_t x, y, z;
static double tStart, tDuration, t;

static double delay;

static int vibSensor_ReadBytes(int handle, char *data, int count) {
    data[0] |= READ_BIT;
    if (count > 1) data[0] |= MULTI_BIT;
    return spiXfer(handle, data, data, count);
}

static int vibSensor_WriteBytes(int handle, char *data, int count) {
    if (count > 1) data[0] |= MULTI_BIT;
    return spiWrite(handle, data, count);
}

int vibSensor_ReadData(void){

    int statusReportedTimes = 0;
    double tCurrent, tClosest, tError, tErrorPrev, tLeft;
    int j, jClosest;
    int samplesRead;
    char data[7];

    int i;

    size_t buf_size;

    buf_size = samplesMaxSPI * sizeof(double);
    bzero(rx,buf_size);
    bzero(ry,buf_size);
    bzero(rz,buf_size);

    buf_size = samples * sizeof(double);
    //bzero(at,buf_size);
    bzero(ax,buf_size);
    bzero(ay,buf_size);
    bzero(az,buf_size);

    tStart = time_time();

#ifdef SENSOR_DATA
    for (i = 0; i < samplesMaxSPI; i++) {
        data[0] = DATAX0;
        bytes = vibSensor_ReadBytes(h, data, 7);
        if (bytes == 7) {
            x = (data[2]<<8)|data[1];
            y = (data[4]<<8)|data[3];
            z = (data[6]<<8)|data[5];
            t = time_time();
            rx[i] = x * accConversion;
            ry[i] = y * accConversion;
            rz[i] = z * accConversion;
            rt[i] = t - tStart;
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
            tLeft = vTime - tDuration;
            if (tLeft < 0) {
                tLeft = 0.0;
            }
           // printf("%.2f seconds left...\n", tLeft);
        }
        if (tDuration > vTime) {  // enough data read
            samplesRead = i;
            break;
        }
    }
#endif

    
//    printf("Writing to the output file...\n");
#ifdef SENSOR_DATA
    printf("Sampling Sensor\n");
#else
    printf("Sine Wave\n");
#endif

    for (i = 0; i < samples; i++) {
#ifdef SENSOR_DATA        
        if (i == 0) {  // always get the first reading from position 0
            tCurrent = 0.0;
            jClosest = 0;
            tClosest = rt[jClosest];
        }
        else {
            tCurrent = (float)i * delay;
            tError = fabs(tClosest - tCurrent);
            tErrorPrev = tError;
            for (j = jClosest; j < samplesRead; j++) {  // lookup starting from previous j value
                if (fabs(rt[j] - tCurrent) < tError) {  // in order to save some iterations
                    jClosest = j;
                    tClosest = rt[jClosest];
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
        ax[i] = rx[jClosest];
        ay[i] = ry[jClosest];
        az[i] = rz[jClosest];

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



int vibSensor_Init(double* outbuf_x, double* outbuf_y, double* outbuf_z, int sample_rate, int sample_time){

    char data[7];

    vTime = sample_time;
    vFreq = sample_rate;
    
   // SPI sensor setup
    samples = vFreq * vTime;
    samplesMaxSPI = freqMaxSPI * vTime;

    rx = malloc(samplesMaxSPI * sizeof(double));
    ry = malloc(samplesMaxSPI * sizeof(double));
    rz = malloc(samplesMaxSPI * sizeof(double));
    rt = malloc(samplesMaxSPI * sizeof(double));


    if((rt==NULL)||(rx==NULL)||(ry==NULL)||(rz==NULL)){
        perror("Raw Buffer Allocation");
        exit(1);
    }
 //   printf("%s - %08x %08x %08x %08x\n",__FUNCTION__,rx,ry,rz,rt);

    if (gpioInitialise() < 0) {
        perror("Failed to initialize GPIO!");
        exit(1);
    }

    // SPI Open and Init Sensor
    h = spiOpen(0, speedSPI, 3);
    //h = spiOpen(1, speedSPI, 3);

    data[0] = BW_RATE;
    data[1] = 0x0E;
    vibSensor_WriteBytes(h, data, 2);
    data[0] = DATA_FORMAT;
    data[1] = DATA_FORMAT_B;
    vibSensor_WriteBytes(h, data, 2);
    data[0] = POWER_CTL;
    data[1] = 0x08;
    vibSensor_WriteBytes(h, data, 2);

    delay = 1.0 / vFreq;  // delay between reads in seconds

    ax = outbuf_x;
    ay = outbuf_y;
    az = outbuf_z;

    return 0;
}




void vibSensor_CleanUp(void){

    spiClose(h);
    gpioTerminate();

    free(rt);
    free(rx);
    free(ry);
    free(rz);

    return;
}