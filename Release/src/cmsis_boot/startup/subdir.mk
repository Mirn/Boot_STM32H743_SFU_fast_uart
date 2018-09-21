################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/cmsis_boot/startup/startup_stm32h743xx.S 

OBJS += \
./src/cmsis_boot/startup/startup_stm32h743xx.o 

S_UPPER_DEPS += \
./src/cmsis_boot/startup/startup_stm32h743xx.d 


# Each subdirectory must supply rules for building sources it contributes
src/cmsis_boot/startup/%.o: ../src/cmsis_boot/startup/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -mcpu=cortex-m7 -O3 -fmessage-length=0 -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -Wunused -Wuninitialized -Wall -Wextra -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -Wno-sign-compare -x assembler-with-cpp -DSTM32H743xx -DUSE_IOEXPANDER -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


