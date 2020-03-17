#ifndef __DATA_HANDLER_H__
#define __DATA_HANDLER_H__

#include <stdint.h>
//#include "fft.h"
#include "cord.h"

#pragma pack(push,1)
typedef struct packet_header {
    uint8_t start;
    uint16_t sensor_id;
    uint16_t data_size;

    /* data for x, y ,z*/
    uint8_t end;
} PACKET_HEADER_t;
#pragma pack(pop)


#define SAMPLES     1600    // Should be got from the client

typedef struct sensor_t {

    //FFT_t tr_data[CORD_MAX];
    double time_domain[CORD_MAX][SAMPLES];
    double freq_domain[CORD_MAX][SAMPLES];
    double freq_magnitude[CORD_MAX][(SAMPLES/2)+1]; 
} SENSOR_t;

#define STR_BUF_LEN 10
#define PACKET_SIZE ((sizeof(PACKET_HEADER_t))+(SAMPLES*(sizeof(int8_t))*STR_BUF_LEN*CORD_MAX*2)) //Size of whole packet
#define DATA_SIZE (SAMPLES*(sizeof(int8_t))*STR_BUF_LEN*CORD_MAX*2)
//int dhandler_InsertQueue(void);
int dhandler_DataExtractor(uint8_t* packet_buf);
int dhandler_Init(void);
int dhandler_CleanUp(void);

#endif