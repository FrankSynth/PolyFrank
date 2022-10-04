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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ARM_MATH_CM7
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
void Error_Handler();
void PolyError_Handler(const char *errorMessage);
void PolyError_HandlerClear();

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_CS_Layer_2B_Pin GPIO_PIN_2
#define SPI_CS_Layer_2B_GPIO_Port GPIOE
#define Layer_RST_Pin GPIO_PIN_3
#define Layer_RST_GPIO_Port GPIOE
#define Layer_Boot_Pin GPIO_PIN_8
#define Layer_Boot_GPIO_Port GPIOI
#define Display_EN_Pin GPIO_PIN_6
#define Display_EN_GPIO_Port GPIOF
#define SPI_READY_LAYER_2A_Pin GPIO_PIN_7
#define SPI_READY_LAYER_2A_GPIO_Port GPIOF
#define SPI_READY_LAYER_2A_EXTI_IRQn EXTI9_5_IRQn
#define Display_PWM_Pin GPIO_PIN_8
#define Display_PWM_GPIO_Port GPIOF
#define SPI_READY_LAYER_2B_Pin GPIO_PIN_9
#define SPI_READY_LAYER_2B_GPIO_Port GPIOF
#define SPI_READY_LAYER_2B_EXTI_IRQn EXTI9_5_IRQn
#define SPI_MOSI_Panel_Pin GPIO_PIN_1
#define SPI_MOSI_Panel_GPIO_Port GPIOC
#define SPI_MISO_Panel_Pin GPIO_PIN_2
#define SPI_MISO_Panel_GPIO_Port GPIOC
#define Control_EEPROM_CS_Pin GPIO_PIN_4
#define Control_EEPROM_CS_GPIO_Port GPIOA
#define SPI_SCK_EEPROM_Pin GPIO_PIN_5
#define SPI_SCK_EEPROM_GPIO_Port GPIOA
#define SPI_MISO_EEPROM_Pin GPIO_PIN_6
#define SPI_MISO_EEPROM_GPIO_Port GPIOA
#define SPI_MOSI_EEPROM_Pin GPIO_PIN_7
#define SPI_MOSI_EEPROM_GPIO_Port GPIOA
#define Panel_1_INT_Pin GPIO_PIN_2
#define Panel_1_INT_GPIO_Port GPIOB
#define Panel_1_INT_EXTI_IRQn EXTI2_IRQn
#define I2C_SCL_IOEXP_Pin GPIO_PIN_10
#define I2C_SCL_IOEXP_GPIO_Port GPIOB
#define I2C_SDA_IOEXP_Pin GPIO_PIN_11
#define I2C_SDA_IOEXP_GPIO_Port GPIOB
#define I2C_SCL_Panel_2_Pin GPIO_PIN_7
#define I2C_SCL_Panel_2_GPIO_Port GPIOH
#define I2C_SDA_Panel_2_Pin GPIO_PIN_8
#define I2C_SDA_Panel_2_GPIO_Port GPIOH
#define I2C_SCL_Panel_1_Pin GPIO_PIN_11
#define I2C_SCL_Panel_1_GPIO_Port GPIOH
#define I2C_SDA_Panel_1_Pin GPIO_PIN_12
#define I2C_SDA_Panel_1_GPIO_Port GPIOH
#define IO_Sync_OUT_Pin GPIO_PIN_12
#define IO_Sync_OUT_GPIO_Port GPIOB
#define USB_MIDI_VBUS_Pin GPIO_PIN_13
#define USB_MIDI_VBUS_GPIO_Port GPIOB
#define USB_MIDI_DM_Pin GPIO_PIN_14
#define USB_MIDI_DM_GPIO_Port GPIOB
#define USB_MIDI_DP_Pin GPIO_PIN_15
#define USB_MIDI_DP_GPIO_Port GPIOB
#define Panel_1_EOC_Pin GPIO_PIN_11
#define Panel_1_EOC_GPIO_Port GPIOD
#define Panel_1_EOC_EXTI_IRQn EXTI15_10_IRQn
#define Control_Enc_INT_Pin GPIO_PIN_12
#define Control_Enc_INT_GPIO_Port GPIOD
#define Control_Enc_INT_EXTI_IRQn EXTI15_10_IRQn
#define Panel_2_EOC_Pin GPIO_PIN_13
#define Panel_2_EOC_GPIO_Port GPIOD
#define Panel_2_EOC_EXTI_IRQn EXTI15_10_IRQn
#define ADC_Mult_A_Pin GPIO_PIN_2
#define ADC_Mult_A_GPIO_Port GPIOG
#define ADC_Mult_B_Pin GPIO_PIN_3
#define ADC_Mult_B_GPIO_Port GPIOG
#define Enable_OUTPUT_Pin GPIO_PIN_5
#define Enable_OUTPUT_GPIO_Port GPIOG
#define Panel_2_INT_Pin GPIO_PIN_6
#define Panel_2_INT_GPIO_Port GPIOC
#define Panel_2_INT_EXTI_IRQn EXTI9_5_IRQn
#define Panel_RST_Pin GPIO_PIN_7
#define Panel_RST_GPIO_Port GPIOC
#define IO_SYNC_IN_Pin GPIO_PIN_8
#define IO_SYNC_IN_GPIO_Port GPIOC
#define IO_SYNC_IN_EXTI_IRQn EXTI9_5_IRQn
#define USB_PORT_VBUS_Pin GPIO_PIN_9
#define USB_PORT_VBUS_GPIO_Port GPIOA
#define USB_PORT_DM_Pin GPIO_PIN_11
#define USB_PORT_DM_GPIO_Port GPIOA
#define USB_PORT_DP_Pin GPIO_PIN_12
#define USB_PORT_DP_GPIO_Port GPIOA
#define Panel_2_CS_Pin GPIO_PIN_0
#define Panel_2_CS_GPIO_Port GPIOI
#define SPI_SCK_Panel_Pin GPIO_PIN_1
#define SPI_SCK_Panel_GPIO_Port GPIOI
#define SPI_READY_LAYER_1B_Pin GPIO_PIN_15
#define SPI_READY_LAYER_1B_GPIO_Port GPIOA
#define SPI_READY_LAYER_1B_EXTI_IRQn EXTI15_10_IRQn
#define SPI_READY_LAYER_1A_Pin GPIO_PIN_10
#define SPI_READY_LAYER_1A_GPIO_Port GPIOC
#define SPI_READY_LAYER_1A_EXTI_IRQn EXTI15_10_IRQn
#define MIDI_IN_Pin GPIO_PIN_12
#define MIDI_IN_GPIO_Port GPIOC
#define ENC_6_SW_Pin GPIO_PIN_3
#define ENC_6_SW_GPIO_Port GPIOD
#define ENC_6_SW_EXTI_IRQn EXTI3_IRQn
#define ENC_6_A_Pin GPIO_PIN_4
#define ENC_6_A_GPIO_Port GPIOD
#define ENC_6_A_EXTI_IRQn EXTI4_IRQn
#define ENC_6_B_Pin GPIO_PIN_5
#define ENC_6_B_GPIO_Port GPIOD
#define ENC_6_B_EXTI_IRQn EXTI9_5_IRQn
#define SPI_MOSI_Layer_Pin GPIO_PIN_7
#define SPI_MOSI_Layer_GPIO_Port GPIOD
#define SPI_MISO_Layer_Pin GPIO_PIN_9
#define SPI_MISO_Layer_GPIO_Port GPIOG
#define Panel_1_CS_Pin GPIO_PIN_10
#define Panel_1_CS_GPIO_Port GPIOG
#define SPI_SCK_Layer_Pin GPIO_PIN_11
#define SPI_SCK_Layer_GPIO_Port GPIOG
#define Control_RST_Pin GPIO_PIN_13
#define Control_RST_GPIO_Port GPIOG
#define Control_INT_Pin GPIO_PIN_14
#define Control_INT_GPIO_Port GPIOG
#define Control_INT_EXTI_IRQn EXTI15_10_IRQn
#define I2C_SCL_Touch_Pin GPIO_PIN_6
#define I2C_SCL_Touch_GPIO_Port GPIOB
#define I2C_SDA_Touch_Pin GPIO_PIN_7
#define I2C_SDA_Touch_GPIO_Port GPIOB
#define SPI_CS_Panel_Pin GPIO_PIN_9
#define SPI_CS_Panel_GPIO_Port GPIOB
#define SPI_CS_Layer_1A_Pin GPIO_PIN_4
#define SPI_CS_Layer_1A_GPIO_Port GPIOI
#define SPI_CS_Layer_2A_Pin GPIO_PIN_5
#define SPI_CS_Layer_2A_GPIO_Port GPIOI
#define SPI_CS_Layer_1B_Pin GPIO_PIN_6
#define SPI_CS_Layer_1B_GPIO_Port GPIOI
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
