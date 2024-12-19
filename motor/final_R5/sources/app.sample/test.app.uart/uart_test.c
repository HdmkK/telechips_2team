/****************************************************************************
 *   FileName    : uart_test.c
 *   Description :
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 This source code contains confidential information of Telechips.
 Any unauthorized use without a written permission of Telechips including not limited to re-
 distribution in source or binary form is strictly prohibited.
 This source code is provided "AS IS" and nothing contained in this source code shall
 constitute any express or implied warranty of any kind, including without limitation, any warranty
 of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
 or other third party intellectual property right. No warranty is made, express or implied,
 regarding the information's accuracy, completeness, or performance.
 In no event shall Telechips be liable for any claim, damages or other liability arising from, out of
 or in connection with this source code or the use in the source code.
 This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
 between Telechips and Company.
 *
 ****************************************************************************/

#include "string.h"
#include "sal_com.h"
#include <app_cfg.h>

#include <debug.h>
#include <reg_phys.h>
#include <uart_test.h>
#include <bsp.h>


void UartJIN(void) {
  mcu_printf("start uart\n");
  mcu_printf("uart start\n");

  while (1) {
    sint32 total_bytes_read = 0;
    uint8 buffer[BUFFER_SIZE] = {0}; // 버퍼 초기화
    
    // 1. UART로 들어오는 값 확인
    while (1) {
            sint32 bytes_read = UART_Read(UART_CH2, &buffer[total_bytes_read], sizeof(buffer) - total_bytes_read-1);
            if (bytes_read <= 0) {
                break;
            }
            total_bytes_read += bytes_read;
            if (buffer[total_bytes_read - 1] == '\n' || buffer[total_bytes_read - 1] == '\0') {
                    break;
            }
        } 

    buffer[total_bytes_read] = '\0'; 

    // 2. 수신된 값 출력      
    if (total_bytes_read > 0) {
      mcu_printf("Received Data: %s\n", buffer);
    } 
    
    SAL_TaskSleep(100);
    
  }
}


void UartJIN2(void) {
  PWM_TestMotorGpio();
  
  uint32 dutyCycle = 0;           // PWM 듀티 사이클
  uint32 prevDutyCycle = 0;       // 이전 듀티 사이클
  PDMModeConfig_t ModeConfigInfo;
  ModeConfigInfo.mcPortNumber = 64UL;
  ModeConfigInfo.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1;
  ModeConfigInfo.mcClockDivide = 0UL;
  ModeConfigInfo.mcLoopCount = 0UL;
  ModeConfigInfo.mcInversedSignal = 0UL;
  ModeConfigInfo.mcPosition1 = 0UL;
  ModeConfigInfo.mcPosition2 = 0UL;
  ModeConfigInfo.mcPosition3 = 0UL;
  ModeConfigInfo.mcPosition4 = 0UL;
  ModeConfigInfo.mcOutPattern1 = 0UL;
  ModeConfigInfo.mcOutPattern2 = 0UL;
  ModeConfigInfo.mcOutPattern3 = 0UL;
  ModeConfigInfo.mcOutPattern4 = 0UL;
  ModeConfigInfo.mcMaxCount = 0UL;
  mcu_printf("start Excel\n");

  while (1) {
    sint32 total_bytes_read = 0;
    uint8 buffer[BUFFER_SIZE] = {0}; // 버퍼 초기화


    // 1. UART로 들어오는 값 확인
    while (1) {
            sint32 bytes_read = UART_Read(UART_CH2, &buffer[total_bytes_read], sizeof(buffer) - total_bytes_read-1);
            if (bytes_read <= 0) {
                break;
            }
            total_bytes_read += bytes_read;
            if (buffer[total_bytes_read - 1] == '\n' || buffer[total_bytes_read - 1] == '\0') {
                    break;
            }
        } 

    buffer[total_bytes_read] = '\0'; 
    
    // 수신 데이터 숫자로 변환
    sint32 receivedValue = atoi((const char *)buffer);     
  
    // 속도 출력 메시지
    dutyCycle = receivedValue;
    mcu_printf("Setting PWM duty cycle to: %d%%\n", dutyCycle);
    
    // 4. 듀티 사이클에 따른 모터 제어
    if (dutyCycle != prevDutyCycle) {
         if (dutyCycle > 0) {
              ModeConfigInfo.mcPeriodNanoSec1 = (1000UL * 1000U); // 1ms 주기
              ModeConfigInfo.mcDutyNanoSec1 = ((dutyCycle * ModeConfigInfo.mcPeriodNanoSec1) / 255UL);

              (void)PDM_SetConfig((uint32)PWM_CH_0, &ModeConfigInfo);
              (void)PDM_Enable((uint32)PWM_CH_0, PMM_ON);
            } else {
              (void)PDM_Disable((uint32)PWM_CH_0, PMM_ON);
            }
            prevDutyCycle = dutyCycle; // 이전 값 갱신
        }
    SAL_TaskSleep(100);
    
  }
   (void)PDM_Disable((uint32)PWM_CH_0, PMM_ON);
}

void HAB_Task(void) 
{   
    mcu_printf("HABHAB\n");
    
    static uint32   HAB_TaskID;
    static uint32   HAB_TEST_Stk[HAB_TEST_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &HAB_TaskID,
        (const uint8 *)"HAB Test Task",
        (SALTaskFunc)&UartJIN2,
        &HAB_TEST_Stk[0],
        HAB_TEST_STK_SIZE,
        SAL_PRIO_TEST,
        NULL
    );
}






