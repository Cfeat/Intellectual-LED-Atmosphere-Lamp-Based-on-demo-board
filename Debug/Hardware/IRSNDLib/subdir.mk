################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hardware/IRSNDLib/irsnd.c 

OBJS += \
./Hardware/IRSNDLib/irsnd.o 

C_DEPS += \
./Hardware/IRSNDLib/irsnd.d 


# Each subdirectory must supply rules for building sources it contributes
Hardware/IRSNDLib/%.o Hardware/IRSNDLib/%.su Hardware/IRSNDLib/%.cyclo: ../Hardware/IRSNDLib/%.c Hardware/IRSNDLib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L433xx -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hardware-2f-IRSNDLib

clean-Hardware-2f-IRSNDLib:
	-$(RM) ./Hardware/IRSNDLib/irsnd.cyclo ./Hardware/IRSNDLib/irsnd.d ./Hardware/IRSNDLib/irsnd.o ./Hardware/IRSNDLib/irsnd.su

.PHONY: clean-Hardware-2f-IRSNDLib

