#include "filter.h"
#include <stdlib.h>

#define WINDOW_SIZE 5 

// 중간값 필터 함수
float median_filter(FILTER *filter, float new_value) {

    struct median_filter_data *data = (struct median_filter_data*)(filter->data);

    int window_size = data->window_size;
    float* window = data->window;
    int head = data->head;
    int tail = data->tail;
    int cur_cnt = data->cur_cnt;

    int tmp_buffer[50];

    window[tail] = new_value;
    tail = (tail + 1) % 50;
    cur_cnt++;

    if (cur_cnt > window_size) {
        head = (head + 1) % 50;
        cur_cnt--;
        
    }

    for (int i = 0, window_index = head; i < window_size; i++, window_index = (window_index + 1) % 50){
        tmp_buffer[i] = window[window_index];
    }

    // 윈도우 내의 요소 정렬
    qsort(tmp_buffer, window_size, sizeof(float), data->compare);
    //print_window(tmp_buffer, window_size);


    data->head = head;
    data->tail = tail;
    data->cur_cnt = cur_cnt;

    if (cur_cnt < window_size) return new_value;
    else return tmp_buffer[window_size/2];
}



// 지수 이동 평균 필터 함수
float exponential_moving_average(FILTER* filter, float new_value) {

    struct ema_data *data = (struct ema_data*)(filter->data);
    int is_initialized = data->is_initialized;
    float ema = data->ema;
    float alpha = data->alpha;

    // 초기화: 첫 번째 데이터 입력 시, ema를 해당 값으로 설정
    if (!is_initialized) {
        ema = new_value;
        is_initialized = 1;
    } else {
        // EMA 계산
        ema = alpha * new_value + (1 - alpha) * ema;
    }

    data->is_initialized = is_initialized;
    data->ema = ema;
    data->alpha = data->alpha;

    return ema;
}


// 이동 평균 필터 함수
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

//가중 이동 평균 필터 함수
float weighted_moving_average(FILTER *filter, float new_value) {
    float weighted_sum = 0;
    int total_weight = 0;

    struct wma_filter_data* data = (struct wma_filter_data*)(filter->data);
    float* buffer = data->buffer;
    int buffer_size = data->buffer_size;

    int count = data->count;

    int* weights = data->weights;
    int weights_size = data->weights_size;


    if (count < buffer_size){
        count++;
    }

    for (int i = 0; i < buffer_size-1; i++){
        buffer[i] = buffer[i+1];
    }

    buffer[buffer_size-1] = new_value;

    for (int i = 0; i < weights_size; i++) {

        if (count + i < 5) continue;

        weighted_sum += buffer[i] * weights[i];
        total_weight += weights[i];
    }


    data->count = count;
    
    return weighted_sum / total_weight;
}