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
#include "stm32h7xx_hal.h"

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
#define Layer_Reset_Pin GPIO_PIN_3
#define Layer_Reset_GPIO_Port GPIOE
#define Layer_1_Boot_2_Pin GPIO_PIN_8
#define Layer_1_Boot_2_GPIO_Port GPIOI
#define Layer_1_Boot_1_Pin GPIO_PIN_13
#define Layer_1_Boot_1_GPIO_Port GPIOC
#define Layer_2_Boot_1_Pin GPIO_PIN_14
#define Layer_2_Boot_1_GPIO_Port GPIOC
#define Layer_2_Boot_2_Pin GPIO_PIN_15
#define Layer_2_Boot_2_GPIO_Port GPIOC
#define Control_Display_Enable_Pin GPIO_PIN_6
#define Control_Display_Enable_GPIO_Port GPIOF
#define Display_PWM_Pin GPIO_PIN_8
#define Display_PWM_GPIO_Port GPIOF
#define Layer_2_CS_2_Pin GPIO_PIN_3
#define Layer_2_CS_2_GPIO_Port GPIOC
#define Layer_2_CS_1_Pin GPIO_PIN_0
#define Layer_2_CS_1_GPIO_Port GPIOA
#define Layer_2_READY_1_Pin GPIO_PIN_3
#define Layer_2_READY_1_GPIO_Port GPIOH
#define EEPROM_CS_Pin GPIO_PIN_4
#define EEPROM_CS_GPIO_Port GPIOA
#define Layer_2_READY_2_Pin GPIO_PIN_4
#define Layer_2_READY_2_GPIO_Port GPIOC
#define Panel_2_Change_Pin GPIO_PIN_5
#define Panel_2_Change_GPIO_Port GPIOC
#define Panel_1_Change_Pin GPIO_PIN_2
#define Panel_1_Change_GPIO_Port GPIOB
#define Sync_OUT_Pin GPIO_PIN_12
#define Sync_OUT_GPIO_Port GPIOB
#define Panel_1_EOC_Pin GPIO_PIN_11
#define Panel_1_EOC_GPIO_Port GPIOD
#define Encoder_Interrupt_Pin GPIO_PIN_12
#define Encoder_Interrupt_GPIO_Port GPIOD
#define Panel_2_EOC_Pin GPIO_PIN_13
#define Panel_2_EOC_GPIO_Port GPIOD
#define Panel_ADC_Mult_A_Pin GPIO_PIN_2
#define Panel_ADC_Mult_A_GPIO_Port GPIOG
#define Panel_ADC_Mult_B_Pin GPIO_PIN_3
#define Panel_ADC_Mult_B_GPIO_Port GPIOG
#define Panel_ADC_Mult_C_Pin GPIO_PIN_5
#define Panel_ADC_Mult_C_GPIO_Port GPIOG
#define Sync_IN_Pin GPIO_PIN_8
#define Sync_IN_GPIO_Port GPIOC
#define Panel_2_CS_Pin GPIO_PIN_0
#define Panel_2_CS_GPIO_Port GPIOI
#define Panel_1_CS_Pin GPIO_PIN_10
#define Panel_1_CS_GPIO_Port GPIOG
#define Control_Reset_Pin GPIO_PIN_13
#define Control_Reset_GPIO_Port GPIOG
#define Control_Touch_Change_Pin GPIO_PIN_14
#define Control_Touch_Change_GPIO_Port GPIOG
#define Panel_Reset_Pin GPIO_PIN_4
#define Panel_Reset_GPIO_Port GPIOB
#define Layer_1_CS_1_Pin GPIO_PIN_4
#define Layer_1_CS_1_GPIO_Port GPIOI
#define Layer_1_CS_2_Pin GPIO_PIN_5
#define Layer_1_CS_2_GPIO_Port GPIOI
#define Layer_1_READY_1_Pin GPIO_PIN_6
#define Layer_1_READY_1_GPIO_Port GPIOI
#define Layer_1_READY_2_Pin GPIO_PIN_7
#define Layer_1_READY_2_GPIO_Port GPIOI
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
