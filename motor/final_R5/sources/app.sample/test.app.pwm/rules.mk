###################################################################################################
#                                       FileName : rules.mk                                       #
###################################################################################################

MCU_BSP_APP_SAMPLE_PWM_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_PWM_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_PWM_TEST_PATH)

# Sources
SRCS += pwm_test.c
