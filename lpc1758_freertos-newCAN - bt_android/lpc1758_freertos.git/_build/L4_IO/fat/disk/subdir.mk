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
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\newlib" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L0_LowLevel" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L1_FreeRTOS" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L1_FreeRTOS\include" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L1_FreeRTOS\portable" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L2_Drivers" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L2_Drivers\base" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L3_Utils" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L3_Utils\tlm" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L4_IO" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L4_IO\fat" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L4_IO\wireless" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L5_Application" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

L4_IO/fat/disk/%.o: ../L4_IO/fat/disk/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\newlib" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L0_LowLevel" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L1_FreeRTOS" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L1_FreeRTOS\include" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L1_FreeRTOS\portable" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L2_Drivers" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L2_Drivers\base" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L3_Utils" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L3_Utils\tlm" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L4_IO" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L4_IO\fat" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L4_IO\wireless" -I"C:\SJSU_Dev_243_CAN\projects\lpc1758_freertos-newCAN - bt_android\lpc1758_freertos.git\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


