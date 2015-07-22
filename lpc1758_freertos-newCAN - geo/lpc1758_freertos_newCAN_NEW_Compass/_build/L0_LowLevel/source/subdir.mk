################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L0_LowLevel/source/low_level_init.cpp \
../L0_LowLevel/source/lpc_sys.cpp \
../L0_LowLevel/source/startup.cpp \
../L0_LowLevel/source/sys_clock.cpp 

C_SRCS += \
../L0_LowLevel/source/core_cm3.c \
../L0_LowLevel/source/lpc_peripherals.c \
../L0_LowLevel/source/uart0_min.c 

OBJS += \
./L0_LowLevel/source/core_cm3.o \
./L0_LowLevel/source/low_level_init.o \
./L0_LowLevel/source/lpc_peripherals.o \
./L0_LowLevel/source/lpc_sys.o \
./L0_LowLevel/source/startup.o \
./L0_LowLevel/source/sys_clock.o \
./L0_LowLevel/source/uart0_min.o 

C_DEPS += \
./L0_LowLevel/source/core_cm3.d \
./L0_LowLevel/source/lpc_peripherals.d \
./L0_LowLevel/source/uart0_min.d 

CPP_DEPS += \
./L0_LowLevel/source/low_level_init.d \
./L0_LowLevel/source/lpc_sys.d \
./L0_LowLevel/source/startup.d \
./L0_LowLevel/source/sys_clock.d 


# Each subdirectory must supply rules for building sources it contributes
L0_LowLevel/source/%.o: ../L0_LowLevel/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\newlib" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L0_LowLevel" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\include" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\portable" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers\base" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils\tlm" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\fat" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\wireless" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L5_Application" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

L0_LowLevel/source/%.o: ../L0_LowLevel/source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\newlib" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L0_LowLevel" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\include" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\portable" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers\base" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils\tlm" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\fat" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\wireless" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


