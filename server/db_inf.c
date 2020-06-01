#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


#include <json-c/json.h>
#include <mysql/mysql.h>

#include "hiredis/hiredis.h"

#include "db_inf.h"

#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <sys/un.h> 


#define CHECK_TABLE_STR	"SHOW TABLES LIKE 'sensor%d'"
#define CREATE_TABLE_STR "CREATE TABLE sensor%d( \
						time TIMESTAMP NULL DEFAULT current_timestamp,\
					 	freq_data json NOT NULL,\
						spectrum json NOT NULL,\
						abs_avg json NOT NULL,\
						vrms json NOT NULL )"
						//alarm_criteria json NOT NULL )"

#define INSERT_DATA_STR "INSERT INTO sensor%d\
						(freq_data, spectrum, abs_avg, vrms)\
						VALUES('%s','%s','%s','%s')"


#define CREATE_ALARM_CRITERIA_STR "CREATE TABLE sensor%d_alarm( alarm_criteria json NOT NULL )"


/*
#define GET_VRMS_STR "SELECT json_extract(vrms, '%s') FROM sensor%d'"

#define GET_VRMS_ALARM_CRITERIA_STR "SELECT alarm_criteria FROM sensor%d"
#define INSERT_VRMS_ALARM_CRITERIA_STR "INSERT INTO sensor%d\
										alarm_criteria VALUES '%s'"
*/

#define REDIS_STR		"{ \"timestamp\": %s, \"freq_data\": %s, \"spectrum\": %s }"
//#define REDIS_STR		"{ \"freq_data\": %s, \"spectrum\": %s }"



MYSQL *sql_ins;
json_object *time_domain_arr[CORD_MAX];
json_object *time_domain_cord_val;

json_object *freq_magnitude_arr[CORD_MAX];
json_object *freq_magnitude_cord_val;

json_object *abs_avg;
json_object *vrms;



#define REDIS_PORT	6379

//time_t sensorDataTimeTable[TOTAL_SENSOR_NUM];
DB_SENSOR_INFO_t sensor_data_table[TOTAL_SENSOR_NUM];

static redisContext* cont = NULL;
static redisReply* rep = NULL;

