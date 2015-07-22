################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../L1_FreeRTOS/MemMang/heap_1.c \
../L1_FreeRTOS/MemMang/heap_2.c \
../L1_FreeRTOS/MemMang/heap_3.c \
../L1_FreeRTOS/MemMang/heap_4.c 

OBJS += \
./L1_FreeRTOS/MemMang/heap_1.o \
./L1_FreeRTOS/MemMang/heap_2.o \
./L1_FreeRTOS/MemMang/heap_3.o \
./L1_FreeRTOS/MemMang/heap_4.o 

C_DEPS += \
./L1_FreeRTOS/MemMang/heap_1.d \
./L1_FreeRTOS/MemMang/heap_2.d \
./L1_FreeRTOS/MemMang/heap_3.d \
./L1_FreeRTOS/MemMang/heap_4.d 


# Each subdirectory must supply rules for building sources it contributes
L1_FreeRTOS/MemMang/%.o: ../L1_FreeRTOS/MemMang/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\newlib" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L0_LowLevel" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L1_FreeRTOS" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L1_FreeRTOS\include" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L1_FreeRTOS\portable" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L2_Drivers" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L2_Drivers\base" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L3_Utils" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L3_Utils\tlm" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L4_IO" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L4_IO\fat" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L4_IO\wireless" -I"C:\Users\Mitesh\git\lpc1758_freertos-newCAN-demo\lpc1758_freertos-newCAN-demo\lpc1758_freertos.git\L5_Application" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


