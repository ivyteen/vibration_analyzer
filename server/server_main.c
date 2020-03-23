#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "data_handler.h"
#include "db_inf.h"

#define UDP_PORT 5100


int sockfd = 0;
socklen_t data_len;
struct sockaddr_in servaddr, cliaddr;


static const int samples = SAMPLES;


int server_Init(void){

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        printf("Create Socket Failed\n");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(UDP_PORT);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return 0;
}

int server_CleanUp(void){

    close(sockfd);
    printf("[%s]\n",__FUNCTION__);

    return 0;
}

int server_RecvData(void){
    uint8_t* packet_buf = NULL;
    //int size=0;
    socklen_t clilen = 0;
    ssize_t recv_len = 0;
    uint16_t comp_size = 0;
    size_t size = PACKET_SIZE;
    packet_buf = (uint8_t*)malloc(size);
    printf("[%s]\n",__FUNCTION__);

    while(1){
       // printf("loop?\n");
        bzero(packet_buf,size);
        clilen = sizeof(cliaddr);
        recv_len = recvfrom(sockfd, packet_buf,PACKET_SIZE,0,(struct sockaddr*)&cliaddr,&clilen);
        if(recv_len < 0){
            printf("[%s] recv error %d\n",__FUNCTION__,recv_len);
            exit(1);
        }

        if((packet_buf[0]!=0xAA)||(packet_buf[recv_len-1]!=0xCC)){
            printf("[%s] wrong packet\n",__FUNCTION__);
            continue;
        }

        dhandler_DataExtractor(packet_buf);
    }

    free(packet_buf);

    return 0;
}



int main(void){
    

    server_Init();
    dbinf_Init();
    dhandler_Init();

    server_RecvData();
    
    dhandler_CleanUp();
    dbinf_CleanUp();
    server_CleanUp();

    return 0;
}