static int dbinf_InitRedis(void){

	//struct timeval timeout = { 1, 500000 };
	cont = redisConnect("127.0.0.1", REDIS_PORT);

	if (cont == NULL || cont->err) {
        if (cont) {
            printf("[%s] Connection error: %s\n", cont->errstr);
            redisFree(cont);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

	return 0;
}

static int dbinf_CleanUpRedis(void){

	if(rep != NULL)
		freeReplyObject(rep);

	if(cont != NULL)
		redisFree(cont);

	return 0;
}

static int dbinf_UploadDataRedis(int sensor_num, char* data_str){

	char num_str[4] = {0,};
	sprintf(num_str,"%d",sensor_num);

	rep = redisCommand(cont, "SET key:%s %s",num_str, data_str);
	if((rep == NULL)||(cont->err)){
		fprintf(stderr,"Redis Upload error: %s\n", cont->errstr);
		exit(1);
	}
	freeReplyObject(rep);

	return 0;
}

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


	/* Array for time domain */
	for(i=CORD_X; i<CORD_MAX; i++){

		for(j=0; j<SAMPLES; j++){
			bzero(str,sizeof(str));
			sprintf(str,"%.6f",sensor_data->time_domain[i][j]);
			json_object_array_add(time_domain_arr[i],json_object_new_double_s(d,str));
		}

		json_object_object_add(time_domain_cord_val,cord[i],time_domain_arr[i]);
	}

	for(i=CORD_X; i<CORD_MAX; i++){

		for(j=1; j<SAMPLES/2; j++){ /* No DC */

			bzero(str,sizeof(str));
			sprintf(str,"%.6f",sensor_data->freq_magnitude[i][j]);
			json_object_array_add(freq_magnitude_arr[i],json_object_new_double_s(d,str));

		}
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


	return 0;

}

static int dbinf_CheckInsertMySQL(int sensor_num, time_t current_time){
	
#define RENEW_INTERVAL 1800 //30min

#if 0
	if(current_time - sensorDataTimeTable[sensor_num] > RENEW_INTERVAL){
		sensorDataTimeTable[sensor_num] = current_time;
		return 1;
	}
#else
	if(current_time - sensor_data_table[sensor_num].sql_save_time > RENEW_INTERVAL){
		sensor_data_table[sensor_num].sql_save_time = current_time;
		return 1;
	}

#endif

	return 0;
}

#if 0
static int dbinf_GetSensorVrmsAvg(int sensor_num, double vrms_avg[3]){

	int ret = -1, i, j;
	my_ulonglong row_num;
	MYSQL_RES *result;
	MYSQL_ROW row;
	const char* cord[3] = {"$.x", "$.y", "$.z"};
	char str[128]={0,};

	for(i=0; i<(sizeof(cord)/sizeof(char*)); i++){
		bzero(str, sizeof(str));
		sprintf(str,GET_VRMS_STR,cord[i],sensor_num);

		mysql_query(sql_ins,str);

		result = mysql_store_result(sql_ins);

		row_num = mysql_num_rows(result);
		row = mysql_fetch_row(result);

		for(j=0; j<row_num; j++){

			vrms_avg[i] += atof(row[j]);
		}

		vrms_avg[i] = vrms_avg[i] / row_num;
		mysql_free_result(result);
	}

	
	return 0;
}
#endif

int dbinf_InsertData(SENSOR_t *sensor_data){

	char* time_domain_str = NULL;
	char* freq_magnitude_str = NULL;
	char* abs_avg_str = NULL;
	char* vrms_str = NULL;
	int sensor_num = sensor_data->sensor_num;

	char* redis_str = NULL;
	size_t str_len;

	time_t current_time;
	struct tm* local_time;
	char timestamp_str[26]={0,};
	double vrms_avg[3] = {0,};
	char vrms_avg_str[128]={0,};

	time(&current_time);

	dbinf_JsoncInit();
	dbinf_BuildJsonCord(sensor_data);


	time_domain_str = (char*)json_object_to_json_string(time_domain_cord_val);
	freq_magnitude_str = (char*)json_object_to_json_string(freq_magnitude_cord_val);
	abs_avg_str = (char*)json_object_to_json_string(abs_avg);
	vrms_str = (char*)json_object_to_json_string(vrms);

	//printf("[hmm] %s\n\n",time_domain_cord_val);

	if(dbinf_CheckInsertMySQL(sensor_num, current_time)){

		char* query_str = NULL;
		size_t query_len;

		if(dbinf_CheckTableExist(sensor_num) == 0)
			dbinf_CreateTable(sensor_num);


		/* Get the basis of alarm */
#if 0
		if(sensor_data_table[sensor_num].vrms_save_flag == 0){
				
			if(sensor_data_table[sensor_num].sql_save_cnt < 50){
				sensor_data_table[sensor_num].sql_save_cnt++;
			}
			else{
				/* Get & Save Vrms avg */
				dbinf_GetSensorVrmsAvg(sensor_num,vrms_avg);
				sprintf(vrms_avg_str, "{ \"x\": %.6f, \"y\": %.6f, \"z\": %.6f }", vrms_avg[1], vrms_avg[2], vrms_avg[3]);
				query_len = strlen(INSERT_VRMS_ALARM_CRITERIA_STR) + strlen(vrms_avg);
				query_str = (char*)malloc(query_len);
				bzero(query_str, query_len);
				sprintf(query_str, INSERT_VRMS_ALARM_CRITERIA_STR,sensor_num, vrms_avg_str);
				if(mysql_query(sql_ins,query_str) == 0){
					sensor_data_table[sensor_num].vrms_save_flag = 1;
				}
				free(query_str);
				printf("%s - alarm criteria set\n",__FUNCTION__);
			}

		}
#endif
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

		free(query_str);

	} 


	local_time = localtime(&current_time);
	sprintf(timestamp_str, "\"%d.%d.%d %02d:%02d:%02d\"",
		local_time->tm_year+1900, 
		local_time->tm_mon+1,
		local_time->tm_mday,
		local_time->tm_hour,
		local_time->tm_min,
		local_time->tm_sec
	);

	str_len = strlen(REDIS_STR)+strlen(timestamp_str)+strlen(time_domain_str)+strlen(freq_magnitude_str)+8;
//	str_len = strlen(REDIS_STR)+strlen(time_domain_str)+strlen(freq_magnitude_str)+8;

	redis_str = (char*)malloc(str_len);
	bzero(redis_str, str_len);
	sprintf(redis_str, REDIS_STR,timestamp_str,time_domain_str,freq_magnitude_str);
//	sprintf(redis_str, REDIS_STR,time_domain_str,freq_magnitude_str);

	dbinf_UploadDataRedis(sensor_num, redis_str);

	free(redis_str);




	dbinf_JsoncCleanUp();


	return 0;
}



int dbinf_Init(void){

	dbinf_MysqlInit();
	//dbinf_JsoncInit();
	//dbinf_InitClientIPC();
	dbinf_InitRedis();

	return 0;
}

int dbinf_CleanUp(void){

	dbinf_MysqlCleanUp();
	//dbinf_JsoncCleanUp();
	//dbinf_CleanUpClientIPC();
	dbinf_CleanUpRedis();

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

