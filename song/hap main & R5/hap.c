/****************************************************************************
 *   FileName    : hap.c
 *   Description : haphaphaphaphaphaphap
 ****************************************************************************

 ****************************************************************************/

#include <hap.h>
//uint32 value=0;

/****************************************************************
Ultra func
****************************************************************/


uint32 calculate_distance(uint32 duration_ticks) {
    // 클럭 값을 마이크로초로 변환 후 거리 계산
    uint32 duration_us = duration_ticks;
    return (duration_us * SOUND_SPEED_CM_PER_US) / 2.0;  // 왕복 거리이므로 나누기 2
}

uint32 ultrasonic_read_distance() {
    timer_on();
    uint32 start_time = 0;
    uint32 end_time = 0;
    uint32 timeout_counter = 0;
    
    GPIO_Set(TRIGGER_PIN, 1);   // HIGH로 설정
    delay_us3(100);              // 100μs 유지
    GPIO_Set(TRIGGER_PIN, 0);   // 다시 LOW로 설정
    
     
    while (GPIO_Get(ECHO_PIN) == 0UL){
    timeout_counter++;
    mcu_printf("hanbin");
    delay_us3(10); 
    if (timeout_counter > 300) {           
	mcu_printf("Timeout waiting\n");
	break;
    }
    }
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


void Ultra_Test(void)
{
    uint32 ultra = 0;
    GPIO_Config(GPIO_GPC(24), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    GPIO_Config(GPIO_GPC(25), (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));  
    mcu_printf("Ultrasonic sensor enabled.\n");
    
    SAL_TaskSleep(500);
    
    
    while(1){
    ultra = ultrasonic_read_distance();
    
    
    if (ultra >60 && ultra <= 80) {
    mcu_printf("1\n");
    } else if (ultra >40 && ultra <= 60) {
        mcu_printf("2\n");
    } else if (ultra > 30 && ultra <= 40) {
        mcu_printf("3\n");
    } else if (ultra >20 && ultra <= 30) {
        mcu_printf("4\n");
    } else if (ultra > 0 && ultra <= 20) {
        mcu_printf("5\n");
    } 

    SAL_TaskSleep(200);
    }
}






/****************************************************************
UART + PWM func
****************************************************************/

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
        
    // 5. 현재 듀티 사이클 값을 UART로 전송
    char send_buffer[4] = {0};
    snprintf(send_buffer, sizeof(send_buffer), "%ld\n", dutyCycle);
    UART_Write(UART_CH2, (uint8 *)send_buffer, strlen(send_buffer)+1);

        
        
        
    SAL_TaskSleep(200);
    
  }
   (void)PDM_Disable((uint32)PWM_CH_0, PMM_ON);
}

/****************************************************************
Create_Test_Task
****************************************************************/


void Create_TEST_Task(void) 
{   
    mcu_printf("Task create\n");
    
    static uint32   Ultra_TaskID, HAB_TaskID;
    static uint32   Ultra_TEST_Stk[Ultra_TEST_STK_SIZE];
    static uint32   HAB_TEST_Stk[HAB_TEST_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &Ultra_TaskID,
        (const uint8 *)"Ultra Test Task",
        (SALTaskFunc)&Ultra_Test,
        &Ultra_TEST_Stk[0],
        Ultra_TEST_STK_SIZE,
        SAL_PRIO_TEST,
        NULL
    );
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







