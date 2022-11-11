# Sensor Tutorial for the STM32L4PG Discovery Board

This is the complete STM32CubeIDE project for the [STM32L4PG sensor tutorial](https://dev.blues.io/guides-and-tutorials/collecting-sensor-data/notecarrier-a/stm32-discovery/c-cpp-stm32cube/) wihch uses a BME680 sensor, Notecarrier A and a Notecard to send sensor readings to Notehub.

Before compiling and uploading the code to your development kit, you will need to set the `PRODUCT_UID`  #define in `Core/Src/main.c` to the value used when you created your Notehub project for the tutorial.