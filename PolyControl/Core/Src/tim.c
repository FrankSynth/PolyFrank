/**
 ******************************************************************************
 * File Name          : TIM.c
 * Description        : This file provides code for the configuration
 *                      of the TIM instances.
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

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim16;

/* TIM2 init function */
void MX_TIM2_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 240 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4294967295;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}
/* TIM3 init function */
void MX_TIM3_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 24 - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 100 - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim3);
}

/* TIM4 init function */ // interrupt once per second
void MX_TIM4_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 24000 - 1;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 10000 - 1;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}

/* TIM5 init function */
void MX_TIM5_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 240 - 1;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 1000000 - 1;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim5) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}
/* TIM13 init function */
void MX_TIM13_Init(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim13.Instance = TIM13;
    htim13.Init.Prescaler = 0;
    htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim13.Init.Period = 10000;
    htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim13) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim13) != HAL_OK) {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim13, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim13);
}

/* TIM16 init function */
void MX_TIM16_Init(void) {

    /* USER CODE BEGIN TIM16_Init 0 */

    /* USER CODE END TIM16_Init 0 */

    /* USER CODE BEGIN TIM16_Init 1 */

    /* USER CODE END TIM16_Init 1 */
    htim16.Instance = TIM16;
    htim16.Init.Prescaler = 240 - 1;
    htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim16.Init.Period = 500 - 1;
    htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim16.Init.RepetitionCounter = 0;
    htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim16) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM16_Init 2 */

    /* USER CODE END TIM16_Init 2 */
}
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {

    if (tim_baseHandle->Instance == TIM2) {
        /* USER CODE BEGIN TIM2_MspInit 0 */

        /* USER CODE END TIM2_MspInit 0 */
        /* TIM2 clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
        /* USER CODE BEGIN TIM2_MspInit 1 */

        /* USER CODE END TIM2_MspInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM3) {
        /* USER CODE BEGIN TIM3_MspInit 0 */

        /* USER CODE END TIM3_MspInit 0 */
        /* TIM3 clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* USER CODE BEGIN TIM3_MspInit 1 */

        /* USER CODE END TIM3_MspInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM4) {
        /* USER CODE BEGIN TIM3_MspInit 0 */

        /* USER CODE END TIM3_MspInit 0 */
        /* TIM3 clock enable */
        __HAL_RCC_TIM4_CLK_ENABLE();
        /* USER CODE BEGIN TIM3_MspInit 1 */

        /* USER CODE END TIM3_MspInit 1 */
        HAL_NVIC_SetPriority(TIM4_IRQn, 8, 1);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }
    else if (tim_baseHandle->Instance == TIM5) {
        /* USER CODE BEGIN TIM3_MspInit 0 */

        /* USER CODE END TIM3_MspInit 0 */
        /* TIM3 clock enable */
        __HAL_RCC_TIM5_CLK_ENABLE();
        /* USER CODE BEGIN TIM3_MspInit 1 */

        /* USER CODE END TIM3_MspInit 1 */
        HAL_NVIC_SetPriority(TIM5_IRQn, 1, 3);
        HAL_NVIC_EnableIRQ(TIM5_IRQn);
    }
    else if (tim_baseHandle->Instance == TIM13) {
        /* USER CODE BEGIN TIM13_MspInit 0 */

        /* USER CODE END TIM13_MspInit 0 */
        /* TIM13 clock enable */
        __HAL_RCC_TIM13_CLK_ENABLE();
        /* USER CODE BEGIN TIM13_MspInit 1 */

        /* USER CODE END TIM13_MspInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM16) {
        /* USER CODE BEGIN TIM16_MspInit 0 */

        /* USER CODE END TIM16_MspInit 0 */
        /* TIM16 clock enable */
        __HAL_RCC_TIM16_CLK_ENABLE();

        /* TIM16 interrupt Init */
        HAL_NVIC_SetPriority(TIM16_IRQn, 8, 0);
        HAL_NVIC_EnableIRQ(TIM16_IRQn);
        /* USER CODE BEGIN TIM16_MspInit 1 */

        /* USER CODE END TIM16_MspInit 1 */
    }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (timHandle->Instance == TIM3) {
        /* USER CODE BEGIN TIM3_MspPostInit 0 */

        /* USER CODE END TIM3_MspPostInit 0 */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM3 GPIO Configuration
        PC7     ------> TIM3_CH2
        */
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM3_MspPostInit 1 */

        /* USER CODE END TIM3_MspPostInit 1 */
    }
    else if (timHandle->Instance == TIM13) {
        /* USER CODE BEGIN TIM13_MspPostInit 0 */

        /* USER CODE END TIM13_MspPostInit 0 */
        __HAL_RCC_GPIOF_CLK_ENABLE();
#ifdef POLYCONTROL
        /**TIM13 GPIO Configuration
        PF8     ------> TIM13_CH1
        */
        GPIO_InitStruct.Pin = Display_PWM_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_TIM13;
        HAL_GPIO_Init(Display_PWM_GPIO_Port, &GPIO_InitStruct);
#endif
        /* USER CODE BEGIN TIM13_MspPostInit 1 */

        /* USER CODE END TIM13_MspPostInit 1 */
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle) {

    if (tim_baseHandle->Instance == TIM2) {
        /* USER CODE BEGIN TIM2_MspDeInit 0 */

        /* USER CODE END TIM2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM2_CLK_DISABLE();
        /* USER CODE BEGIN TIM2_MspDeInit 1 */

        /* USER CODE END TIM2_MspDeInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM3) {
        /* USER CODE BEGIN TIM3_MspDeInit 0 */

        /* USER CODE END TIM3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
        /* USER CODE BEGIN TIM3_MspDeInit 1 */

        /* USER CODE END TIM3_MspDeInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM4) {
        /* USER CODE BEGIN TIM3_MspDeInit 0 */

        /* USER CODE END TIM3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM4_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM4_IRQn);

        /* USER CODE BEGIN TIM3_MspDeInit 1 */

        /* USER CODE END TIM3_MspDeInit 1 */
    }

    else if (tim_baseHandle->Instance == TIM5) {
        /* USER CODE BEGIN TIM3_MspDeInit 0 */

        /* USER CODE END TIM3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM5_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM5_IRQn);

        /* USER CODE BEGIN TIM3_MspDeInit 1 */

        /* USER CODE END TIM3_MspDeInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM13) {
        /* USER CODE BEGIN TIM13_MspDeInit 0 */

        /* USER CODE END TIM13_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM13_CLK_DISABLE();
        /* USER CODE BEGIN TIM13_MspDeInit 1 */

        /* USER CODE END TIM13_MspDeInit 1 */
    }
    else if (tim_baseHandle->Instance == TIM16) {
        /* USER CODE BEGIN TIM16_MspDeInit 0 */

        /* USER CODE END TIM16_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM16_CLK_DISABLE();

        /* TIM16 interrupt Deinit */
        HAL_NVIC_DisableIRQ(TIM16_IRQn);
        /* USER CODE BEGIN TIM16_MspDeInit 1 */

        /* USER CODE END TIM16_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
