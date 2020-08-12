################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../note-c/n_atof.c \
../note-c/n_b64.c \
../note-c/n_cjson.c \
../note-c/n_cjson_helpers.c \
../note-c/n_const.c \
../note-c/n_ftoa.c \
../note-c/n_helpers.c \
../note-c/n_hooks.c \
../note-c/n_i2c.c \
../note-c/n_request.c \
../note-c/n_serial.c \
../note-c/n_str.c 

OBJS += \
./note-c/n_atof.o \
./note-c/n_b64.o \
./note-c/n_cjson.o \
./note-c/n_cjson_helpers.o \
./note-c/n_const.o \
./note-c/n_ftoa.o \
./note-c/n_helpers.o \
./note-c/n_hooks.o \
./note-c/n_i2c.o \
./note-c/n_request.o \
./note-c/n_serial.o \
./note-c/n_str.o 

C_DEPS += \
./note-c/n_atof.d \
./note-c/n_b64.d \
./note-c/n_cjson.d \
./note-c/n_cjson_helpers.d \
./note-c/n_const.d \
./note-c/n_ftoa.d \
./note-c/n_helpers.d \
./note-c/n_hooks.d \
./note-c/n_i2c.d \
./note-c/n_request.d \
./note-c/n_serial.d \
./note-c/n_str.d 


# Each subdirectory must supply rules for building sources it contributes
note-c/n_atof.o: ../note-c/n_atof.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_atof.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_b64.o: ../note-c/n_b64.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_b64.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_cjson.o: ../note-c/n_cjson.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_cjson.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_cjson_helpers.o: ../note-c/n_cjson_helpers.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_cjson_helpers.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_const.o: ../note-c/n_const.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_const.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_ftoa.o: ../note-c/n_ftoa.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_ftoa.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_helpers.o: ../note-c/n_helpers.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_helpers.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_hooks.o: ../note-c/n_hooks.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_hooks.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_i2c.o: ../note-c/n_i2c.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_i2c.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_request.o: ../note-c/n_request.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_request.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_serial.o: ../note-c/n_serial.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_serial.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
note-c/n_str.o: ../note-c/n_str.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L432xx -c -I../Drivers/CMSIS/Include -I../BME680_driver -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../note-c -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"note-c/n_str.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

