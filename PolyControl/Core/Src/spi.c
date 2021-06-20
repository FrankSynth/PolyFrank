/**
 ******************************************************************************
 * File Name          : SPI.c
 * Description        : This file provides code for the configuration
 *                      of the SPI instances.
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
#include "spi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi4;
SPI_HandleTypeDef hspi5;
SPI_HandleTypeDef hspi6;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi4_tx;
DMA_HandleTypeDef hdma_spi5_tx;

/* SPI1 init function */
void MX_SPI1_Init(void) {

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_17BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 0x0;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}
/* SPI2 init function */
void MX_SPI2_Init(void) {

    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 0x0;
    hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        Error_Handler();
    }
}
/* SPI4 init function */
void MX_SPI4_Init(void) {

    hspi4.Instance = SPI4;
    hspi4.Init.Mode = SPI_MODE_MASTER;
    hspi4.Init.Direction = SPI_DIRECTION_1LINE;
    hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi4.Init.NSS = SPI_NSS_SOFT;
    hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi4.Init.CRCPolynomial = 0x0;
    hspi4.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi4.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi4.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    if (HAL_SPI_Init(&hspi4) != HAL_OK) {
        Error_Handler();
    }
}
/* SPI5 init function */
void MX_SPI5_Init(void) {

    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_1LINE;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi5.Init.CRCPolynomial = 0x0;
    hspi5.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    hspi5.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi5.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi5.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi5.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi5.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi5.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi5.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi5.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi5.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    if (HAL_SPI_Init(&hspi5) != HAL_OK) {
        Error_Handler();
    }
}
/* SPI6 init function */
void MX_SPI6_Init(void) {

    hspi6.Instance = SPI6;
    hspi6.Init.Mode = SPI_MODE_MASTER;
    hspi6.Init.Direction = SPI_DIRECTION_2LINES;
    hspi6.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi6.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi6.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi6.Init.NSS = SPI_NSS_SOFT;
    hspi6.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi6.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi6.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi6.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi6.Init.CRCPolynomial = 0x0;
    hspi6.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    hspi6.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi6.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi6.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi6.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi6.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi6.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi6.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi6.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    hspi6.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    if (HAL_SPI_Init(&hspi6) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *spiHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (spiHandle->Instance == SPI1) {
        /* USER CODE BEGIN SPI1_MspInit 0 */

        /* USER CODE END SPI1_MspInit 0 */
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**SPI1 GPIO Configuration
        PD7     ------> SPI1_MOSI
        PG9     ------> SPI1_MISO
        PG11     ------> SPI1_SCK
        */
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        /* SPI1 DMA Init */
        /* SPI1_TX Init */
        hdma_spi1_tx.Instance = DMA2_Stream0;
        hdma_spi1_tx.Init.Request = DMA_REQUEST_SPI1_TX;
        hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_spi1_tx.Init.Mode = DMA_NORMAL;
        hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(spiHandle, hdmatx, hdma_spi1_tx);

        /* SPI1 interrupt Init */
        HAL_NVIC_SetPriority(SPI1_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(SPI1_IRQn);
        /* USER CODE BEGIN SPI1_MspInit 1 */

        /* USER CODE END SPI1_MspInit 1 */
    }
    else if (spiHandle->Instance == SPI2) {
        /* USER CODE BEGIN SPI2_MspInit 0 */

        /* USER CODE END SPI2_MspInit 0 */
        /* SPI2 clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOI_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PC1     ------> SPI2_MOSI
        PC2_C     ------> SPI2_MISO
        PI1     ------> SPI2_SCK
        */
        GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        /* SPI2 DMA Init */
        /* SPI2_TX Init */
        hdma_spi2_tx.Instance = DMA2_Stream1;
        hdma_spi2_tx.Init.Request = DMA_REQUEST_SPI2_TX;
        hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_spi2_tx.Init.Mode = DMA_NORMAL;
        hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(spiHandle, hdmatx, hdma_spi2_tx);

        /* SPI2 interrupt Init */
        HAL_NVIC_SetPriority(SPI2_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(SPI2_IRQn);
        /* USER CODE BEGIN SPI2_MspInit 1 */

        /* USER CODE END SPI2_MspInit 1 */
    }
    else if (spiHandle->Instance == SPI4) {
        /* USER CODE BEGIN SPI4_MspInit 0 */

        /* USER CODE END SPI4_MspInit 0 */
        /* SPI4 clock enable */
        __HAL_RCC_SPI4_CLK_ENABLE();

        __HAL_RCC_GPIOE_CLK_ENABLE();
        /**SPI4 GPIO Configuration
        PE2     ------> SPI4_SCK
        PE6     ------> SPI4_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI4;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        /* SPI4 DMA Init */
        /* SPI4_TX Init */
        hdma_spi4_tx.Instance = DMA1_Stream0;
        hdma_spi4_tx.Init.Request = DMA_REQUEST_SPI4_TX;
        hdma_spi4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_spi4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi4_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_spi4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_spi4_tx.Init.Mode = DMA_NORMAL;
        hdma_spi4_tx.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_spi4_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_spi4_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(spiHandle, hdmatx, hdma_spi4_tx);

        /* SPI4 interrupt Init */
        HAL_NVIC_SetPriority(SPI4_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(SPI4_IRQn);
        /* USER CODE BEGIN SPI4_MspInit 1 */

        /* USER CODE END SPI4_MspInit 1 */
    }
    else if (spiHandle->Instance == SPI5) {
        /* USER CODE BEGIN SPI5_MspInit 0 */

        /* USER CODE END SPI5_MspInit 0 */
        /* SPI5 clock enable */
        __HAL_RCC_SPI5_CLK_ENABLE();

        __HAL_RCC_GPIOF_CLK_ENABLE();
        /**SPI5 GPIO Configuration
        PF7     ------> SPI5_SCK
        PF9     ------> SPI5_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        /* SPI5 DMA Init */
        /* SPI5_TX Init */
        hdma_spi5_tx.Instance = DMA1_Stream1;
        hdma_spi5_tx.Init.Request = DMA_REQUEST_SPI5_TX;
        hdma_spi5_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_spi5_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi5_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi5_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_spi5_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_spi5_tx.Init.Mode = DMA_NORMAL;
        hdma_spi5_tx.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_spi5_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_spi5_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(spiHandle, hdmatx, hdma_spi5_tx);

        /* SPI5 interrupt Init */
        HAL_NVIC_SetPriority(SPI5_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(SPI5_IRQn);
        /* USER CODE BEGIN SPI5_MspInit 1 */

        /* USER CODE END SPI5_MspInit 1 */
    }
    else if (spiHandle->Instance == SPI6) {
        /* USER CODE BEGIN SPI6_MspInit 0 */

        /* USER CODE END SPI6_MspInit 0 */
        /* SPI6 clock enable */
        __HAL_RCC_SPI6_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**SPI6 GPIO Configuration
        PA5     ------> SPI6_SCK
        PA6     ------> SPI6_MISO
        PA7     ------> SPI6_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF8_SPI6;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USER CODE BEGIN SPI6_MspInit 1 */

        /* USER CODE END SPI6_MspInit 1 */
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *spiHandle) {

    if (spiHandle->Instance == SPI1) {
        /* USER CODE BEGIN SPI1_MspDeInit 0 */

        /* USER CODE END SPI1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**SPI1 GPIO Configuration
        PD7     ------> SPI1_MOSI
        PG9     ------> SPI1_MISO
        PG11     ------> SPI1_SCK
        */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7);

        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9 | GPIO_PIN_11);

        /* SPI1 DMA DeInit */
        HAL_DMA_DeInit(spiHandle->hdmatx);

        /* SPI1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(SPI1_IRQn);
        /* USER CODE BEGIN SPI1_MspDeInit 1 */

        /* USER CODE END SPI1_MspDeInit 1 */
    }
    else if (spiHandle->Instance == SPI2) {
        /* USER CODE BEGIN SPI2_MspDeInit 0 */

        /* USER CODE END SPI2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI2 GPIO Configuration
        PC1     ------> SPI2_MOSI
        PC2_C     ------> SPI2_MISO
        PI1     ------> SPI2_SCK
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1 | GPIO_PIN_2);

        HAL_GPIO_DeInit(GPIOI, GPIO_PIN_1);

        /* SPI2 DMA DeInit */
        HAL_DMA_DeInit(spiHandle->hdmatx);

        /* SPI2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(SPI2_IRQn);
        /* USER CODE BEGIN SPI2_MspDeInit 1 */

        /* USER CODE END SPI2_MspDeInit 1 */
    }
    else if (spiHandle->Instance == SPI4) {
        /* USER CODE BEGIN SPI4_MspDeInit 0 */

        /* USER CODE END SPI4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI4_CLK_DISABLE();

        /**SPI4 GPIO Configuration
        PE2     ------> SPI4_SCK
        PE6     ------> SPI4_MOSI
        */
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2 | GPIO_PIN_6);

        /* SPI4 DMA DeInit */
        HAL_DMA_DeInit(spiHandle->hdmatx);

        /* SPI4 interrupt Deinit */
        HAL_NVIC_DisableIRQ(SPI4_IRQn);
        /* USER CODE BEGIN SPI4_MspDeInit 1 */

        /* USER CODE END SPI4_MspDeInit 1 */
    }
    else if (spiHandle->Instance == SPI5) {
        /* USER CODE BEGIN SPI5_MspDeInit 0 */

        /* USER CODE END SPI5_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI5_CLK_DISABLE();

        /**SPI5 GPIO Configuration
        PF7     ------> SPI5_SCK
        PF9     ------> SPI5_MOSI
        */
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_7 | GPIO_PIN_9);

        /* SPI5 DMA DeInit */
        HAL_DMA_DeInit(spiHandle->hdmatx);

        /* SPI5 interrupt Deinit */
        HAL_NVIC_DisableIRQ(SPI5_IRQn);
        /* USER CODE BEGIN SPI5_MspDeInit 1 */

        /* USER CODE END SPI5_MspDeInit 1 */
    }
    else if (spiHandle->Instance == SPI6) {
        /* USER CODE BEGIN SPI6_MspDeInit 0 */

        /* USER CODE END SPI6_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI6_CLK_DISABLE();

        /**SPI6 GPIO Configuration
        PA5     ------> SPI6_SCK
        PA6     ------> SPI6_MISO
        PA7     ------> SPI6_MOSI
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

        /* USER CODE BEGIN SPI6_MspDeInit 1 */

        /* USER CODE END SPI6_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
