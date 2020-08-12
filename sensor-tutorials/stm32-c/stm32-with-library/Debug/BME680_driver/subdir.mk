################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BME680_driver/bme680.c 

OBJS += \
./BME680_driver/bme680.o 

C_DEPS += \
./BME680_driver/bme680.d 


# Each subdirectory must supply rules for building sources it contributes
BME680_driver/bme680.o: ../BME680_driver/bme680.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"BME680_driver/bme680.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

