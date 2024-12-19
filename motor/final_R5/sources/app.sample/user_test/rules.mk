###################################################################################################
#                                       FileName : rules.mk                                       #
###################################################################################################

MCU_BSP_APP_SAMPLE_EXTERNAL_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

VPATH += $(MCU_BSP_APP_SAMPLE_EXTERNAL_TEST_PATH)
 
# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_EXTERNAL_TEST_PATH)
 
# Sources
SRCS += external.c 
