#include <stdio.h>
#include <stdlib.h>
#include "../filter.h"

struct median_filter_data{
    int (*compare)(const void *a, const void* b);
    int window_size;
    float window[50];
    int head, tail;
    int cur_cnt;
};

// 비교 함수 (qsort에서 사용)
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void print_window(int* arr, int size){
    for (int i = 0; i < size; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}

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




/*int main() {
    // 예제 데이터 (노이즈가 있는 시계열 데이터)
    float sensor_data[20] = {50, 52, 51, 53, 200, 54, 55, 57, 59, 60, 65, 70, 80, 85, 90, 150, 100, 95, 94, 93};

    int length = sizeof(sensor_data) / sizeof(sensor_data[0]);
    int window_size = 3;
    float output[length];

    struct median_filter_data data = {
        .compare = compare,
        .window_size = 5,
        .head = 0,
        .tail = 0,
        .cur_cnt = 0
    };

    FILTER filter;
    filter.data = &data;
    filter.filtering = median_filter;

    for (int i = 0; i < length; i++){
        output[i] = filter.filtering(&filter, sensor_data[i]);
    }

    // 결과 출력
    for (int i = 0; i < length; i++) {
        printf("original value : %f, filtered value : %f\n", sensor_data[i], output[i]);
    }


    return 0;
}*/
