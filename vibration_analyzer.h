#ifndef __VIBRATION_ANALYZER_H__
#define __VIBRATION_ANALYZER_H__

#include "fft.h"
#include "adxl345.h"



typedef enum {
    CORD_X = 0,
    CORD_Y,
    CORD_Z,
    CORD_MAX
} CORD_t;


typedef struct vib_analyzer_t {

    SENSOR_DEV_t dev;
    FFT_t tr_data[CORD_MAX];

} VIB_ANALYZER_t;

#endif