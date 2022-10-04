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
void Error_Handler(void);
void PolyError_Handler(const char *errorMessage);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_READY_Pin GPIO_PIN_5
#define SPI_READY_GPIO_Port GPIOF
#define SPI_CS_SOFT_Pin GPIO_PIN_10
#define SPI_CS_SOFT_GPIO_Port GPIOF
#define SPI_CS_SOFT_EXTI_IRQn EXTI15_10_IRQn
#define CHIP_ID_A_Pin GPIO_PIN_0
#define CHIP_ID_A_GPIO_Port GPIOA
#define CHIP_ID_B_Pin GPIO_PIN_1
#define CHIP_ID_B_GPIO_Port GPIOA
#define STATUS_LED_Pin GPIO_PIN_2
#define STATUS_LED_GPIO_Port GPIOA
#define AUDIO_RST_Pin GPIO_PIN_11
#define AUDIO_RST_GPIO_Port GPIOH
#define SWITCH_1_A_Pin GPIO_PIN_10
#define SWITCH_1_A_GPIO_Port GPIOD
#define SWITCH_1_B_Pin GPIO_PIN_11
#define SWITCH_1_B_GPIO_Port GPIOD
#define SWITCH_2_A_Pin GPIO_PIN_12
#define SWITCH_2_A_GPIO_Port GPIOD
#define SWITCH_2_B_Pin GPIO_PIN_13
#define SWITCH_2_B_GPIO_Port GPIOD
#define INTERCHIP_SCL_Pin GPIO_PIN_6
#define INTERCHIP_SCL_GPIO_Port GPIOB
#define INTERCHIP_SDA_Pin GPIO_PIN_7
#define INTERCHIP_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
