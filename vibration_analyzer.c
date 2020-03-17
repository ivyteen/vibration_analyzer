#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <zlib.h>

#include "vibration_analyzer.h"
#include "socket_client.h"

#define TOTAL_SENSOR_NUM    2
VIB_ANALYZER_t* analyzer = NULL;

pthread_t thread[TOTAL_SENSOR_NUM];

void* analyzer_SubThread(void* arg){

	int i;
	CORD_t j;
	pthread_t thread_id;

	VIB_ANALYZER_t* az = (VIB_ANALYZER_t*)arg;
	FFT_t* fft_data = az->tr_data;
	SENSOR_DEV_t* dev = &(az->dev);
	int samples = dev->samples;

	thread_id = pthread_self();

	while(1){

		vibSensor_ReadData(dev);

		for(j=CORD_X; j<CORD_MAX; j++){
			fft_Transform(&fft_data[j]);
			//fft_CalHalfComplexMagnitude(&fft_data[j]);
		}

		//sclient_MutexLock();
		sclient_SendData(az);
		//usleep(10000);
		//sclient_SendData(az, FREQ_DOMAIN_VALUE);
		//sclient_MutexUnlock();
	}

	for(j=CORD_X; j<CORD_MAX; j++){
		fft_Destroy(&fft_data[j]);
	}

	vibSensor_SpiCleanUp(dev);

	return NULL;
}

int main(void){

    int i;
	int samples;
	CORD_t j;
	FFT_t* fft_data;
	SENSOR_DEV_t* dev;

	
	analyzer = (VIB_ANALYZER_t*)malloc(sizeof(VIB_ANALYZER_t)*TOTAL_SENSOR_NUM);

	// Config for sensor data
	analyzer[0].dev.bus_num = 0;
	analyzer[0].dev.cs_num = 0;
	analyzer[0].dev.sample_rate = 1600;
	analyzer[0].dev.sample_time = 1;

	analyzer[1].dev.bus_num = 0;
	analyzer[1].dev.cs_num = 1;
	analyzer[1].dev.sample_rate = 1600;
	analyzer[1].dev.sample_time = 1;

	sclient_Init();

	vibSensor_PiGpioInit();

	for(i=0; i<TOTAL_SENSOR_NUM; i++ ){

		dev = &analyzer[i].dev;
		fft_data = analyzer[i].tr_data;

		samples = dev->sample_rate * dev->sample_time;
		for(j=CORD_X; j<CORD_MAX; j++){
			fft_data[j].samples = samples;
			fft_Setup(&fft_data[j]);
		}

		// here - device output buffer to fft time domain buffer 
		dev->outbuf_x = fft_data[CORD_X].time_domain_data;
		dev->outbuf_y = fft_data[CORD_Y].time_domain_data;
		dev->outbuf_z = fft_data[CORD_Z].time_domain_data;

		vibSensor_SpiInit(dev);
		
	}

	pthread_create(&thread[0],NULL,analyzer_SubThread,(void*)&analyzer[0]);
	pthread_create(&thread[1],NULL,analyzer_SubThread,(void*)&analyzer[1]);

	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);

	//analyzer_SubThread(&analyzer[0]);

	vibSensor_PiGpioCleanUp();
	sclient_CleanUp();

    return 0;
}

