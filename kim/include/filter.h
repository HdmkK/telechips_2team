#ifndef __FILTER_H__
#define __FILTER_H__

#include <stdlib.h>


#define MAX_QUEUE_SIZE 100

typedef struct filter{
	void* data;
	float (*filtering) (struct filter* filter, float new_value);
}FILTER;


//중간값 필터용 데이터 구조체
struct median_filter_data{
    int (*compare)(const void *a, const void* b);
    int window_size;
    float window[50];
    int head, tail;
    int cur_cnt;
};


//지수 이동 평균 필터용 데이터 구조체
struct ema_data{
        int is_initialized;
        float ema;
        float alpha;
};

//이동 평균 필터용 데이터 구조체
struct mv_avg_queue{
    float buffer[MAX_QUEUE_SIZE];
    int head, tail, count;
    int sum;
};


//가중 이동 평균 필터용 데이터 구조체
struct wma_filter_data{
    float *buffer;
    int buffer_size;

    int count;

    int* weights;
    int weights_size;
};

float exponential_moving_average(FILTER* filter, float new_value);
float median_filter(FILTER *filter, float new_value);
float weighted_moving_average(FILTER *filter, float new_value);
float moving_average(struct filter* filter, float new_value);

#endif