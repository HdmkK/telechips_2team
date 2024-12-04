// external.h
#ifndef EXTERNAL_H
#define EXTERNAL_H

#define LED_TEST_STK_SIZE               (512UL)
#include <sal_internal.h>

void Ultra_Test(void *pArg);
uint32 calculate_distance(uint32 duration_ticks);
uint32 ultrasonic_read_distance(void);
void ultrasonic_test(void); 
void LED_TEST_Task(void);
#endif // EXTERNAL_H

