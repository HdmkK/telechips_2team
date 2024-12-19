/****************************************************************************
 *   FileName    : Motor_System.h
 *   Description : Motor_System head file
 ****************************************************************************/


#ifndef Motor_System_H
#define Motor_System_H

#include <stdio.h>
#include <uart.h>
#include <pdm_test.h>
#include <pwm_test.h>
#include <stdlib.h>
#include <sal_internal.h>
#include <string.h>
#include <sal_com.h>
#include <app_cfg.h>
#include <sal_api.h>
#include <main.h>
#include <stdint.h>
#include <gpio.h>
#include <timer.h>
#include <timer_test.h>
#include <debug.h>
#include <reg_phys.h>
#include <bsp.h>

#define TRIGGER_PIN GPIO_GPC(24) 
#define ECHO_PIN    GPIO_GPC(25)  
#define SOUND_SPEED_CM_PER_US 0.0343  // 음속 (cm/µs)
#define Ultra_main_STK_SIZE             (512UL)
#define Uart_PWM_main_STK_SIZE             (512UL)
#define BUFFER_SIZE 100
#define PWM_CH_0                        (0UL)

uint32 calculate_distance(uint32 duration_ticks);
uint32 ultrasonic_read_distance(void);
void UltraSonic_main(void);
void Uart_PWM(void);
void Create_Motor_System_Task(void); 


#endif // HAP_H




