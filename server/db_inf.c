#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <bson.h>
#include <mongoc.h>

#include "db_inf.h"

#define DB_NAME "sensor_data"

static mongoc_client_t *client;



int dbinf_InsertData(DB_SET_t *data){

    mongoc_collection_t *collection;
    bson_oid_t oid;
    bson_t *doc;
    bson_t child;
    bson_t child2;
    bson_error_t error;
    bool ret;

    time_t rawtime;
    struct tm *timeinfo;

    int i = 0;
    char sensor_num[32];

    bzero(sensor_num,sizeof(sensor_num));
    //printf("sensor_%d\n", data->sensor_num);
    sprintf(sensor_num,"sensor_%d", data->sensor_num);
    collection = mongoc_client_get_collection(client, DB_NAME,sensor_num);

    doc = bson_new();

    bson_oid_init(&oid,NULL);

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    printf("%d %d %d\n",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday);

    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_INT32(doc,"year",(int32_t)(timeinfo->tm_year+1900));
    BSON_APPEND_INT32(doc,"month",(int32_t)timeinfo->tm_mon+1);
    BSON_APPEND_INT32(doc,"day",(int32_t)(timeinfo->tm_mday));
    BSON_APPEND_INT32(doc,"hour",(int32_t)(timeinfo->tm_hour));
    BSON_APPEND_INT32(doc,"min",(int32_t)(timeinfo->tm_min));
    BSON_APPEND_INT32(doc,"sec",(int32_t)(timeinfo->tm_sec));

    /* Insert array for x, y, z - time domain & magnitude */
    BSON_APPEND_ARRAY_BEGIN(doc,"time_domain_data",&child);
    for(i = 0; i < sizeof(); i++){

        bson_append_document_begin()
        BSON_APPEND_DOUBLE();
        BSON_APPEND_DOUBLE();
        BSON_APPEND_DOUBLE();
        
    }
    bson_append_array_end(doc,&child);


    BSON_APPEND_ARRAY_BEGIN(doc,"freq_magnitude_data",&child);
    for(i = 0; i < sizeof(); i++){

    }
    bson_append_array_end(doc,&child);


    if (!mongoc_collection_insert_one (
           collection, doc, NULL, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
        return -1;
    }
    bson_destroy(doc);
    mongoc_collection_destroy(collection);


    return 0;
}


int dbinf_Init(void){

    mongoc_init();
    client = mongoc_client_new("mongodb://localhost:27017/?appname=vibration-analyzer");

    return 0;
}

int dbinf_CleanUp(void){

    mongoc_client_destroy(client);
    mongoc_cleanup();

    return 0;
}

//#define MAIN

#ifdef MAIN

int main(void){

    DB_SET_t db_data;

    dbinf_Init();

    db_data.sensor_num = 1;
    dbinf_InsertData(&db_data);

    db_data.sensor_num = 2;
    dbinf_InsertData(&db_data);
 
    dbinf_CleanUp();

    return 0;
}


#endif