#ifndef __FFT_H__
#define __FFT_H__

#include <fftw3.h>

typedef struct fft {
    double* time_domain_data;
    double* freq_domain_data;
    double* freq_domain_magnitude; //Sharing with time_domain_data for lower memory consumption
    fftw_plan tr_plan;
    int samples;
} FFT_t;

extern int fft_CalHalfComplexMagnitude(FFT_t* data);
extern int fft_Transform(FFT_t* data);
//extern int fft_inverse_transform(FFT_t* data);
extern int fft_Setup(FFT_t* data);
extern void fft_Destroy(FFT_t* data);

#endif //__FFT_H__