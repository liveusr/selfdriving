################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L5_Application/examples/examples.cpp \
../L5_Application/examples/rn_xv_task.cpp 

OBJS += \
./L5_Application/examples/examples.o \
./L5_Application/examples/rn_xv_task.o 

CPP_DEPS += \
./L5_Application/examples/examples.d \
./L5_Application/examples/rn_xv_task.d 


# Each subdirectory must supply rules for building sources it contributes
L5_Application/examples/%.o: ../L5_Application/examples/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\newlib" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L0_LowLevel" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\include" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\portable" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers\base" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils\tlm" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\fat" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\wireless" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


