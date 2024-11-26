/*
#include <ultrasonic_test.h>

#define TIMER_CLK_FREQ 1000000  // 타이머 주파수: 1MHz = 1µs
#define SPEED_OF_SOUND 0.0343  // cm/s

// 타이머 값 읽기
uint32 Timer_GetValue(void) {
   uint32 reg_base = MCU_BSP_TIMER_BASE + (2UL * 0x100UL); // 채널에 따라 타이머 베이스 주소 계산 (2채널 기준)
    uint32 reg_main_counter = reg_base + TMR_MAIN_CNT;     // MAIN_CNT 레지스터 주소
    return SAL_ReadReg(reg_main_counter);                  // MAIN_CNT 값 읽기
}

uint32 calculate_distance(uint32 duration_ticks) {
    // 클럭 값을 마이크로초로 변환 후 거리 계산
    uint32 duration_us = duration_ticks;
    return (duration_us * SPEED_OF_SOUND) / 2.0;  // 왕복 거리이므로 나누기 2
}



//GPIO SETUP
void GPIO_Setup(void) {
    // TRIG 핀을 출력으로 설정(23)
    GPIO_Config(GPIO_GPC(23UL), GPIO_FUNC(0UL) | GPIO_OUTPUT);
    // ECHO 핀을 입력으로 설정(24)
    GPIO_Config(GPIO_GPC(24UL), GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN);    
}

//TRIGGER FOR ULTRASONIC SENSOR
void SendTriggerPulse(void) {
    GPIO_Set(GPIO_GPC(23UL),0);
    SAL_TaskSleep(50); // 준비 시간 대기
    GPIO_Set(GPIO_GPC(23UL),1);// HIGH 신호 전송
    delay_us3(10);
    GPIO_Set(GPIO_GPC(23UL),0); // LOW 신호 전송
}

// Echo 신호 시간 측정
void MeasureEchoTime(void) {
    timer_on();
    uint32 pulse_start = 0, pulse_end = 0;    

    SendTriggerPulse(); 

    // Echo 신호 시작 대기
    while (GPIO_Get(GPIO_GPC(24UL)) == 0UL) 
    pulse_start = TIMER_GetCurrentMainCounter();   

    //echo 신호 종료 대기
    while (GPIO_Get(GPIO_GPC(24UL)) == 1UL);   
    pulse_end = TIMER_GetCurrentMainCounter(); // 종료 시간 기록
   
    // Echo 신호의 지속 시간 계산 (마이크로초 단위)
    uint32 duration_us = pulse_end - pulse_start;
    TIMER_Disable(TIMER_CH_2);

    // 거리 계산 및 출력
    uint32 distance = calculate_distance(duration_us);
    mcu_printf("Distance: %d cm\n", distance);
}



// 초음파 센서 테스트 함수
void ultrasonic_test(void *pArg)
{
    (void)pArg;
    GPIO_Setup();    
   
while(1){
    MeasureEchoTime();    
    SAL_TaskSleep(200); // 0.2초 대기
    }
}
*/

// external.c
//주의사항 : mcu_printf 난사시 정상작동X
#include "ultrasonic_test.h"   // 헤더 파일 포함
#include <debug.h>
#include <sal_api.h>
#include <main.h>
#include <stdint.h>
#include <gpio.h>
#include <timer.h>
#include <timer_test.h>
#define TRIGGER_PIN GPIO_GPG(23)  // 트리거 핀 (예: GPIO_GPG(6))
#define ECHO_PIN    GPIO_GPG(24)  // 에코 핀 (예: GPIO_GPG(7))
#define SOUND_SPEED_CM_PER_US 0.0343  // 음속 (cm/µs)
/////////////////////<Declaration>///////////////////////
static void Ultra_Test(void *pArg);
uint32 calculate_distance(uint32 duration_ticks);
static void ultrasonic_read_distance(void) ;
/////////////////////<Function>///////////////////////
uint32 calculate_distance(uint32 duration_ticks) {
    // 클럭 값을 마이크로초로 변환 후 거리 계산
    uint32 duration_us = duration_ticks;
    return (duration_us * SOUND_SPEED_CM_PER_US) / 2.0;  // 왕복 거리이므로 나누기 2
}

static void ultrasonic_read_distance() {
    timer_on();
    uint32 start_time = 0;
    uint32 end_time = 0;
    
     GPIO_Set(GPIO_GPC(23UL),0);
     SAL_TaskSleep(50); // 준비 시간 대기
     GPIO_Set(GPIO_GPC(23UL),1);// HIGH 신호 전송
     delay_us3(10);
     GPIO_Set(GPIO_GPC(23UL),0); // LOW 신호 전송

   
     
    while (GPIO_Get(ECHO_PIN) == 0UL);
    start_time = TIMER_GetCurrentMainCounter();  // 타이머 값 읽기

    // 에코 핀이 LOW가 될 때까지 대기 (end_time 기록)
    while (GPIO_Get(ECHO_PIN) == 1UL);
    end_time = TIMER_GetCurrentMainCounter();    // 타이머 값 읽기
    
    uint32 duration_us = end_time - start_time;
    TIMER_Disable(TIMER_CH_2);  
    // 거리 계산
   // mcu_printf("dura : 0x%08X\n",duration_us);
   // mcu_printf("tick: %d\n",duration_us);
    mcu_printf("dist : %d cm\n",(calculate_distance(duration_us)));
}

void ultrasonic_test(void *pArg)
{
    (void)pArg;
    mcu_printf("ultra test start\n");
    
    //ultrasonic_init();
    GPIO_Config(GPIO_GPG(23), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    // 입력 핀 설정
    GPIO_Config(GPIO_GPG(24), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));    
    
    while (1) {        
        ultrasonic_read_distance();
        SAL_TaskSleep(1000); // 1초 대기
    }
}
