#include <stdio.h>
#include <stdlib.h>

#define INVALID_VALUE -1  // 불량 데이터를 나타내는 값
#define VALID 50              // 임계값 (abs 기준)

// 불량 데이터를 확인하는 함수
int detect_error_and_average(int data1, int data2, int data3) {
    // 두 데이터 간 차이를 계산
    int abs12 = abs(data1 - data2);
    int abs13 = abs(data1 - data3);
    int abs23 = abs(data2 - data3);

    // 불량 데이터 확인
    if (abs12 > VALID && abs13 > VALID) {
        // data1이 다른 두 데이터와 차이가 큼 → data1이 불량
        printf("error data: %d data1\n", data1);
        return (data2 + data3) / 2; // 나머지 평균값 반환
    } else if (abs12 > VALID && abs23 > VALID) {
        // data2가 다른 두 데이터와 차이가 큼 → data2가 불량
        printf("불량 데이터: %d data2\n", data2);
        return (data1 + data3) / 2; // 나머지 평균값 반환
    } else if (abs13 > VALID && abs23 > VALID) {
        // data3가 다른 두 데이터와 차이가 큼 → data3가 불량
        printf("불량 데이터: %d (데이터3)\n", data3);
        return (data1 + data2) / 2; // 나머지 평균값 반환
    }

    // 불량 데이터가 없으면 평균값 반환
    printf("불량 데이터 없음\n");
    return (data1 + data2 + data3) / 3;
}

// 메인 함수
int main() {
    int data1 = 50;  // 입력 데이터
    int data2 = 55;
    int data3 = 180;

    int result = detect_error_and_average(data1, data2, data3);

    printf("정상 데이터: %d\n", result); // 최종 데이터 출력
    return 0;
}
