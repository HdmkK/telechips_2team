#ifndef __VENT_APP_H__
#define __VENT_APP_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "i2c.h"
#include "filter.h"
#include "motor.h"
#include "ultra.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>



#define SENSOR_M_DLY 100
#define DURATION 1 //조건을 일정시간동안 만족할때 창문을 제어하기 위한 용도의 DURATION 상수

#define TIME_KEEP_VENTILATE 20


#define PRECIPITATION_THRESHOLD 100
#define DISTANCE_THRESHOLD 70
#define AIR_QUALITY_THRESHOLD 100
#define FINE_DUST_THRESHOLD 170


typedef struct sensor_data{
	float precipitation1;
	float precipitation2;
	float distance;
	float air_quality;
	float fine_dust;
}SENSOR_DATA;


struct VentDTO {
    int raining;
    int in_tunnel;
    int air_condition;
    int fine_dust;
};



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

static void timer_handler(int signum);
static int is_outer_condition_bad(float cur_precipitation, float cur_precipitation2, float cur_distance, float cur_particulate_matter);
static void init_ventilate_timer();
static void set_ventilate_timer(int sec);
int init_i2c();
int destory_i2c();

static int read_precipitation1();
static int read_precipitation2();
static int read_air_quality();
static int read_distance();
static int read_fine_dust();





//전역변수
//===========================================
SENSOR_DATA sensor_data;
TIMER ventilate_timer;

VENT_STATE cur_vent_state = CLOSED_COMPLETELY;

int vent_lock = 0; //내부 공기 좋아져도 일정 시간 동안 창문 open 유지하도록

pthread_mutex_t mutex;
//===========================================

#endif


extern int i2cfile,ret48,ret49,ret4A,ret4B;
extern FILE *valueFile;