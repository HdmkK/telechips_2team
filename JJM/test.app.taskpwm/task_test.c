/*
 ***************************************************************************************************
 *                                         FileName : task_test.c
 ***************************************************************************************************
 */

#include "pdm.h"
#include "task_test.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"
#include "sal_api.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portable.h"

#define PWM_CH_0                        (0UL)
#define SWITCH1_GPIO                    GPIO_GPC(21UL)
#define SWITCH2_GPIO                    GPIO_GPC(20UL)
#define SWITCH3_GPIO                    GPIO_GPC(22UL)
#define LED1_GPIO                       GPIO_GPC(28UL)
#define LED2_GPIO                       GPIO_GPC(29UL)

/*
 ***************************************************************************************************
 *                                           GPIO
 ***************************************************************************************************
 */

static void LED_GPIO(void) {
    (void)GPIO_Config(LED1_GPIO, (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));
    return;
}

static void SWITCH_GPIO(void) {
    (void)GPIO_Config(SWITCH1_GPIO, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    (void)GPIO_Config(SWITCH2_GPIO, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    (void)GPIO_Config(SWITCH3_GPIO, (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    return;
}

static void MOTOR_GPIO(void)
{
    (void)GPIO_Config(GPIO_GPC(28UL) , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));
    (void)GPIO_Config(GPIO_GPC(29UL), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    (void)GPIO_Config(GPIO_GPC(4UL), (GPIO_FUNC(0UL) | GPIO_OUTPUT));

    (void)GPIO_Set(GPIO_GPC(29UL), 1UL);
    (void)GPIO_Set(GPIO_GPC(4UL), 0UL);
    return;
}

/*
 ***************************************************************************************************
 *                                           Task -> Switch -> LED PWM
 ***************************************************************************************************
 */

static uint32 QueueId, SwitchTaskId, MotorTaskId;
static uint32 Switch_TEST_Stk[LED_TEST_STK_SIZE], Motor_TEST_Stk[LED_TEST_STK_SIZE];

typedef struct {
    uint8 sw1_state;
    uint8 sw2_state;
    uint8 sw3_state;
} SwitchState_t;

static void SWITCH_Task(void *pArg) {
    mcu_printf("\n#Switch Run#\n");

    SwitchState_t switchState;
    uint32 dataSize = sizeof(dutyCycle);
    uint32 dutyCycle = 0;
    uint32 sw1_prev = 0;
    uint32 sw2_prev = 0;
    uint32 sw3_prev = 0;

    SWITCH_GPIO();

    while (1) {
        switchState.sw1_state = GPIO_Get(GPIO_GPC(21UL));
        switchState.sw2_state = GPIO_Get(GPIO_GPC(20UL));
        switchState.sw3_state = GPIO_Get(GPIO_GPC(22UL));

        mcu_printf("sw1 : %d, sw2 : %d, sw3 : %d\n", switchState.sw1_state, switchState.sw2_state, switchState.sw3_state);
        if (switchState.sw1_state) {
            if (dutyCycle < 100) {
                dutyCycle += 10;
                mcu_printf("Speed Up: %d%%\n", dutyCycle);
            }
        }
        if (switchState.sw2_state) {
            if (dutyCycle > 0) {
                dutyCycle -= 10;
                mcu_printf("Speed Down: %d%%\n", dutyCycle);
            }
        }
        if (switchState.sw3_state) {
            dutyCycle = 0;
            mcu_printf("Emergency Stop!\n");
        }

        if( (switchState.sw1_state && !sw1_prev) || 
            (switchState.sw2_state && !sw2_prev) || 
            (switchState.sw3_state && !sw3_prev) ) {
            SAL_QueuePut(QueueId,
                         &dutyCycle,
                         dataSize,
                         0,
                         SAL_OPT_BLOCKING);
        }

        sw1_prev = switchState.sw1_state;
        sw2_prev = switchState.sw2_state;
        sw3_prev = switchState.sw3_state;

        SAL_TaskSleep(1);
    }
}

static void MOTOR_Task(void *pArg) {
    mcu_printf("\n#Motor Run#\n");

    uint32          dutyCycle = 0;
    uint32          prevDutyCycle = 0;
    PDMModeConfig_t ModeConfigInfo;

    MOTOR_GPIO();

    (void)PDM_Init();

    ModeConfigInfo.mcPortNumber = 64UL;
    ModeConfigInfo.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1;
    ModeConfigInfo.mcClockDivide = 0UL;
    ModeConfigInfo.mcLoopCount = 0UL;''
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
        if((SAL_QueueGet(QueueId, &dutyCycle, NULL, 0, SAL_OPT_BLOCKING)) == SAL_RET_SUCCESS) {
            mcu_printf("Received dutyCycle: %d\n", dutyCycle);
        }
        mcu_printf("dutycycle2 : %d\n", dutyCycle);
        if (dutyCycle != prevDutyCycle) {
                mcu_printf("dutycycle3 : %d\n", dutyCycle);
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
        
        SAL_TaskSleep(1);
    }
}

static void MOTOR_TEST_Task(void) {
    mcu_printf("\n#Switch-Motor Task TEST#\n");

    SAL_QueueCreate( &QueueId, 
                    (const uint8 *)"Queue Test", 
                    10, 
                    sizeof( SwitchState_t ) );

    SAL_TaskCreate(&SwitchTaskId,
                   (const uint8 *)"Switch Task",
                   (SALTaskFunc)&SWITCH_Task,
                   &Switch_TEST_Stk[0], 
                   LED_TEST_STK_SIZE, 
                   8, 
                   NULL );

    SAL_TaskCreate(&MotorTaskId,
                   (const uint8 *)"Motor Task",
                   (SALTaskFunc)&MOTOR_Task,
                   &Motor_TEST_Stk[0], 
                   LED_TEST_STK_SIZE, 
                   7, 
                   NULL );

    while (1);
}

/*
 ***************************************************************************************************
 *                                           Task -> Message
 ***************************************************************************************************
 */

static void MSG1_Task(void *pArg) {
    mcu_printf("\n#Task1 Run#\n");

    while (1) {
        SAL_TaskSleep(500);
        mcu_printf("A\n");
    }
}

static void MSG2_Task(void *pArg) {
    mcu_printf("\n#Task2 Run#\n");

    while (1) {
        SAL_TaskSleep(1000);
        mcu_printf("B\n");
    }
}

static void MSG_TEST_Task(void) {
    mcu_printf("\n#Task TEST#\n");

    static uint32 MSG1_TaskID, MSG2_TaskID;
    static uint32 MSG1_TEST_Stk[LED_TEST_STK_SIZE], MSG2_TEST_Stk[LED_TEST_STK_SIZE];

    (void)SAL_TaskCreate(&MSG1_TaskID, (const uint8 *)"MSG1 Test Task", (SALTaskFunc)&MSG1_Task, &MSG1_TEST_Stk[0], LED_TEST_STK_SIZE, 7, NULL );
    (void)SAL_TaskCreate(&MSG2_TaskID, (const uint8 *)"MSG2 Test Task", (SALTaskFunc)&MSG2_Task, &MSG2_TEST_Stk[0], LED_TEST_STK_SIZE, 7, NULL );

    while (1);
}

/*
 ***************************************************************************************************
 *                                           Task -> LED
 ***************************************************************************************************
 */

static void LED1_Task(void *pArg) {
    mcu_printf("\n#Task1 Run#\n");

    (void)GPIO_Config(LED1_GPIO, (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    while (1) {
        (void)GPIO_Set(GPIO_GPC(28UL), 1UL);
        SAL_TaskSleep(500);
        (void)GPIO_Set(GPIO_GPC(28UL), 0UL);
        SAL_TaskSleep(500);
    }
}

static void LED2_Task(void *pArg) {
    mcu_printf("\n#Task2 Run#\n");

    (void)GPIO_Config(LED2_GPIO, (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    while (1) {
        (void)GPIO_Set(GPIO_GPC(29UL), 1UL);
        SAL_TaskSleep(1000);
        (void)GPIO_Set(GPIO_GPC(29UL), 0UL);
        SAL_TaskSleep(1000);
    }
}

static void LED_TEST_Task(void) {
    mcu_printf("\n#Task TEST#\n");

    static uint32 LED1_TaskID, LED2_TaskID;
    static uint32 LED1_TEST_Stk[LED_TEST_STK_SIZE], LED2_TEST_Stk[LED_TEST_STK_SIZE];

    (void)SAL_TaskCreate(&LED1_TaskID, (const uint8 *)"LED1 Test Task", (SALTaskFunc)&LED1_Task, &LED1_TEST_Stk[0], LED_TEST_STK_SIZE, 7, NULL );
    (void)SAL_TaskCreate(&LED2_TaskID, (const uint8 *)"LED2 Test Task", (SALTaskFunc)&LED2_Task, &LED2_TEST_Stk[0], LED_TEST_STK_SIZE, 7, NULL );

    while (1);
}

/*
 ***************************************************************************************************
 *                                           TASK_SelectTestCase
 ***************************************************************************************************
 */

static void MotorMotor(void *pArg) {
    mcu_printf("\n#Motor Run#\n");

    uint32          dutyCycle = 0;
    uint32          prevDutyCycle = 0;
    PDMModeConfig_t ModeConfigInfo;

    MOTOR_GPIO();

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
        if (dutyCycle > 100) {
            dutyCycle = 0;
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
        dutyCycle += 10;

        SAL_TaskSleep(500);
    }
}

static void MOTORMOTOR_Task(void) {
    mcu_printf("\n#MOTORMOTOR Task TEST#\n");

    SAL_TaskCreate(&MotorTaskId,
                   (const uint8 *)"MotorMotor Task",
                   (SALTaskFunc)&MotorMotor,
                   &Motor_TEST_Stk[0], 
                   LED_TEST_STK_SIZE, 
                   7, 
                   NULL );

    while (1);
}

/*
 ***************************************************************************************************
 *                                           TASK_SelectTestCase
 ***************************************************************************************************
 */

void TASK_SelectTestCase(uint32 uiTestCase)
{
    switch(uiTestCase)
    {
        case    1:
        {
            mcu_printf("\nRunning Motor task 1\n");
            MOTOR_TEST_Task();
            break;
        }

        case    2:
        {
            mcu_printf("\nRunning Massage task 2\n");
            MSG_TEST_Task();
            break;
        }   

        case    3:
        {
            mcu_printf("\nRunning LED task 3\n");
            LED_TEST_Task();
            break;
        }
        
        case    4:
        {
            mcu_printf("\nRunning MOTORMOTOR task 3\n");
            MOTORMOTOR_Task();
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

