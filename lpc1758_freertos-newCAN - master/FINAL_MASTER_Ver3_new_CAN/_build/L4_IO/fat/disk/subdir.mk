################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L4_IO/fat/disk/spi_flash.cpp 

C_SRCS += \
../L4_IO/fat/disk/diskio.c \
../L4_IO/fat/disk/sd.c 

OBJS += \
./L4_IO/fat/disk/diskio.o \
./L4_IO/fat/disk/sd.o \
./L4_IO/fat/disk/spi_flash.o 

C_DEPS += \
./L4_IO/fat/disk/diskio.d \
./L4_IO/fat/disk/sd.d 

CPP_DEPS += \
./L4_IO/fat/disk/spi_flash.d 


# Each subdirectory must supply rules for building sources it contributes
L4_IO/fat/disk/%.o: ../L4_IO/fat/disk/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\newlib" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L0_LowLevel" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\include" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\portable" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers\base" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils\tlm" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\fat" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\wireless" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L5_Application" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

L4_IO/fat/disk/%.o: ../L4_IO/fat/disk/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\newlib" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L0_LowLevel" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\include" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\portable" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers\base" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils\tlm" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\fat" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\wireless" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


