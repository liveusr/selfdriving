################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../L3_Utils/tlm/src/c_tlm_binary.c \
../L3_Utils/tlm/src/c_tlm_comp.c \
../L3_Utils/tlm/src/c_tlm_stream.c \
../L3_Utils/tlm/src/c_tlm_var.c 

OBJS += \
./L3_Utils/tlm/src/c_tlm_binary.o \
./L3_Utils/tlm/src/c_tlm_comp.o \
./L3_Utils/tlm/src/c_tlm_stream.o \
./L3_Utils/tlm/src/c_tlm_var.o 

C_DEPS += \
./L3_Utils/tlm/src/c_tlm_binary.d \
./L3_Utils/tlm/src/c_tlm_comp.d \
./L3_Utils/tlm/src/c_tlm_stream.d \
./L3_Utils/tlm/src/c_tlm_var.d 


# Each subdirectory must supply rules for building sources it contributes
L3_Utils/tlm/src/%.o: ../L3_Utils/tlm/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\newlib" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L0_LowLevel" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\include" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L1_FreeRTOS\portable" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L2_Drivers\base" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L3_Utils\tlm" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\fat" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L4_IO\wireless" -I"F:\SJSU\CMPE_243\SJSU_Dev\projects\lpc1758_freertos_newCAN_NEW_Compass\L5_Application" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


