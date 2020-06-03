#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

#include <stdint.h>
#include <time.h>
#include "vibration_analyzer.h"

#define TIME_DOMAIN_VALUE 0x1
#define FREQ_DOMAIN_VALUE 0x2

#pragma pack(push,1)
typedef struct packet_header {
    uint8_t start;
    uint16_t sensor_id;
    uint16_t data_size;
    uint8_t end;
} PACKET_HEADER_t;
#pragma pack(pop)


int sclient_Init(int dev_num);
int sclient_CleanUp(void);
int sclient_SendData(VIB_ANALYZER_t* sensor);
//int sclient_MutexLock(void);
//int sclient_MutexUnlock(void);

#endif