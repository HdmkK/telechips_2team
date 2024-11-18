#include <stdio.h>
#include "../filter.h"


struct wma_filter_data{
    float *buffer;
    int buffer_size;

    int count;

    int* weights;
    int weights_size;
};

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

int main() {

    float sensor_data[20] = {50, 52, 51, 53, 200, 54, 55, 57, 59, 60, 65, 70, 80, 85, 90, 150, 100, 95, 94, 93};
    float output;

    float buffer[5] = {0};
    int weights[5] = {1,2,3,4,5};

    struct wma_filter_data data = {
        .buffer = buffer,
        .buffer_size = sizeof(buffer)/sizeof(buffer[0]),
        .count = 0,
        .weights = weights,
        .weights_size = sizeof(weights)/sizeof(weights[0]),
    };

    FILTER filter;
    filter.data = &data;
    filter.filtering = weighted_moving_average;
    
    for (int i = 0; i < 20; i++){
        output = filter.filtering(&filter, sensor_data[i]);
        printf("original value : %f, filtered value : %f\n", sensor_data[i] ,output);
    }

    return 0;
}
