#ifndef ULTRASONIC_TEST_H
#define ULTRASONIC_TEST_H

#include <gpio_test.h>
#include <debug.h>
#include <gpio.h>
#include <gic.h>
#include <fmu.h>
#include <bsp.h>
#include "FreeRTOS.h"      // FreeRTOS 헤더 파일 포함
#include "task.h"          // FreeRTOS의 task 관련 함수들 포함
#include <main.h>
#include <timer_test.h>
#include <stdint.h>
#include <timer.h>
#include <reg_phys.h>
#include <sal_api.h>
#include <sal_internal.h>

#define TMR_MAIN_CNT 0x14UL  // 메인 카운터 레지스터 오프셋


// GPIO 설정 함수
void GPIO_Setup(void);          // GPIO 핀 설정 함수

// 초음파 센서 제어 함수
void ultrasonic_test(void *pArg); 

// 트리거 신호 전송
void SendTriggerPulse(void);

// Echo 신호 시간 측정
void MeasureEchoTime(void);

//uint32 Timer_GetValue(void);
void timer_on(void);

uint32 calculate_distance(uint32 duration_us);




#endif // ULTRASONIC_TEST_H

