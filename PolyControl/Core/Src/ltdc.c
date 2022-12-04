/**
 ******************************************************************************
 * File Name          : LTDC.c
 * Description        : This file provides code for the configuration
 *                      of the LTDC instances.
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
#include "ltdc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

LTDC_HandleTypeDef hltdc;

/* LTDC init function */
void MX_LTDC_Init(void) {
    LTDC_LayerCfgTypeDef pLayerCfg = {0};

    hltdc.Instance = LTDC;
    hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    hltdc.Init.HorizontalSync = 29;
    hltdc.Init.VerticalSync = 9;
    hltdc.Init.AccumulatedHBP = 75;
    hltdc.Init.AccumulatedVBP = 32;
    hltdc.Init.AccumulatedActiveW = 875;
    hltdc.Init.AccumulatedActiveH = 512;
    hltdc.Init.TotalWidth = 879;
    hltdc.Init.TotalHeigh = 534;
    hltdc.Init.Backcolor.Blue = 0;
    hltdc.Init.Backcolor.Green = 0;
    hltdc.Init.Backcolor.Red = 0;
    if (HAL_LTDC_Init(&hltdc) != HAL_OK) {
        Error_Handler();
    }

    pLayerCfg.WindowX0 = 0;
    pLayerCfg.WindowX1 = 800;
    pLayerCfg.WindowY0 = 0;
    pLayerCfg.WindowY1 = 480;
    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    pLayerCfg.Alpha = 0xFF;
    pLayerCfg.Alpha0 = 0;
    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    pLayerCfg.FBStartAdress = 0xD0000000;
    pLayerCfg.ImageWidth = 800;
    pLayerCfg.ImageHeight = 480;
    pLayerCfg.Backcolor.Blue = 0;
    pLayerCfg.Backcolor.Green = 0;
    pLayerCfg.Backcolor.Red = 0;
    if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *ltdcHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (ltdcHandle->Instance == LTDC) {
        /* USER CODE BEGIN LTDC_MspInit 0 */

        /* USER CODE END LTDC_MspInit 0 */
        /* LTDC clock enable */
        __HAL_RCC_LTDC_CLK_ENABLE();

        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOI_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**LTDC GPIO Configuration
        PE4     ------> LTDC_B0
        PE5     ------> LTDC_G0
        PE6     ------> LTDC_G1
        PI9     ------> LTDC_VSYNC
        PI10     ------> LTDC_HSYNC
        PI11     ------> LTDC_G6
        PF10     ------> LTDC_DE
        PC0     ------> LTDC_R5
        PA1     ------> LTDC_R2
        PA2     ------> LTDC_R1
        PH2     ------> LTDC_R0
        PH4     ------> LTDC_G5
        PA3     ------> LTDC_B5
        PB0     ------> LTDC_R3
        PB1     ------> LTDC_R6
        PH10     ------> LTDC_R4
        PG6     ------> LTDC_R7
        PG7     ------> LTDC_CLK
        PC9     ------> LTDC_B2
        PA8     ------> LTDC_B3
        PA10     ------> LTDC_B1
        PH13     ------> LTDC_G2
        PH14     ------> LTDC_G3
        PH15     ------> LTDC_G4
        PI2     ------> LTDC_G7
        PG12     ------> LTDC_B4
        PB8     ------> LTDC_B6
        PI7     ------> LTDC_B7
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_2 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* LTDC interrupt Init */
        HAL_NVIC_SetPriority(LTDC_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(LTDC_IRQn);
        HAL_NVIC_SetPriority(LTDC_ER_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);
        /* USER CODE BEGIN LTDC_MspInit 1 */

        /* USER CODE END LTDC_MspInit 1 */
    }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *ltdcHandle) {

    if (ltdcHandle->Instance == LTDC) {
        /* USER CODE BEGIN LTDC_MspDeInit 0 */

        /* USER CODE END LTDC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LTDC_CLK_DISABLE();

        /**LTDC GPIO Configuration
        PE4     ------> LTDC_B0
        PE5     ------> LTDC_G0
        PE6     ------> LTDC_G1
        PI9     ------> LTDC_VSYNC
        PI10     ------> LTDC_HSYNC
        PI11     ------> LTDC_G6
        PF10     ------> LTDC_DE
        PC0     ------> LTDC_R5
        PA1     ------> LTDC_R2
        PA2     ------> LTDC_R1
        PH2     ------> LTDC_R0
        PH4     ------> LTDC_G5
        PA3     ------> LTDC_B5
        PB0     ------> LTDC_R3
        PB1     ------> LTDC_R6
        PH10     ------> LTDC_R4
        PG6     ------> LTDC_R7
        PG7     ------> LTDC_CLK
        PC9     ------> LTDC_B2
        PA8     ------> LTDC_B3
        PA10     ------> LTDC_B1
        PH13     ------> LTDC_G2
        PH14     ------> LTDC_G3
        PH15     ------> LTDC_G4
        PI2     ------> LTDC_G7
        PG12     ------> LTDC_B4
        PB8     ------> LTDC_B6
        PI7     ------> LTDC_B7
        */
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);

        HAL_GPIO_DeInit(GPIOI, GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_2 | GPIO_PIN_7);

        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_9);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_10);

        HAL_GPIO_DeInit(GPIOH, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8);

        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_12);

        /* LTDC interrupt Deinit */
        HAL_NVIC_DisableIRQ(LTDC_IRQn);
        HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);
        /* USER CODE BEGIN LTDC_MspDeInit 1 */

        /* USER CODE END LTDC_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
