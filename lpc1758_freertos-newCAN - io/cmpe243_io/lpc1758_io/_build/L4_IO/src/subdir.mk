################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L4_IO/src/gpio.cpp \
../L4_IO/src/io_source.cpp \
../L4_IO/src/storage.cpp 

OBJS += \
./L4_IO/src/gpio.o \
./L4_IO/src/io_source.o \
./L4_IO/src/storage.o 

CPP_DEPS += \
./L4_IO/src/gpio.d \
./L4_IO/src/io_source.d \
./L4_IO/src/storage.d 


# Each subdirectory must supply rules for building sources it contributes
L4_IO/src/%.o: ../L4_IO/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\newlib" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L0_LowLevel" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS\include" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS\portable" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L2_Drivers" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L2_Drivers\base" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L3_Utils" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L3_Utils\tlm" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO\fat" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO\wireless" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


