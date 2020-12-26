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

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_CS_fromControl_Pin GPIO_PIN_8
#define SPI_CS_fromControl_GPIO_Port GPIOI
#define SPI_CS_fromControl_EXTI_IRQn EXTI9_5_IRQn
#define SPI_Ready_toControl_Pin GPIO_PIN_13
#define SPI_Ready_toControl_GPIO_Port GPIOC
#define LDAC_14_Pin GPIO_PIN_9
#define LDAC_14_GPIO_Port GPIOI
#define LDAC_15_Pin GPIO_PIN_10
#define LDAC_15_GPIO_Port GPIOI
#define LDAC_16_Pin GPIO_PIN_11
#define LDAC_16_GPIO_Port GPIOI
#define LDAC_5_Pin GPIO_PIN_2
#define LDAC_5_GPIO_Port GPIOF
#define LDAC_6_Pin GPIO_PIN_3
#define LDAC_6_GPIO_Port GPIOF
#define LDAC_7_Pin GPIO_PIN_4
#define LDAC_7_GPIO_Port GPIOF
#define LDAC_8_Pin GPIO_PIN_5
#define LDAC_8_GPIO_Port GPIOF
#define LDAC_9_Pin GPIO_PIN_6
#define LDAC_9_GPIO_Port GPIOF
#define LDAC_10_Pin GPIO_PIN_7
#define LDAC_10_GPIO_Port GPIOF
#define LDAC_11_Pin GPIO_PIN_8
#define LDAC_11_GPIO_Port GPIOF
#define LDAC_12_Pin GPIO_PIN_9
#define LDAC_12_GPIO_Port GPIOF
#define LDAC_13_Pin GPIO_PIN_10
#define LDAC_13_GPIO_Port GPIOF
#define LDAC_1_Pin GPIO_PIN_0
#define LDAC_1_GPIO_Port GPIOC
#define LDAC_2_Pin GPIO_PIN_1
#define LDAC_2_GPIO_Port GPIOC
#define LDAC_3_Pin GPIO_PIN_2
#define LDAC_3_GPIO_Port GPIOC
#define LDAC_4_Pin GPIO_PIN_3
#define LDAC_4_GPIO_Port GPIOC
#define Audio_Reset_Pin GPIO_PIN_15
#define Audio_Reset_GPIO_Port GPIOE
#define switch_1_open_Pin GPIO_PIN_6
#define switch_1_open_GPIO_Port GPIOD
#define switch_2_open_Pin GPIO_PIN_7
#define switch_2_open_GPIO_Port GPIOD
#define switch_1_A_Pin GPIO_PIN_10
#define switch_1_A_GPIO_Port GPIOG
#define switch_1_B_Pin GPIO_PIN_11
#define switch_1_B_GPIO_Port GPIOG
#define switch_1_C_Pin GPIO_PIN_12
#define switch_1_C_GPIO_Port GPIOG
#define switch_2_A_Pin GPIO_PIN_13
#define switch_2_A_GPIO_Port GPIOG
#define switch_2_B_Pin GPIO_PIN_14
#define switch_2_B_GPIO_Port GPIOG
#define switch_2_C_Pin GPIO_PIN_15
#define switch_2_C_GPIO_Port GPIOG
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
