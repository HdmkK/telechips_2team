// external.c
#include "external.h"   // 헤더 파일 포함
#include <debug.h>
#include <sal_api.h>
#include <main.h>
#include <stdint.h>
#include <gpio.h>
#include <timer.h>
#include <timer_test.h>
#define TRIGGER_PIN GPIO_GPG(6) 
#define ECHO_PIN    GPIO_GPG(7)  
#define SOUND_SPEED_CM_PER_US 0.0343  // 음속 (cm/µs)

/////////////////////<Function>///////////////////////

uint32 calculate_distance(uint32 duration_ticks) {
    // 클럭 값을 마이크로초로 변환 후 거리 계산
    uint32 duration_us = duration_ticks;
    return (duration_us * SOUND_SPEED_CM_PER_US) / 2.0;  // 왕복 거리이므로 나누기 2
}

uint32 ultrasonic_read_distance() {
    timer_on();
    uint32 start_time = 0;
    uint32 end_time = 0;
    
    
    SAL_WriteReg(0x00000000UL, 0x1b935180UL);
    SAL_TaskSleep(50);
    // 트리거 신호 (10μs 동안 HIGH)
    SAL_WriteReg(0x00000040UL, 0x1b935180UL);
    delay_us3(10);
   
    SAL_WriteReg(0x00000000UL, 0x1b935180UL);
   
     
    while (GPIO_Get(ECHO_PIN) == 0UL);
    start_time = TIMER_GetCurrentMainCounter();  // 타이머 값 읽기

    // 에코 핀이 LOW가 될 때까지 대기 (end_time 기록)
    while (GPIO_Get(ECHO_PIN) == 1UL);
    end_time = TIMER_GetCurrentMainCounter();    // 타이머 값 읽기
    
    uint32 duration_us = end_time - start_time;
    TIMER_Disable(TIMER_CH_2);  
    // 거리 계산
    mcu_printf("dist : %d cm\n",(calculate_distance(duration_us)));
    
    return calculate_distance(duration_us);
}

void Ultra_Test(void *pArg)
{
    uint32 ultra = 0;
    GPIO_Config(GPIO_GPC(24), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    GPIO_Config(GPIO_GPC(25), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));  
    mcu_printf("Ultrasonic sensor enabled.\n");
    
    while(1){
    ultra = ultrasonic_read_distance();
    if (ultra >60 && ultra <= 80) {

    } else if (ultra >40 && ultra <= 60) {
        dutyCycle-=50 ; // 초음파 값 60~40
    } else if (ultra > 30 && ultra <= 40) {
        dutyCycle-=70; // 초음파 값 40~30
    } else if (ultra >20 && ultra <= 30) {
        dutyCycle-=100 ; // 초음파 값 30~20
    } else if (ultra > 0 && ultra <= 20) {
        dutyCycle = 0; // 초음파 값 20~0
    } 
    
    
    
    mcu_printf("ultra test start\n");
    SAL_TaskSleep(00);
    }
    /*
    //ultrasonic_init();
    GPIO_Config(GPIO_GPG(6), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    // 입력 핀 설정
    GPIO_Config(GPIO_GPG(7), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));    
    
    while (1) {        
        ultrasonic_read_distance();
        SAL_TaskSleep(1000); // 1초 대기
    }*/
}


void LED_TEST_Task(void) 
{   
    mcu_printf("!!@#Ultra task test !!!@#@\n");
    
    static uint32   LED_TaskID;
    static uint32   LED_TEST_Stk[LED_TEST_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &LED_TaskID,
        (const uint8 *)"Ultra Test Task",
        (SALTaskFunc)&Ultra_Test,
        &LED_TEST_Stk[0],
        LED_TEST_STK_SIZE,
        SAL_PRIO_TEST,
        NULL
    );
}















void ultrasonic_test(void) // for console test
{

   mcu_printf("ultra console test start\n");

    //ultrasonic_init();
    GPIO_Config(GPIO_GPG(6), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    // 입력 핀 설정
    GPIO_Config(GPIO_GPG(7), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
 
    while (1) {
        ultrasonic_read_distance();
        SAL_TaskSleep(1000); // 1초 대기
    }
}

