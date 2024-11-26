/*
 ***************************************************************************************************
 *                                         FileName : pwm_test.c
 ***************************************************************************************************
 */

#include "pdm.h"
#include "pwm_test.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"

/*
 ***************************************************************************************************
 *                                           PWM_TestSleepForSec
 ***************************************************************************************************
 */

#define PWM_CH_0                        (0UL)

static void PWM_TestSleepForSec(uint32 uiSec)
{
    (void)SAL_TaskSleep(uiSec*(1000UL));

    return;
}

/*
 ***************************************************************************************************
 *                                           PWM_TestSetGpio
 ***************************************************************************************************
 */

static void PWM_TestSetGpio(void)
{
    (void)GPIO_Config(GPIO_GPC(28UL) , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));

    return;
}

/*
 ***************************************************************************************************
 *                                           PWM_TestMotorGpio
 ***************************************************************************************************
 */

static void PWM_TestMotorGpio(void)
{
    (void)GPIO_Config(GPIO_GPC(28UL) , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));

    (void)GPIO_Config(GPIO_GPC(29UL), (GPIO_FUNC(0UL) | GPIO_OUTPUT)); // IN1
    (void)GPIO_Config(GPIO_GPC(4UL), (GPIO_FUNC(0UL) | GPIO_OUTPUT)); // IN2
    // Forward
    (void)GPIO_Set(GPIO_GPC(29UL), 1UL);
    (void)GPIO_Set(GPIO_GPC(4UL), 0UL);
    return;
}

/*
 ***************************************************************************************************
 *                                           PWM_TestSwitchGpio
 ***************************************************************************************************
 */

static void PWM_TestSwitchGpio(void)
{
    (void)GPIO_Config(GPIO_GPC(21), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    (void)GPIO_Config(GPIO_GPC(20), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    (void)GPIO_Config(GPIO_GPC(22), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));

    return;
}

/*
 ***************************************************************************************************
 *                                           PWM_TestLED
 ***************************************************************************************************
 */

static void PWM_TestLED(void) {
    uint32          dutyCycle;
    uint32          loopControl;
    PDMModeConfig_t ModeConfigInfo;

    dutyCycle = 0;
    loopControl = 1UL;

    mcu_printf("\n#Start PWM LED Brightness Control#\n");

    PWM_TestSleepForSec(1);
    
    PWM_TestSetGpio();
    (void)PDM_Init();

    PWM_TestSleepForSec(1);

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

    while (loopControl > 0UL)
    {
        mcu_printf("\n#Duty Cycle : %d%%#\n", dutyCycle);

        if(dutyCycle == 0UL) {
            (void)PDM_Disable((uint32)PWM_CH_0, PMM_ON);
        }
        else {
            ModeConfigInfo.mcPeriodNanoSec1 = 1000UL * 1000UL;
            ModeConfigInfo.mcDutyNanoSec1 = (dutyCycle * ModeConfigInfo.mcPeriodNanoSec1) / 100UL;

            (void)PDM_SetConfig((uint32)PWM_CH_0, (PDMModeConfig_t *)&ModeConfigInfo);
            (void)PDM_Enable((uint32)PWM_CH_0, PMM_ON);
        }

        PWM_TestSleepForSec(1UL);
        
        dutyCycle += 10;

        if(dutyCycle > 100UL)
        {
            dutyCycle = 0;
            break;
        }
    }

    (void)PDM_Disable((uint32)PWM_CH_0, PMM_ON);
    mcu_printf("\n#End PWM LED Brightness Control#\n");

    return;
}

/*
 ***************************************************************************************************
 *                                           PWM_SwitchCtrl
 ***************************************************************************************************
 */

static void PWM_SwitchCtrl(void) {
    uint32          dutyCycle = 0;
    uint32          prevDutyCycle = 0;
    PDMModeConfig_t ModeConfigInfo;

    uint32 sw1_state = 0;
    uint32 sw2_state = 0;
    uint32 sw3_state = 0;

    uint32 sw1_prev = 0;
    uint32 sw2_prev = 0;
    uint32 sw3_prev = 0;

    mcu_printf("\n#Start PWM Motor Control with Switch#\n");

    PWM_TestMotorGpio();
    PWM_TestSwitchGpio();

    (void)PDM_Init();

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

    while (1) {
        sw1_state = GPIO_Get(GPIO_GPC(21UL));
        sw2_state = GPIO_Get(GPIO_GPC(20UL));
        sw3_state = GPIO_Get(GPIO_GPC(22UL));

        if (sw1_state && !sw1_prev) {
            if (dutyCycle < 100) {
                dutyCycle += 10;
                mcu_printf("Speed Up: %d%%\n", dutyCycle);
            }
        }
        if (sw2_state && !sw2_prev) {
            if (dutyCycle > 0) {
                dutyCycle -= 10;
                mcu_printf("Speed Down: %d%%\n", dutyCycle);
            }
        }
        if (sw3_state && !sw3_prev) {
            dutyCycle = 0;
            mcu_printf("Emergency Stop!\n");
        }

        if (dutyCycle != prevDutyCycle) {
            if (dutyCycle > 0) {
                ModeConfigInfo.mcPeriodNanoSec1 = (1000UL * 1000U);
                ModeConfigInfo.mcDutyNanoSec1 = ((dutyCycle * ModeConfigInfo.mcPeriodNanoSec1) / 100UL);

                (void)PDM_SetConfig((uint32)PWM_CH_0, &ModeConfigInfo);
                (void)PDM_Enable((uint32)PWM_CH_0, PMM_ON);
            } else {
                (void)PDM_Disable((uint32)PWM_CH_0, PMM_ON);
            }
            
            prevDutyCycle = dutyCycle;
        }


        sw1_prev = sw1_state;
        sw2_prev = sw2_state;
        sw3_prev = sw3_state;

        PWM_TestSleepForSec(0.1);
    }
}

/*
 ***************************************************************************************************
 *                                           PWM_SelectTestCase
 ***************************************************************************************************
 */
void PWM_SelectTestCase(uint32 uiTestCase)
{
    switch(uiTestCase)
    {
        case    1:
        {
            PWM_TestLED();
            break;
        }

        case    2:
        {
            PWM_SwitchCtrl();
            break;
        }   

        case    3:
        {
            
            break;
        }

        default :
        {
            mcu_printf("\n== Invaild Test Case ==\n");
            break;
        }
    }

    return;
}

