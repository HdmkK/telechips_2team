#ifndef __VENT_APP_H__
#define __VENT_APP_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


#define SENSOR_M_DLY 100

#define TIME_KEEP_VENTILATE 20


#define PRECIPITATION_THRESHOLD 100
#define DISTANCE_THRESHOLD 10
#define AIR_QUALITY_THRESHOLD 100
#define FINE_DUST_THRESHOLD 100


typedef struct sensor_data{
	float precipitation;
	float distance;
	float air_quality;
	float fine_dust;
}SENSOR_DATA;


typedef struct timer_struct{
	struct itimerspec timer;
	timer_t timerid;
}TIMER;


typedef enum vent_state{
	CLOSED_COMPLETELY,
	BEING_OPEN,
	OPEN_COMPLETELY,
	BEING_CLOSED
}VENT_STATE;

void timer_handler(int signum);
void* thread_func1(void* arg);
void* thread_func2(void* arg);
int is_outer_condition_bad(float cur_precipitation, float cur_distance, float cur_particulate_matter);
void init_ventilate_timer();
void set_ventilate_timer(int sec);
int init_i2c();
int destory_i2c();

int read_precipitation();
int read_air_quality();
int read_distance();
int read_fine_dust();



//전역변수
//===========================================
SENSOR_DATA sensor_data;
TIMER ventilate_timer;

VENT_STATE cur_vent_state = CLOSED_COMPLETELY;

int vent_lock = 0; //내부 공기 좋아져도 일정 시간 동안 창문 open 유지하도록

pthread_mutex_t mutex;
//===========================================

#endif


extern int i2cfile,ret48,ret49,ret4A;
extern FILE *valueFile;