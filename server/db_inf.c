#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef MONGODB
#include <bson.h>
#include <mongoc.h>
#else
#include <json-c/json.h>
#include <mysql/mysql.h>
#endif

#include "db_inf.h"

#ifdef MONGODB
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
#else // MONGODB


#define CHECK_TABLE_STR	"SHOW TABLES LIKE 'sensor%d'"
#define CREATE_TABLE_STR "CREATE TABLE sensor%d( \
						time TIMESTAMP NULL DEFAULT current_timestamp,\
					 	freq_data json NOT NULL,\
						spectrum json NOT NULL,\
						abs_avg json NOT NULL,\
						vrms json NOT NULL\
						)"

#define INSERT_DATA_STR "INSERT INTO sensor%d\
						(freq_data, spectrum, abs_avg, vrms)\
						VALUES('%s','%s','%s','%s')"

 
MYSQL *sql_ins;
//json_object *sensor_data;
json_object *time_domain_arr[CORD_MAX];
json_object *time_domain_cord_val;

json_object *freq_magnitude_arr[CORD_MAX];
json_object *freq_magnitude_cord_val;

json_object *abs_avg;
json_object *vrms;
//json_object *cord_values[2];

static int dbinf_CreateTable(int sensor_num){

//	int ret = -1;
	char* str = NULL;
	str = (char*)malloc(strlen(CREATE_TABLE_STR));

	sprintf(str,CREATE_TABLE_STR,sensor_num);
	if(mysql_query(sql_ins,str) != 0){
		fprintf(stderr,"create table error - %s\n",mysql_error(sql_ins));
		free(str);
		//return -1;
		exit(1);
	}

	free(str);
	printf("%s\n",__FUNCTION__);
	return 0;
}

static int dbinf_CheckTableExist(int sensor_num){

	int ret = -1, i;
	int num_field;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char str[32]={0,};

	sprintf(str,CHECK_TABLE_STR,sensor_num);
	if(mysql_query(sql_ins,str) != 0){
		fprintf(stderr,"query error - %s\n",mysql_error(sql_ins));
		//return ret;
		exit(1);
	}

	result = mysql_store_result(sql_ins);
	if(result == NULL){
		fprintf(stderr,"store result error - %s\n",mysql_error(sql_ins));
		//return ret;
		exit(1);
	}

	row = mysql_fetch_row(result);
	if(row == NULL){
		printf("No db for sensor %d\n",sensor_num);
		ret = 0;
	}
	else{
		printf("DB detected - %s\n",row[0]);
		ret = 1;
	}
	mysql_free_result(result);

	return ret;
}

static int dbinf_JsoncInit(void){
	CORD_t i;
	//sensor_data = json_object_new_object();
	for(i=CORD_X; i<CORD_MAX; i++){
		time_domain_arr[i] = json_object_new_array();
		freq_magnitude_arr[i] = json_object_new_array();
	}

	time_domain_cord_val = json_object_new_object();
	freq_magnitude_cord_val = json_object_new_object();
	abs_avg = json_object_new_object();
	vrms = json_object_new_object();

	return 0;
}

static int dbinf_JsoncCleanUp(void){

	CORD_t i;

	for(i=CORD_X; i<CORD_MAX; i++){
		json_object_put(time_domain_arr[i]);
		json_object_put(freq_magnitude_arr[i]);
	}

	json_object_put(freq_magnitude_cord_val);
	json_object_put(time_domain_cord_val);
	json_object_put(abs_avg);
	json_object_put(vrms);


	return 0;
}

static int dbinf_MysqlInit(void){
	
	int ret = 0;

    ret = mysql_library_init(0, NULL, NULL);
    if(ret != 0){
        fprintf(stderr, "Cannot init mysql lib\n");
        exit(1);
    }

    sql_ins = mysql_init(NULL);
    if( sql_ins == NULL ){
        fprintf(stderr, "%s\n", mysql_error(sql_ins));
        exit(1);
    }

    if(mysql_real_connect(sql_ins, "localhost", "magiceco", "1234","vibration_analyzer",0,NULL,0 )==NULL){
        fprintf(stderr, "mysql connect error -%s\n", mysql_error(sql_ins));
        exit(1);
    }

	return 0;
}

static int dbinf_MysqlCleanUp(void){

	mysql_close(sql_ins);
	mysql_library_end();

	return 0;
}

