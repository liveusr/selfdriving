################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L5_Application/source/can_common.cpp \
../L5_Application/source/controller_bt_android.cpp \
../L5_Application/source/controller_dummy.cpp \
../L5_Application/source/controller_geo.cpp \
../L5_Application/source/controller_io.cpp \
../L5_Application/source/controller_master.cpp \
../L5_Application/source/controller_motor.cpp \
../L5_Application/source/controller_sensor.cpp \
../L5_Application/source/high_level_init.cpp \
../L5_Application/source/remote.cpp \
../L5_Application/source/terminal.cpp 

OBJS += \
./L5_Application/source/can_common.o \
./L5_Application/source/controller_bt_android.o \
./L5_Application/source/controller_dummy.o \
./L5_Application/source/controller_geo.o \
./L5_Application/source/controller_io.o \
./L5_Application/source/controller_master.o \
./L5_Application/source/controller_motor.o \
./L5_Application/source/controller_sensor.o \
./L5_Application/source/high_level_init.o \
./L5_Application/source/remote.o \
./L5_Application/source/terminal.o 

CPP_DEPS += \
./L5_Application/source/can_common.d \
./L5_Application/source/controller_bt_android.d \
./L5_Application/source/controller_dummy.d \
./L5_Application/source/controller_geo.d \
./L5_Application/source/controller_io.d \
./L5_Application/source/controller_master.d \
./L5_Application/source/controller_motor.d \
./L5_Application/source/controller_sensor.d \
./L5_Application/source/high_level_init.d \
./L5_Application/source/remote.d \
./L5_Application/source/terminal.d 


# Each subdirectory must supply rules for building sources it contributes
L5_Application/source/%.o: ../L5_Application/source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\newlib" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L0_LowLevel" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\include" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L1_FreeRTOS\portable" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L2_Drivers\base" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L3_Utils\tlm" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\fat" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L4_IO\wireless" -I"C:\Users\Rutwik\SJSU_Dev\projects\FINAL_MASTER_Ver3_new_CAN\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


