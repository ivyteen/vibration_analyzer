#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "adxl345.h"
#include "fft.h"

FFT_t data_x, data_y, data_z;
const int sample_rate = 1600;
const int sample_time = 1; 
int samples = sample_rate * sample_time;

int main(void){

    int i = 0;

    fft_Setup(&data_x,samples);
	fft_Setup(&data_y,samples);
	fft_Setup(&data_z,samples);

	vibSensor_Init(
		data_x.time_domain_data,
		data_y.time_domain_data,
		data_z.time_domain_data,
		sample_rate,
		sample_time
	);

	while(1){
		vibSensor_ReadData();

		fft_Transform(&data_x, samples);
		fft_Transform(&data_y, samples);
		fft_Transform(&data_z, samples);

		fft_CalHalfComplexMagnitude(&data_x, samples);
		fft_CalHalfComplexMagnitude(&data_y, samples);
		fft_CalHalfComplexMagnitude(&data_z, samples);

	#if 0
	/*
		printf("FFT Result\n");
		for(i=0; i<(samples); ++i){

			if((i <= 50)||(i >= 1550))
				printf("%d : %1.7f %1.7f %1.7f \n", i,data_x.freq_domain_data[i], data_y.freq_domain_data[i], data_z.freq_domain_data[i]);
		}
	*/
		printf("Magnidute\n");
		for(i=0; i<(samples/2)+1; ++i){
			printf("%d : %1.7f %1.7f %1.7f \n", i,data_x.freq_domain_magnitude[i], data_y.freq_domain_magnitude[i], data_z.freq_domain_magnitude[i]);
		}
	#else
		//printf("Magnidute\n");
		for(i=0; i<(samples/2)+1; ++i){
			if((data_x.freq_domain_magnitude[i] > 0.01)||(data_y.freq_domain_magnitude[i] > 0.01)||(data_z.freq_domain_magnitude[i] > 0.01))
				printf("%d : %1.7f %1.7f %1.7f \n", i,data_x.freq_domain_magnitude[i], data_y.freq_domain_magnitude[i], data_z.freq_domain_magnitude[i]);
		}		
	#endif

	}

    fft_Destroy(&data_x);
    fft_Destroy(&data_y);
    fft_Destroy(&data_z);

	vibSensor_CleanUp();

    return 0;
}

