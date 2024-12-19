/*
 ***************************************************************************************************
 *                                         FileName : pwm_test.h
 ***************************************************************************************************
 */

#if !defined(PWM_TEST_HEADER)
#define PWM_TEST_HEADER
#include "gpio.h"
#include "pdm.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                           PWM_SelectTestCase
*
* @param test case number
* @return
*
* Notes
*
***************************************************************************************************
*/

void PWM_SelectTestCase(uint32 uiTestCase);
void PWM_TestSleepForSec(uint32 uiSec);
void PWM_TestSetGpio(void);
void PWM_TestMotorGpio(void);
void PWM_TestSwitchGpio(void);
void PWM_TestLED(void);
void PWM_SwitchCtrl(void);
void PWM_SelectTestCase(uint32 uiTestCase);






#endif
