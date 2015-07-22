################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L2_Drivers/base/char_dev.cpp \
../L2_Drivers/base/i2c_base.cpp \
../L2_Drivers/base/uart_dev.cpp 

OBJS += \
./L2_Drivers/base/char_dev.o \
./L2_Drivers/base/i2c_base.o \
./L2_Drivers/base/uart_dev.o 

CPP_DEPS += \
./L2_Drivers/base/char_dev.d \
./L2_Drivers/base/i2c_base.d \
./L2_Drivers/base/uart_dev.d 


# Each subdirectory must supply rules for building sources it contributes
L2_Drivers/base/%.o: ../L2_Drivers/base/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\newlib" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L0_LowLevel" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\include" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\portable" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers\base" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils\tlm" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\fat" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\wireless" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


