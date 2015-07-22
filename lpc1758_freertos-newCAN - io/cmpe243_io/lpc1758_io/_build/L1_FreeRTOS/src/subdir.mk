################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../L1_FreeRTOS/src/croutine.c \
../L1_FreeRTOS/src/event_groups.c \
../L1_FreeRTOS/src/list.c \
../L1_FreeRTOS/src/queue.c \
../L1_FreeRTOS/src/tasks.c \
../L1_FreeRTOS/src/timers.c 

OBJS += \
./L1_FreeRTOS/src/croutine.o \
./L1_FreeRTOS/src/event_groups.o \
./L1_FreeRTOS/src/list.o \
./L1_FreeRTOS/src/queue.o \
./L1_FreeRTOS/src/tasks.o \
./L1_FreeRTOS/src/timers.o 

C_DEPS += \
./L1_FreeRTOS/src/croutine.d \
./L1_FreeRTOS/src/event_groups.d \
./L1_FreeRTOS/src/list.d \
./L1_FreeRTOS/src/queue.d \
./L1_FreeRTOS/src/tasks.d \
./L1_FreeRTOS/src/timers.d 


# Each subdirectory must supply rules for building sources it contributes
L1_FreeRTOS/src/%.o: ../L1_FreeRTOS/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\newlib" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L0_LowLevel" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS\include" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L1_FreeRTOS\portable" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L2_Drivers" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L2_Drivers\base" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L3_Utils" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L3_Utils\tlm" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO\fat" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L4_IO\wireless" -I"C:\Users\Dell\git\cmpe243_io\lpc1758_io\L5_Application" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


