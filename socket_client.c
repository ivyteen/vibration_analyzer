#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <zlib.h>

#include "vibration_analyzer.h"
#include "socket_client.h"

#define UDP_PORT 5100
#define NET_ADDR "192.168.1.201"
//#define NET_ADDR "192.168.1.12"
#define STR_BUF_LEN 10


static int sockfd = 0;
static struct sockaddr_in servaddr;

pthread_mutex_t mid;

int sclient_Init(void){

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        printf("Create Socket Failed\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(NET_ADDR);
    servaddr.sin_port = htons(UDP_PORT);


    pthread_mutex_init(&mid,NULL);

    return 0;
}

int sclient_CleanUp(void){

    close(sockfd);

    return 0;
}

#if 0
int sclient_CompressData(void* data, size_t data_size){

    // compress serialized data with zlib



    return 0;
}


static int sclient_MakePacket(uint8_t data_type, double* data_arr){



    return 0;
}

int sclient_MutexLock(void){

    pthread_mutex_lock(&mid);

    return 0;
}

int sclient_MutexUnlock(void){
    
    pthread_mutex_unlock(&mid);

    return 0;
}

#endif


int sclient_SendData(VIB_ANALYZER_t* sensor){

    PACKET_HEADER_t ph;

    Bytef *data_start=NULL;
    Bytef *data_ptr=NULL;
    Bytef *comp_data=NULL;
    uLong data_size, comp_size;

    size_t header_size;

    uint8_t *packet_start=NULL;
    uint8_t *packet_ptr = NULL;
    size_t packet_size;
    
    CORD_t i;
    int j;

    int ret = 0;

 
    data_size = ((sensor->dev.samples)*sizeof(int8_t)*STR_BUF_LEN*CORD_MAX*2);
    data_start = (Bytef*)malloc(data_size);
    bzero(data_start,data_size);
    data_ptr = data_start;


    
    for(i=CORD_X; i<CORD_MAX; i++){
        for(j=0; j<sensor->tr_data[i].samples; j++){
            sprintf((char*)data_ptr,"%.6f",sensor->tr_data[i].time_domain_data[j]);
            data_ptr += STR_BUF_LEN;
        }
    }

    for(i=CORD_X; i<CORD_MAX; i++){
        for(j=0; j<sensor->tr_data[i].samples; j++){
            sprintf((char*)data_ptr,"%.6f",sensor->tr_data[i].freq_domain_data[j]);
            data_ptr += STR_BUF_LEN;
        }

    }

    comp_data = (Bytef*)malloc(data_size+(data_size/10));
    ret = compress(comp_data,&comp_size,data_start,data_size);
    if(ret != Z_OK){
        printf("compress error : %d\n",ret);
        return -1;
    }
    else{
        printf("src len : %d, dest len : %d\n",data_size, comp_size);

    }

    ph.start = 0xAA;
    ph.end = 0xCC;
    ph.data_size = htons(comp_size);
    ph.sensor_id = htons(sensor->dev.handler);


    header_size = sizeof(PACKET_HEADER_t);
    packet_size = header_size+comp_size;
    packet_start = (uint8_t*)malloc(packet_size);
    bzero(packet_start, packet_size);
    packet_ptr = packet_start;

    packet_ptr = memcpy(packet_ptr,&ph.start,sizeof(ph.start));
    packet_ptr += sizeof(ph.start);
    packet_ptr = memcpy(packet_ptr,&ph.sensor_id,sizeof(ph.sensor_id) );
    packet_ptr += sizeof(ph.sensor_id);
    packet_ptr = memcpy(packet_ptr,&ph.data_size,sizeof(ph.data_size) );
    packet_ptr += sizeof(ph.data_size);

    packet_ptr = memcpy(packet_ptr, comp_data, comp_size);
    packet_ptr += comp_size;

    packet_ptr = memcpy(packet_ptr,&ph.end, sizeof(ph.end));
    packet_ptr += sizeof(ph.end);
    

#if 1
    pthread_mutex_lock(&mid);

    ret = sendto(sockfd, packet_start, 
            packet_size,0 ,(struct sockaddr*)&servaddr, sizeof(servaddr));

    pthread_mutex_unlock(&mid);

    if(ret < 0){
        perror("sendto error");
    }
#endif
    free(data_start);
    free(comp_data);
    free(packet_start);

    return 0;
}
