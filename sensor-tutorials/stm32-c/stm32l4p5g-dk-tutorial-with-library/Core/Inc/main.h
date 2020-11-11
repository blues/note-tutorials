/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define JOY_RIGHT_Pin GPIO_PIN_10
#define JOY_RIGHT_GPIO_Port GPIOI
#define MCU_SMPS_V1_Pin GPIO_PIN_2
#define MCU_SMPS_V1_GPIO_Port GPIOH
#define DCMI_D2_Pin GPIO_PIN_0
#define DCMI_D2_GPIO_Port GPIOE
#define TP_RST_Pin GPIO_PIN_4
#define TP_RST_GPIO_Port GPIOB
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LCD_PWM_EN_Pin GPIO_PIN_15
#define LCD_PWM_EN_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define ARD_SPI2_NSS_TIM5_CH4_Pin GPIO_PIN_0
#define ARD_SPI2_NSS_TIM5_CH4_GPIO_Port GPIOI
#define DCMI_D4_Pin GPIO_PIN_14
#define DCMI_D4_GPIO_Port GPIOH
#define JOY_DOWN_Pin GPIO_PIN_9
#define JOY_DOWN_GPIO_Port GPIOI
#define DCMI_D7_Pin GPIO_PIN_7
#define DCMI_D7_GPIO_Port GPIOI
#define LCD_VSYNC_Pin GPIO_PIN_1
#define LCD_VSYNC_GPIO_Port GPIOE
#define SAI1_SDB_Pin GPIO_PIN_5
#define SAI1_SDB_GPIO_Port GPIOB
#define DCMI_D6_Pin GPIO_PIN_6
#define DCMI_D6_GPIO_Port GPIOI
#define ARD_TIM8_CH4_Pin GPIO_PIN_2
#define ARD_TIM8_CH4_GPIO_Port GPIOI
#define TP_INT_Pin GPIO_PIN_1
#define TP_INT_GPIO_Port GPIOI
#define MCU_SMPS_SW_Pin GPIO_PIN_15
#define MCU_SMPS_SW_GPIO_Port GPIOH
#define DCMI_D3_Pin GPIO_PIN_12
#define DCMI_D3_GPIO_Port GPIOH
#define JOY_LEFT_Pin GPIO_PIN_11
#define JOY_LEFT_GPIO_Port GPIOI
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define MCU_SMPS_V3_Pin GPIO_PIN_13
#define MCU_SMPS_V3_GPIO_Port GPIOH
#define DCMI_D10_Pin GPIO_PIN_3
#define DCMI_D10_GPIO_Port GPIOI
#define JOY_UP_Pin GPIO_PIN_8
#define JOY_UP_GPIO_Port GPIOI
#define SAI1_FS_A_Pin GPIO_PIN_4
#define SAI1_FS_A_GPIO_Port GPIOE
#define LCD_R6_Pin GPIO_PIN_3
#define LCD_R6_GPIO_Port GPIOE
#define LCD_R7_Pin GPIO_PIN_2
#define LCD_R7_GPIO_Port GPIOE
#define I2C1_SDA_Pin GPIO_PIN_7
#define I2C1_SDA_GPIO_Port GPIOB
#define SDMMC1_CMD_Pin GPIO_PIN_2
#define SDMMC1_CMD_GPIO_Port GPIOD
#define SDMMC1_D2_Pin GPIO_PIN_10
#define SDMMC1_D2_GPIO_Port GPIOC
#define DCMI_D5_Pin GPIO_PIN_4
#define DCMI_D5_GPIO_Port GPIOI
#define DCMI_D0_Pin GPIO_PIN_9
#define DCMI_D0_GPIO_Port GPIOH
#define MCU_SMPS_PG_Pin GPIO_PIN_7
#define MCU_SMPS_PG_GPIO_Port GPIOH
#define USB_OTG_FS_DP_Pin GPIO_PIN_12
#define USB_OTG_FS_DP_GPIO_Port GPIOA
#define JOY_SEL_Pin GPIO_PIN_13
#define JOY_SEL_GPIO_Port GPIOC
#define LCD_G6_Pin GPIO_PIN_6
#define LCD_G6_GPIO_Port GPIOE
#define LCD_G7_Pin GPIO_PIN_5
#define LCD_G7_GPIO_Port GPIOE
#define BOOT0_Pin GPIO_PIN_3
#define BOOT0_GPIO_Port GPIOH
#define STMOD_TIM15_CH2_Pin GPIO_PIN_11
#define STMOD_TIM15_CH2_GPIO_Port GPIOG
#define SAI1_SD_A_Pin GPIO_PIN_6
#define SAI1_SD_A_GPIO_Port GPIOD
#define DFSDM1_DATIN0_Pin GPIO_PIN_3
#define DFSDM1_DATIN0_GPIO_Port GPIOD
#define SDMMC1_D3_Pin GPIO_PIN_11
#define SDMMC1_D3_GPIO_Port GPIOC
#define DCMI_VSYNC_Pin GPIO_PIN_5
#define DCMI_VSYNC_GPIO_Port GPIOI
#define MCU_SMPS_EN_Pin GPIO_PIN_6
#define MCU_SMPS_EN_GPIO_Port GPIOH
#define USB_OTGFS_DM_Pin GPIO_PIN_11
#define USB_OTGFS_DM_GPIO_Port GPIOA
#define OCTOSPIM_P2_IO2_Pin GPIO_PIN_2
#define OCTOSPIM_P2_IO2_GPIO_Port GPIOF
#define OCTOSPIM_P2_IO1_Pin GPIO_PIN_1
#define OCTOSPIM_P2_IO1_GPIO_Port GPIOF
#define OCTOSPIM_P2_IO0_Pin GPIO_PIN_0
#define OCTOSPIM_P2_IO0_GPIO_Port GPIOF
#define OCTOSPIM_P2_CS_Pin GPIO_PIN_12
#define OCTOSPIM_P2_CS_GPIO_Port GPIOG
#define SDMMC1_CK_Pin GPIO_PIN_12
#define SDMMC1_CK_GPIO_Port GPIOC
#define USB_OTGFS_ID_Pin GPIO_PIN_10
#define USB_OTGFS_ID_GPIO_Port GPIOA
#define USB_OTGFS_VBUS_Pin GPIO_PIN_9
#define USB_OTGFS_VBUS_GPIO_Port GPIOA
#define OCTOSPIM_P2_IO3_Pin GPIO_PIN_3
#define OCTOSPIM_P2_IO3_GPIO_Port GPIOF
#define MFX_WAKEUP_Pin GPIO_PIN_5
#define MFX_WAKEUP_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOG
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOG
#define SAI1_SCK_A_Pin GPIO_PIN_8
#define SAI1_SCK_A_GPIO_Port GPIOA
#define SDMMC1_D1_Pin GPIO_PIN_9
#define SDMMC1_D1_GPIO_Port GPIOC
#define SDMMC1_D0_Pin GPIO_PIN_8
#define SDMMC1_D0_GPIO_Port GPIOC
#define OCTOSPIM_P1_DQS_Pin GPIO_PIN_6
#define OCTOSPIM_P1_DQS_GPIO_Port GPIOG
#define STMOD_DFSDM1_CKOUT_Pin GPIO_PIN_10
#define STMOD_DFSDM1_CKOUT_GPIO_Port GPIOF
#define ARD_ADC12_IN13_Pin GPIO_PIN_4
#define ARD_ADC12_IN13_GPIO_Port GPIOC
#define OCTOSPIM_P1_CLK_Pin GPIO_PIN_10
#define OCTOSPIM_P1_CLK_GPIO_Port GPIOE
#define LCD_DISP_Pin GPIO_PIN_8
#define LCD_DISP_GPIO_Port GPIOG
#define SAI1_MCLK_A_Pin GPIO_PIN_7
#define SAI1_MCLK_A_GPIO_Port GPIOG
#define ARD_TIM4_CH4_Pin GPIO_PIN_15
#define ARD_TIM4_CH4_GPIO_Port GPIOD
#define LCD_DE_Pin GPIO_PIN_0
#define LCD_DE_GPIO_Port GPIOC
#define ARD_ADC12_IN2_Pin GPIO_PIN_1
#define ARD_ADC12_IN2_GPIO_Port GPIOC
#define LCD_HSYNC_Pin GPIO_PIN_2
#define LCD_HSYNC_GPIO_Port GPIOC
#define STMOD_SPI1_NSS_Pin GPIO_PIN_5
#define STMOD_SPI1_NSS_GPIO_Port GPIOG
#define STMOD_SPI1_MOSI_Pin GPIO_PIN_4
#define STMOD_SPI1_MOSI_GPIO_Port GPIOG
#define STMOD_SPI1_MISO_Pin GPIO_PIN_3
#define STMOD_SPI1_MISO_GPIO_Port GPIOG
#define STMOD_SPI1_SCK_Pin GPIO_PIN_2
#define STMOD_SPI1_SCK_GPIO_Port GPIOG
#define STMOD_INT_Pin GPIO_PIN_10
#define STMOD_INT_GPIO_Port GPIOD
#define ARD_ADC12_IN10_Pin GPIO_PIN_5
#define ARD_ADC12_IN10_GPIO_Port GPIOA
#define ARD_ADC12_IN15_Pin GPIO_PIN_0
#define ARD_ADC12_IN15_GPIO_Port GPIOB
#define I2C4_SDA_Pin GPIO_PIN_15
#define I2C4_SDA_GPIO_Port GPIOF
#define LCD_B7_Pin GPIO_PIN_8
#define LCD_B7_GPIO_Port GPIOE
#define MCU_SMPS_V2_Pin GPIO_PIN_4
#define MCU_SMPS_V2_GPIO_Port GPIOH
#define ARD_TIM4_CH3_Pin GPIO_PIN_14
#define ARD_TIM4_CH3_GPIO_Port GPIOD
#define STMOD_USART3_RTS_Pin GPIO_PIN_12
#define STMOD_USART3_RTS_GPIO_Port GPIOD
#define STMOD_USART3_CTS_Pin GPIO_PIN_11
#define STMOD_USART3_CTS_GPIO_Port GPIOD
#define BL_EN_Pin GPIO_PIN_13
#define BL_EN_GPIO_Port GPIOD
#define LCD_CLK_Pin GPIO_PIN_4
#define LCD_CLK_GPIO_Port GPIOA
#define OCTOSPIM_P1_IO2_Pin GPIO_PIN_7
#define OCTOSPIM_P1_IO2_GPIO_Port GPIOA
#define ARD_ADC12_IN16_Pin GPIO_PIN_1
#define ARD_ADC12_IN16_GPIO_Port GPIOB
#define I2C4_SCL_Pin GPIO_PIN_14
#define I2C4_SCL_GPIO_Port GPIOF
#define LCD_B6_Pin GPIO_PIN_7
#define LCD_B6_GPIO_Port GPIOE
#define OCTOSPIM_P1_IO1_Pin GPIO_PIN_13
#define OCTOSPIM_P1_IO1_GPIO_Port GPIOE
#define DCMI_PIXCLK_Pin GPIO_PIN_5
#define DCMI_PIXCLK_GPIO_Port GPIOH
#define STMOD_USART3_TX_Pin GPIO_PIN_9
#define STMOD_USART3_TX_GPIO_Port GPIOD
#define STMOD_USART3_TXD8_Pin GPIO_PIN_8
#define STMOD_USART3_TXD8_GPIO_Port GPIOD
#define USART2_RX_Pin GPIO_PIN_3
#define USART2_RX_GPIO_Port GPIOA
#define OCTOSPIM_P1_IO3_Pin GPIO_PIN_6
#define OCTOSPIM_P1_IO3_GPIO_Port GPIOA
#define OCTOSPIM_P1_IO0_Pin GPIO_PIN_12
#define OCTOSPIM_P1_IO0_GPIO_Port GPIOE
#define DCMI_D1_Pin GPIO_PIN_10
#define DCMI_D1_GPIO_Port GPIOH
#define SPI2_MOSI_Pin GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port GPIOB
#define SPI2_MISO_Pin GPIO_PIN_14
#define SPI2_MISO_GPIO_Port GPIOB
#define USART2_TX_Pin GPIO_PIN_2
#define USART2_TX_GPIO_Port GPIOA
#define ARD_ADC12_IN6_Pin GPIO_PIN_1
#define ARD_ADC12_IN6_GPIO_Port GPIOA
#define LCD_RTC_OUT2_Pin GPIO_PIN_2
#define LCD_RTC_OUT2_GPIO_Port GPIOB
#define OCTOSPIM_P2_DQS_Pin GPIO_PIN_12
#define OCTOSPIM_P2_DQS_GPIO_Port GPIOF
#define OCTOSPIM_P1_NCS_Pin GPIO_PIN_11
#define OCTOSPIM_P1_NCS_GPIO_Port GPIOE
#define DCMI_HSYNC_Pin GPIO_PIN_8
#define DCMI_HSYNC_GPIO_Port GPIOH
#define DFDATIN1_Pin GPIO_PIN_12
#define DFDATIN1_GPIO_Port GPIOB
#define SPI2_CLK_Pin GPIO_PIN_13
#define SPI2_CLK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