static int dbinf_BuildJsonCord(SENSOR_t* sensor_data){

	int i=0, j=0;
	char str[10] = {0,};
	char* cord[CORD_MAX] = {"x","y","z"};
	double d = 0.23412;

#if 0	
	/* Array for time domain */
	for(i=0; i<SAMPLES; i++){

		for(j=0; j<CORD_MAX; j++){
			bzero(str,sizeof(str));
			sprintf(str,"%.7f",sensor_data->time_domain[j][i]);
			json_object_object_add(time_domain_cord_val,cord[j],json_object_new_double_s(d,str));		
		}

		json_object_array_add(time_domain_arr,time_domain_cord_val);
	}

//	json_object_object_add(sensor_data,"time_domain", time_domain_arr);
	for(i=0; i<SAMPLES/2; i++){

		for(j=0; j<CORD_MAX; j++){
			bzero(str,sizeof(str));
			sprintf(str,"%.7f",0.56789);
			json_object_object_add(freq_magnitude_cord_val,cord[j],json_object_new_double_s(d,str));
		}

		json_object_array_add(freq_magnitude_arr,freq_magnitude_cord_val);
	}
#else
	/* Array for time domain */
	for(i=CORD_X; i<CORD_MAX; i++){

		for(j=0; j<SAMPLES; j++){
		//for(j=0; j<4; j++){
			bzero(str,sizeof(str));
			sprintf(str,"%.6f",sensor_data->time_domain[i][j]);
			//json_object_object_add(time_domain_cord_val,cord[i],json_object_new_double_s(d,str));		
			json_object_array_add(time_domain_arr[i],json_object_new_double_s(d,str));
		}

		//json_object_array_add(time_domain_arr,time_domain_cord_val);
		json_object_object_add(time_domain_cord_val,cord[i],time_domain_arr[i]);
	}

//	json_object_object_add(sensor_data,"time_domain", time_domain_arr);
	for(i=CORD_X; i<CORD_MAX; i++){

		for(j=0; j<SAMPLES/2; j++){
		//for(j=0; j<2; j++){

			bzero(str,sizeof(str));
			sprintf(str,"%.6f",sensor_data->freq_magnitude[i][j]);
			//json_object_object_add(freq_magnitude_cord_val,cord[i],json_object_new_double_s(d,str));
			json_object_array_add(freq_magnitude_arr[i],json_object_new_double_s(d,str));

		}

		//json_object_array_add(freq_magnitude_arr,freq_magnitude_cord_val);
		json_object_object_add(freq_magnitude_cord_val,cord[i],freq_magnitude_arr[i]);

	}

	for(i=CORD_X; i<CORD_MAX; i++){
		bzero(str,sizeof(str));
		sprintf(str,"%.6f",sensor_data->abs_avg[i]);
		json_object_object_add(abs_avg,cord[i],json_object_new_double_s(d,str));		
		bzero(str,sizeof(str));
		sprintf(str,"%.6f",sensor_data->vrms[i]);
		json_object_object_add(vrms,cord[i],json_object_new_double_s(d,str));

	}

#endif

	return 0;

}

int dbinf_InsertData(SENSOR_t *sensor_data){

	char* query_str = NULL;
	char* time_domain_str = NULL;
	char* freq_magnitude_str = NULL;
	char* abs_avg_str = NULL;
	char* vrms_str = NULL;
	int sensor_num = sensor_data->sensor_num;
	size_t query_len;
	if(dbinf_CheckTableExist(sensor_num) == 0)
        dbinf_CreateTable(sensor_num);

	dbinf_JsoncInit();
	dbinf_BuildJsonCord(sensor_data);


	time_domain_str = (char*)json_object_to_json_string(time_domain_cord_val);
	freq_magnitude_str = (char*)json_object_to_json_string(freq_magnitude_cord_val);
	abs_avg_str = (char*)json_object_to_json_string(abs_avg);
	vrms_str = (char*)json_object_to_json_string(vrms);

	query_len = strlen(INSERT_DATA_STR)+strlen(time_domain_str)+strlen(freq_magnitude_str)+strlen(abs_avg_str)+strlen(vrms_str);
	query_str = (char*)malloc(query_len);
	bzero(query_str, query_len);
	sprintf(query_str, INSERT_DATA_STR,sensor_num,time_domain_str,freq_magnitude_str,abs_avg_str,vrms_str);
 	//printf("%s\n\n",query_str);
	if(mysql_query(sql_ins,query_str) != 0){
		fprintf(stderr,"insert data error - %s\n",mysql_error(sql_ins));
		free(query_str);
		return -1;
	}

	dbinf_JsoncCleanUp();

	free(query_str);

	return 0;
}



int dbinf_Init(void){

	dbinf_MysqlInit();
	//dbinf_JsoncInit();


	return 0;
}

int dbinf_CleanUp(void){

	dbinf_MysqlCleanUp();
	//dbinf_JsoncCleanUp();


	return 0;
}


#endif
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
