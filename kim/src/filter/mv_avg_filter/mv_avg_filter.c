#include <stdio.h>
#include "../filter.h"

#define WINDOW_SIZE 5  // 이동 평균 윈도우 크기 정의

#define MAX_QUEUE_SIZE 100

//filtering data
struct mv_avg_queue{
    float buffer[MAX_QUEUE_SIZE];
    int head, tail, count;
    int sum;
};

struct mv_avg_queue mv_avg_queue = {
    {},
    .head = 0,
    .tail = 0,
    .count = 0,
    .sum = 0,
};

// 이동 평균을 계산하는 함수
float moving_average(struct filter* filter, float new_value) {

    struct mv_avg_queue *data = (struct mv_avg_queue*)(filter->data);
    float* buffer = data->buffer;
    int head = data->head;
    int tail = data->tail;
    int count = data->count;
    int sum = data->sum;

    // 새로운 값을 버퍼에 저장
    buffer[tail] = new_value;
    tail = (tail + 1) % MAX_QUEUE_SIZE;
    sum += new_value;

    //맨 앞 값을 버퍼에서 삭제
    if (count > WINDOW_SIZE){
        sum -= buffer[head];
        head = (head + 1) % MAX_QUEUE_SIZE;
    }else{
        count++;
    }

    data->head = head;
    data->tail = tail;
    data->sum = sum;
    data->count = count;
   

    // 평균을 계산하여 반환
    return sum / count;
}

/*int main() {
    float data[20] = {50, 52, 51, 53, 200, 54, 55, 57, 59, 60, 65, 70, 80, 85, 90, 150, 100, 95, 94, 93};


    FILTER filter1;
    filter1.data = &mv_avg_queue;
    filter1.filtering = moving_average;
    
    for (int i = 0; i < 20; i++) {
        float avg = filter1.filtering(&filter1, data[i]);
        printf("Data: %f, Moving Average: %f\n", data[i], avg);
    }

    return 0;
}*/
