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
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\newlib" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L0_LowLevel" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS\include" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS\portable" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L2_Drivers" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L2_Drivers\base" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L3_Utils" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L3_Utils\tlm" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO\fat" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO\wireless" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L5_Application" -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


