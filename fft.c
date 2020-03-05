// FFT for real data

#include <stdio.h>
#include <math.h>
#include <complex.h>

//#include <fftw3.h>

#include <stdlib.h>
#include <string.h>
#include "fft.h"

static int samples = 0;

// Calculate the magnitude for the result of half-complex format DFT
int fft_CalHalfComplexMagnitude(FFT_t* data){
	int i = 0;
	double real_sqr, img_sqr;
	//bzero(samples * sizeof(data->time_domain_data[0]));
	bzero(data->time_domain_data,data->samples * sizeof(double));
	data->freq_domain_magnitude = data->time_domain_data;

	for(i = 0; i<(((data->samples)/2)+1); i++){

		real_sqr = pow(data->freq_domain_data[i],2.0); 

		if((i==0)||(i==((data->samples)/2))){
			img_sqr = 0; 
		}
		else{
			img_sqr = pow(data->freq_domain_data[(data->samples)-i],2.0);
		}
		data->freq_domain_magnitude[i] = sqrt(real_sqr+img_sqr);

	}

	return 0;
} 

int fft_Transform(FFT_t* data){

	int i = 0;
	// first, clear ther buffers
	fftw_execute(data->tr_plan);

	for(i = 0; i<(data->samples); i++)
		data->freq_domain_data[i] = (data->freq_domain_data[i]/(data->samples)); // Normalised by the number of samples

	return 0;
}

int fft_Setup(FFT_t* data){

	data->time_domain_data = fftw_malloc(sizeof(double)*(data->samples));	
	data->freq_domain_data = fftw_malloc(sizeof(double)*(data->samples));

	// Allocate memory for transform
	if((data->freq_domain_data == NULL)||(data->time_domain_data == NULL)){
		perror("Failed to Allocate buf");
		exit(1);
	}	
	// Set Plan
	//printf("%s - %08X , %08X\n",__FUNCTION__,&data->time_domain_data,&data->freq_domain_data);

	data->tr_plan = fftw_plan_r2r_1d(data->samples, data->time_domain_data, data->freq_domain_data, /*FFTW_DHT*/ FFTW_R2HC, FFTW_ESTIMATE);
	if(data->tr_plan < 0){
		perror("Failed to set plan");
		exit(1);
	}

	//printf("%s OK\n",__FUNCTION__);

	return 0;
}

void fft_Destroy(FFT_t* data){
	
	fftw_destroy_plan(data->tr_plan);

	fftw_free(data->time_domain_data);
	fftw_free(data->freq_domain_data);

	return;
}

#ifdef FFT_Main
int main(void){

	int n = 128;
#if 0	
	fftw_complex* b1 = fftw_malloc(sizeof(fftw_complex)*n);
	fftw_complex* b2 = fftw_malloc(sizeof(fftw_complex)*n);

	fftw_plan p1 = fftw_plan_dft_1d(n, b1, b2, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_plan p2 = fftw_plan_dft_1d(n, b2, b1, FFTW_BACKWARD, FFTW_ESTIMATE);
#else
	double* b1 = fftw_malloc(sizeof(double)*n);
	double* b2 = fftw_malloc(sizeof(double)*n);

	fftw_plan p1 = fftw_plan_r2r_1d(n, b1, b2, FFTW_R2HC, FFTW_ESTIMATE);
	fftw_plan p2 = fftw_plan_r2r_1d(n, b2, b1, FFTW_HC2R, FFTW_ESTIMATE);


#endif

	int i;
	printf("Sine Wave\n");
	for(i=0; i<n; ++i){
		b1[i] =(4*sin((double)i/(double)n*2.0*M_PI*5)) + sin((double)i/(double)n*2.0*M_PI*2)+ sin((double)i/(double)n*2.0*M_PI*7);
		printf("%d : %1.7f\n",i,(double)b1[i]);

	}

//	printf("\n\n\n");
#if 0
	fftw_execute(p1);

	printf("FFT Result\n");
	for(i=0; i<n; ++i){
		printf("%d : %1.7f\t%1.7f\n", i,creal(b2[i]), cimag(b2[i]));
	}

	fftw_execute(p2);
	printf("\n");

	printf("IFFT Result\n");
	for(i=0; i<n; ++i){
		printf("%d : %1.7f\t%1.7f\n", i,creal(b1[i]/n), cimag(b1[i]));
	}

#else
	fftw_execute(p1);

	printf("FFT Result\n");
	for(i=0; i<n; ++i){
		printf("%d : %1.7f\n", i,b2[i]);
	}

	fftw_execute(p2);
	printf("\n");

	printf("IFFT Result\n");
	for(i=0; i<n; ++i){
		printf("%d : %1.7f\n", i, b1[i]/n);
	}


#endif

	fftw_destroy_plan(p1);
	fftw_destroy_plan(p2);

	fftw_free(b1);
	fftw_free(b2);

	return 0;

}
#endif //FFT_MAIN