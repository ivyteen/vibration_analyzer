#ifndef __DB_INF_H__
#define __DB_INF_H__

#include "cord.h"
#include "data_handler.h"


#define TOTAL_INF_CLIENT    1
#define SENSORS_FOR_DEV     2
#define TOTAL_SENSOR_NUM    (TOTAL_INF_CLIENT*SENSORS_FOR_DEV)


typedef struct db_sensor_info {
    time_t sql_save_time;
    int sql_save_cnt;
    int vrms_save_flag;
    float vrms_avg[CORD_MAX];
} DB_SENSOR_INFO_t;

int dbinf_InsertData(SENSOR_t *sensor_data);
int dbinf_Init(void);
int dbinf_CleanUp(void);
//int dbinf_CreateTable(int sensor_num);
//int dbinf_CheckTableExist(int sensor_num);
#endif // __DB_INF_H__