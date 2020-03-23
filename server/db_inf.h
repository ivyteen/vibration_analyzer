#ifndef __DB_INF_H__
#define __DB_INF_H__

#include "cord.h"
#include "data_handler.h"




int dbinf_InsertData(SENSOR_t *sensor_data);
int dbinf_Init(void);
int dbinf_CleanUp(void);
//int dbinf_CreateTable(int sensor_num);
//int dbinf_CheckTableExist(int sensor_num);
#endif // __DB_INF_H__