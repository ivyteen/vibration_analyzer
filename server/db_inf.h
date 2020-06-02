#ifndef __DB_INF_H__
#define __DB_INF_H__

#include "cord.h"
#include "data_handler.h"


#define TOTAL_INF_CLIENT    1
#define SENSORS_FOR_DEV     2
#define TOTAL_SENSOR_NUM    (TOTAL_INF_CLIENT*SENSORS_FOR_DEV)


typedef struct db_sensor_info {
    unsigned char table_create;
    time_t last_sql_save_time;
    int sql_save_cnt;
    unsigned char alarm_criteria_set_flag;
    float alarm_criteria[CORD_MAX];
} DB_SENSOR_INFO_t;

int dbinf_InsertData(SENSOR_t *sensor_data);
int dbinf_Init(void);
int dbinf_CleanUp(void);
//int dbinf_CreateTable(int sensor_num);
//int dbinf_CheckTableExist(int sensor_num);
#endif // __DB_INF_H__