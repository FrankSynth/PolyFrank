/**
 ******************************************************************************
 * @file    i2c.c
 * @brief   This file provides code for the configuration
 *          of the I2C instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;
DMA_HandleTypeDef hdma_i2c3_tx;
DMA_HandleTypeDef hdma_i2c4_tx;

/* I2C1 init function */
void MX_I2C1_Init(void) {

    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x2010091A;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */
}
/* I2C2 init function */
void MX_I2C2_Init(void) {

    /* USER CODE BEGIN I2C2_Init 0 */

    /* USER CODE END I2C2_Init 0 */

    /* USER CODE BEGIN I2C2_Init 1 */

    /* USER CODE END I2C2_Init 1 */
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x2010091A;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C2_Init 2 */

    /* USER CODE END I2C2_Init 2 */
}
/* I2C3 init function */
void MX_I2C3_Init(void) {

    /* USER CODE BEGIN I2C3_Init 0 */

    /* USER CODE END I2C3_Init 0 */

    /* USER CODE BEGIN I2C3_Init 1 */

    /* USER CODE END I2C3_Init 1 */
    hi2c3.Instance = I2C3;
    hi2c3.Init.Timing = 0x2010091A;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C3_Init 2 */

    /* USER CODE END I2C3_Init 2 */
}
/* I2C4 init function */
void MX_I2C4_Init(void) {

    /* USER CODE BEGIN I2C4_Init 0 */

    /* USER CODE END I2C4_Init 0 */

    /* USER CODE BEGIN I2C4_Init 1 */

    /* USER CODE END I2C4_Init 1 */
    hi2c4.Instance = I2C4;
    hi2c4.Init.Timing = 0x2010091A;
    hi2c4.Init.OwnAddress1 = 0;
    hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c4.Init.OwnAddress2 = 0;
    hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c4) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C4_Init 2 */

    /* USER CODE END I2C4_Init 2 */
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (i2cHandle->Instance == I2C1) {
        /* USER CODE BEGIN I2C1_MspInit 0 */

        /* USER CODE END I2C1_MspInit 0 */

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C1 GPIO Configuration
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA
        */
        GPIO_InitStruct.Pin = I2C_SCL_Touch_Pin | I2C_SDA_Touch_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* I2C1 clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
        /* USER CODE BEGIN I2C1_MspInit 1 */

        /* USER CODE END I2C1_MspInit 1 */
    }
    else if (i2cHandle->Instance == I2C2) {
        /* USER CODE BEGIN I2C2_MspInit 0 */

        /* USER CODE END I2C2_MspInit 0 */

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C2 GPIO Configuration
        PB10     ------> I2C2_SCL
        PB11     ------> I2C2_SDA
        */
        GPIO_InitStruct.Pin = I2C_SCL_IOEXP_Pin | I2C_SDA_IOEXP_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* I2C2 clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();
        /* USER CODE BEGIN I2C2_MspInit 1 */

        /* USER CODE END I2C2_MspInit 1 */
    }
    else if (i2cHandle->Instance == I2C3) {
        /* USER CODE BEGIN I2C3_MspInit 0 */

        /* USER CODE END I2C3_MspInit 0 */

        __HAL_RCC_GPIOH_CLK_ENABLE();
        /**I2C3 GPIO Configuration
        PH7     ------> I2C3_SCL
        PH8     ------> I2C3_SDA
        */
        GPIO_InitStruct.Pin = I2C_SCL_Panel_2_Pin | I2C_SDA_Panel_2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        /* I2C3 clock enable */
        __HAL_RCC_I2C3_CLK_ENABLE();

        /* I2C3 DMA Init */
        /* I2C3_TX Init */
        hdma_i2c3_tx.Instance = DMA1_Stream3;
        hdma_i2c3_tx.Init.Request = DMA_REQUEST_I2C3_TX;
        hdma_i2c3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_i2c3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c3_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_i2c3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_i2c3_tx.Init.Mode = DMA_NORMAL;
        hdma_i2c3_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_i2c3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_i2c3_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(i2cHandle, hdmatx, hdma_i2c3_tx);

        /* I2C3 interrupt Init */
        HAL_NVIC_SetPriority(I2C3_EV_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
        HAL_NVIC_SetPriority(I2C3_ER_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
        /* USER CODE BEGIN I2C3_MspInit 1 */

        /* USER CODE END I2C3_MspInit 1 */
    }
    else if (i2cHandle->Instance == I2C4) {
        /* USER CODE BEGIN I2C4_MspInit 0 */

        /* USER CODE END I2C4_MspInit 0 */

        __HAL_RCC_GPIOH_CLK_ENABLE();
        /**I2C4 GPIO Configuration
        PH11     ------> I2C4_SCL
        PH12     ------> I2C4_SDA
        */
        GPIO_InitStruct.Pin = I2C_SCL_Panel_1_Pin | I2C_SDA_Panel_1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        /* I2C4 clock enable */
        __HAL_RCC_I2C4_CLK_ENABLE();

        /* I2C4 DMA Init */
        /* I2C4_TX Init */
        hdma_i2c4_tx.Instance = BDMA_Channel2;
        hdma_i2c4_tx.Init.Request = BDMA_REQUEST_I2C4_TX;
        hdma_i2c4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_i2c4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c4_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_i2c4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_i2c4_tx.Init.Mode = DMA_NORMAL;
        hdma_i2c4_tx.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&hdma_i2c4_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(i2cHandle, hdmatx, hdma_i2c4_tx);

        /* I2C4 interrupt Init */
        HAL_NVIC_SetPriority(I2C4_EV_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
        HAL_NVIC_SetPriority(I2C4_ER_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);
        /* USER CODE BEGIN I2C4_MspInit 1 */

        /* USER CODE END I2C4_MspInit 1 */
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle) {

    if (i2cHandle->Instance == I2C1) {
        /* USER CODE BEGIN I2C1_MspDeInit 0 */

        /* USER CODE END I2C1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA
        */
        HAL_GPIO_DeInit(I2C_SCL_Touch_GPIO_Port, I2C_SCL_Touch_Pin);

        HAL_GPIO_DeInit(I2C_SDA_Touch_GPIO_Port, I2C_SDA_Touch_Pin);

        /* USER CODE BEGIN I2C1_MspDeInit 1 */

        /* USER CODE END I2C1_MspDeInit 1 */
    }
    else if (i2cHandle->Instance == I2C2) {
        /* USER CODE BEGIN I2C2_MspDeInit 0 */

        /* USER CODE END I2C2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C2_CLK_DISABLE();

        /**I2C2 GPIO Configuration
        PB10     ------> I2C2_SCL
        PB11     ------> I2C2_SDA
        */
        HAL_GPIO_DeInit(I2C_SCL_IOEXP_GPIO_Port, I2C_SCL_IOEXP_Pin);

        HAL_GPIO_DeInit(I2C_SDA_IOEXP_GPIO_Port, I2C_SDA_IOEXP_Pin);

        /* USER CODE BEGIN I2C2_MspDeInit 1 */

        /* USER CODE END I2C2_MspDeInit 1 */
    }
    else if (i2cHandle->Instance == I2C3) {
        /* USER CODE BEGIN I2C3_MspDeInit 0 */

        /* USER CODE END I2C3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C3_CLK_DISABLE();

        /**I2C3 GPIO Configuration
        PH7     ------> I2C3_SCL
        PH8     ------> I2C3_SDA
        */
        HAL_GPIO_DeInit(I2C_SCL_Panel_2_GPIO_Port, I2C_SCL_Panel_2_Pin);

        HAL_GPIO_DeInit(I2C_SDA_Panel_2_GPIO_Port, I2C_SDA_Panel_2_Pin);

        /* I2C3 DMA DeInit */
        HAL_DMA_DeInit(i2cHandle->hdmatx);

        /* I2C3 interrupt Deinit */
        HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
        /* USER CODE BEGIN I2C3_MspDeInit 1 */

        /* USER CODE END I2C3_MspDeInit 1 */
    }
    else if (i2cHandle->Instance == I2C4) {
        /* USER CODE BEGIN I2C4_MspDeInit 0 */

        /* USER CODE END I2C4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C4_CLK_DISABLE();

        /**I2C4 GPIO Configuration
        PH11     ------> I2C4_SCL
        PH12     ------> I2C4_SDA
        */
        HAL_GPIO_DeInit(I2C_SCL_Panel_1_GPIO_Port, I2C_SCL_Panel_1_Pin);

        HAL_GPIO_DeInit(I2C_SDA_Panel_1_GPIO_Port, I2C_SDA_Panel_1_Pin);

        /* I2C4 DMA DeInit */
        HAL_DMA_DeInit(i2cHandle->hdmatx);

        /* I2C4 interrupt Deinit */
        HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);
        /* USER CODE BEGIN I2C4_MspDeInit 1 */

        /* USER CODE END I2C4_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
