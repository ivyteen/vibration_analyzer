#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>
#include <zlib.h>
#include "data_handler.h"
//#include "fft.h"
//#include "db_inf.h"


#define TOTAL_INF_CLIENT    1
#define SENSORS_FOR_DEV     2
#define TOTAL_SENSOR_NUM    (TOTAL_INF_CLIENT*SENSORS_FOR_DEV)

static SENSOR_t* sensor = NULL;
static Bytef* data_start = NULL;


static int dhandler_InsertQueue(void){


    return 0;
}


static void* dhandler_ThreadLoop(void* arg){


    return NULL;
}

static int dhandler_CalHalfComplexMagnitude(SENSOR_t *data){
    CORD_t i;
    int j = 0;
    double real_sqr, img_sqr;

    int max_idx = (SAMPLES/2)+1;
    //bzero(data->freq_magnitude)
    for(i = CORD_X; i<CORD_MAX; i++){
        for(j = 0; j<max_idx; j++){

            real_sqr = pow(data->freq_domain[i][j],2.0);

            if((j==0)||(j==((SAMPLES)/2))){
                img_sqr = 0;
            }
            else{
                img_sqr = pow(data->freq_domain[i][SAMPLES-j],2.0);
            }

            data->freq_magnitude[i][j] = sqrt(real_sqr+img_sqr);
        }

    }


    return 0;
}

int dhandler_DataExtractor(uint8_t* packet_buf){

    uint8_t data_type;
    uint16_t sensor_num;
    PACKET_HEADER_t header;
    //DB_SET_t db_data;
    CORD_t i = 0;
    int j = 0;

    Bytef *comp_ptr=NULL;
    uint8_t *data_ptr = NULL;
    uLongf data_size;
    uint16_t comp_size;

    char tmp[STR_BUF_LEN] = {0,};
    int ret = 0;

    memcpy(&comp_size,(packet_buf+3),sizeof(uint16_t));
    comp_size = ntohs(comp_size);

    comp_ptr = (packet_buf+(sizeof(PACKET_HEADER_t)-sizeof(header.end)));

    bzero(data_start,DATA_SIZE);
    ret = uncompress(data_start,&data_size,comp_ptr, comp_size);
    if(ret != Z_OK){
        printf("Uncompress Error : %d\n",ret);
        return -1;
    }

//    printf("Uncompressed : %d\n",data_size);



    bzero(sensor,sizeof(SENSOR_t));

    memcpy(&sensor_num,(packet_buf+1),sizeof(uint16_t));
    sensor_num = ntohs(sensor_num);
  
    data_ptr = data_start;
    //Convert string to double for time domain
    for(i=CORD_X; i<CORD_MAX; i++){
        for(j=0; j<SAMPLES; j++){
            //sprintf((char*)buf_idx,"%.6f",sensor->tr_data[i].time_domain_data[j]);
            bzero(tmp, sizeof(tmp));
            memcpy(tmp,data_ptr,STR_BUF_LEN);
            sensor->time_domain[i][j] = atof(tmp);
            data_ptr += STR_BUF_LEN;
        }

    }

    for(i=CORD_X; i<CORD_MAX; i++){
        for(j=0; j<SAMPLES; j++){
            //sprintf((char*)buf_idx,"%.6f",sensor->tr_data[i].time_domain_data[j]);
            bzero(tmp, sizeof(tmp));
            memcpy(tmp,data_ptr,STR_BUF_LEN);
            sensor->freq_domain[i][j] = atof(tmp);
            data_ptr += STR_BUF_LEN;
        }
    }

    dhandler_CalHalfComplexMagnitude(sensor);

    printf("[FM]:%d %.6f %.6f %.6f\n",sensor_num,
        sensor->freq_magnitude[CORD_X][0],
        sensor->freq_magnitude[CORD_Y][0],
        sensor->freq_magnitude[CORD_Z][0]);
        

    //return dbinf_InsertData(&db_data);

    return 0;
}


int dhandler_Init(void){

	sensor = (SENSOR_t*)malloc(sizeof(SENSOR_t));
    data_start = (Bytef*)malloc(DATA_SIZE);

    return 0;
}

int dhandler_CleanUp(void){

    free(sensor);
    free(data_start);
    return 0;
}
