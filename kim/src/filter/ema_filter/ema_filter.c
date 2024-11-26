#include <stdio.h>
#include "../filter.h"

//#define ALPHA 0.1  // EMA의 가중치 계수

struct ema_data{
        int is_initialized;
        float ema;
        float alpha;
};

// 지수 이동 평균을 계산하는 함수
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




/*int main() {
    float data[20] = {50, 52, 51, 53, 200, 54, 55, 57, 59, 60, 65, 70, 80, 85, 90, 150, 100, 95, 94, 93};


    printf("Exponential Moving Average Filtered Values:\n");

    
    struct ema_data ema_dt = {
        .is_initialized = 0,
        .alpha = 0.7,
    };


    FILTER filter1;
    filter1.data = &ema_dt;
    filter1.filtering = exponential_moving_average;

    for (int i = 0; i < 20; i++) {
        float ema = filter1.filtering(&filter1, data[i]);
        printf("Data: %f, EMA: %.2f\n", data[i], ema);
    }

    return 0;
}*/
