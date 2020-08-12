/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "note.h"

#include "bme680.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define myProductUID "com.your-company.your-project"
#define DELAY_PERIOD    (5*60000)       // 5 minutes

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// Data used for Notecard I/O functions
static char serialInterruptBuffer[1];
static volatile size_t serialFillIndex = 0;
static volatile size_t serialDrainIndex = 0;
static uint32_t serialOverruns = 0;
static char serialBuffer[512];
bool uart1Initialized = false;

struct bme680_dev gas_sensor;
uint8_t i2c_reading_buf[100];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

void delay(uint32_t ms);
long unsigned int millis();

void noteSerialReset(void);
void noteSerialTransmit(uint8_t *text, size_t len, bool flush);
bool noteSerialAvailable(void);
char noteSerialReceive(void);
size_t noteDebugSerialOutput(const char *message);

// BME680 Forward Declarations
void user_delay_ms(uint32_t period);
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  gas_sensor.dev_id = BME680_I2C_ADDR_SECONDARY;
  gas_sensor.intf = BME680_I2C_INTF;
  gas_sensor.read = user_i2c_read;
  gas_sensor.write = user_i2c_write;
  gas_sensor.delay_ms = user_delay_ms;
  gas_sensor.amb_temp = 25;

  int8_t rslt = BME680_OK;
  rslt = bme680_init(&gas_sensor);

  if (rslt != BME680_OK)
  {
	uint8_t bme_msg[] = "BME680 Initialization Error\r\n";
	HAL_UART_Transmit(&huart2, bme_msg, sizeof(bme_msg), 10);
  }
  else
  {
	uint8_t bme_msg[] = "BME680 Initialized and Ready\r\n";
	HAL_UART_Transmit(&huart2, bme_msg, sizeof(bme_msg), 10);
  }

  gas_sensor.tph_sett.os_hum = BME680_OS_2X;
  gas_sensor.tph_sett.os_pres = BME680_OS_4X;
  gas_sensor.tph_sett.os_temp = BME680_OS_8X;
  gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

  gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
  gas_sensor.gas_sett.heatr_temp = 320;
  gas_sensor.gas_sett.heatr_dur = 150;

  gas_sensor.power_mode = BME680_FORCED_MODE;

  uint8_t set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL
	        | BME680_GAS_SENSOR_SEL;

  rslt = bme680_set_sensor_settings(set_required_settings, &gas_sensor);
  rslt = bme680_set_sensor_mode(&gas_sensor);

  uint16_t meas_period;
  bme680_get_profile_dur(&meas_period, &gas_sensor);

  struct bme680_field_data data;

  NoteSetFn(malloc, free, delay, millis);
  NoteSetFnSerial(noteSerialReset, noteSerialTransmit, noteSerialAvailable, noteSerialReceive);

  J *req = NoteNewRequest("service.set");
  JAddStringToObject(req, "product", myProductUID);
  JAddStringToObject(req, "mode", "continuous");

  NoteRequest(req);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	// READ FROM BME680
	user_delay_ms(meas_period);
    rslt = bme680_get_sensor_data(&data, &gas_sensor);

    sprintf((char *)i2c_reading_buf,
    		"T: %u.%u degC, H %u.%u %%rH\r\n",
			(unsigned int)data.temperature / 100,
			(unsigned int)data.temperature % 100,
			(unsigned int)data.humidity / 1000,
			(unsigned int)data.humidity % 1000);
    HAL_UART_Transmit(&huart2, i2c_reading_buf, strlen((char *)i2c_reading_buf), HAL_MAX_DELAY);

    J *req = NoteNewRequest("note.add");
	if (req != NULL) {
		JAddStringToObject(req, "file", "sensors.qo");
		JAddBoolToObject(req, "start", true);
		J *body = JCreateObject();
		if (body != NULL) {
			JAddNumberToObject(body, "temp", data.temperature / 100.0);
			JAddNumberToObject(body, "humidity", data.humidity / 1000.0);
			JAddNumberToObject(body, "pressure", data.pressure / 1000.0);
			if (data.status & BME680_GASM_VALID_MSK)
				JAddNumberToObject(body, "gas", data.gas_resistance / 1000.0);
			JAddItemToObject(req, "body", body);
		}
		NoteRequest(req);
	}


	delay(DELAY_PERIOD);

	if (gas_sensor.power_mode == BME680_FORCED_MODE) {
		rslt = bme680_set_sensor_mode(&gas_sensor);
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration 
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00707CBB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  if (uart1Initialized)
    return;
  uart1Initialized = true;

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  // Reset our buffer management
  serialFillIndex = serialDrainIndex = serialOverruns = 0;

  // Unused, but included for documentation
  ((void)(serialOverruns));

  // Start the inbound receive
  HAL_UART_Receive_IT(&huart1, (uint8_t *) &serialInterruptBuffer, sizeof(serialInterruptBuffer));

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void delay(uint32_t ms) {
  HAL_Delay(ms);
}

long unsigned int millis() {
  return (long unsigned int) HAL_GetTick();
}

void MY_UART_IRQHandler(UART_HandleTypeDef *huart) {
  // See if the transfer is completed
  if (huart->RxXferCount == 0) {
      if (serialFillIndex < sizeof(serialBuffer)) {
          if (serialFillIndex+1 == serialDrainIndex)
              serialOverruns++;
          else
              serialBuffer[serialFillIndex++] = serialInterruptBuffer[0];
      } else {
          if (serialDrainIndex == 1)
              serialOverruns++;
          else {
              serialBuffer[0] = serialInterruptBuffer[0];
              serialFillIndex = 1;
          }
      }
  }

  // Start another receive
  HAL_UART_Receive_IT(&huart1, (uint8_t *) &serialInterruptBuffer, sizeof(serialInterruptBuffer));

}

void MX_USART1_UART_DeInit(void) {
  if (!uart1Initialized)
      return;
  uart1Initialized = false;

  HAL_UART_DeInit(&huart1);
}

void noteSerialReset() {
  MX_USART1_UART_DeInit();
  MX_USART1_UART_Init();
}

void noteSerialTransmit(uint8_t *text, size_t len, bool flush) {
  HAL_UART_Transmit(&huart1, text, len, 5000);
}

bool noteSerialAvailable() {
  return (serialFillIndex != serialDrainIndex);
}

char noteSerialReceive() {
  char data;
  while (!noteSerialAvailable()) ;
  if (serialDrainIndex < sizeof(serialBuffer))
      data = serialBuffer[serialDrainIndex++];
  else {
      data = serialBuffer[0];
      serialDrainIndex = 1;
  }
  return data;
}

void user_delay_ms(uint32_t period)
{
	HAL_Delay(period);
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	if (HAL_I2C_Master_Transmit(&hi2c1, (dev_id << 1), &reg_addr, 1, 10) != HAL_OK)
		return -1;

	if (HAL_I2C_Master_Receive (&hi2c1, (dev_id << 1) | 0x01, reg_data, len, 10) != HAL_OK)
		return -1;

	return 0;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t *buf;
	buf = malloc(len + 1);
	buf[0] = reg_addr;
	memcpy(buf + 1, reg_data, len);

	if (HAL_I2C_Master_Transmit(&hi2c1, (dev_id << 1), (uint8_t *) buf, len + 1, HAL_MAX_DELAY) != HAL_OK)
		return -1;

	free(buf);

	return 0;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
