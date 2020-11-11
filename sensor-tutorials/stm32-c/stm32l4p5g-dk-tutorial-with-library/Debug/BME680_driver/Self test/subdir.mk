################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BME680_driver/Self\ test/bme680_selftest.c 

OBJS += \
./BME680_driver/Self\ test/bme680_selftest.o 

C_DEPS += \
./BME680_driver/Self\ test/bme680_selftest.d 


# Each subdirectory must supply rules for building sources it contributes
BME680_driver/Self\ test/bme680_selftest.o: ../BME680_driver/Self\ test/bme680_selftest.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L4P5xx -DUSE_HAL_DRIVER -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../note-c -I../BME680_driver -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"BME680_driver/Self test/bme680_selftest.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

