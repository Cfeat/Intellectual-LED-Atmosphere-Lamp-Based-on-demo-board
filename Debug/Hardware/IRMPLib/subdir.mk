################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hardware/IRMPLib/irmp.c \
../Hardware/IRMPLib/irmpextlog.c 

OBJS += \
./Hardware/IRMPLib/irmp.o \
./Hardware/IRMPLib/irmpextlog.o 

C_DEPS += \
./Hardware/IRMPLib/irmp.d \
./Hardware/IRMPLib/irmpextlog.d 


# Each subdirectory must supply rules for building sources it contributes
Hardware/IRMPLib/%.o Hardware/IRMPLib/%.su Hardware/IRMPLib/%.cyclo: ../Hardware/IRMPLib/%.c Hardware/IRMPLib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L433xx -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Hardware-2f-IRMPLib

clean-Hardware-2f-IRMPLib:
	-$(RM) ./Hardware/IRMPLib/irmp.cyclo ./Hardware/IRMPLib/irmp.d ./Hardware/IRMPLib/irmp.o ./Hardware/IRMPLib/irmp.su ./Hardware/IRMPLib/irmpextlog.cyclo ./Hardware/IRMPLib/irmpextlog.d ./Hardware/IRMPLib/irmpextlog.o ./Hardware/IRMPLib/irmpextlog.su

.PHONY: clean-Hardware-2f-IRMPLib

