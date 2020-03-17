#ifndef __DB_INF_H__
#define __DB_INF_H__

//#include <time.h>
#include "cord.h"
//#include "fft.h"

typedef struct db_set {
    int sensor_num;
    //struct tm *timeinfo;
   //FFT_t* fft_data[CORD_MAX];
} DB_SET_t;


int dbinf_InsertData(DB_SET_t *data);
int dbinf_Init(void);
int dbinf_CleanUp(void);

#endif // __DB_INF_H__