/****************************************************************************
 *   FileName    : uart_test.h
 *   Description : Demo Application for console function
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


#include <uart.h>
#include <pdm_test.h>
#include <pwm_test.h>
#include <stdlib.h>
#include <sal_internal.h>

#define HAB_TEST_STK_SIZE              (512UL)
#define BUFFER_SIZE 100
#define PWM_CH_0                        (0UL)
void UartJIN(void);
void UartJIN2(void);
void HAB_Task(void);

