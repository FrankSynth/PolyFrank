/**
 ******************************************************************************
 * File Name          : mdma.c
 * Description        : This file provides code for the configuration
 *                      of all the requested global MDMA transfers.
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
#include "mdma.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure MDMA                                                              */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
MDMA_HandleTypeDef hmdma_mdma_channel40_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel41_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel42_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel43_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel44_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel45_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel46_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel47_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel48_sw_0;

/**
 * Enable MDMA controller clock
 * Configure MDMA for global transfers
 *   hmdma_mdma_channel40_sw_0
 */
void MX_MDMA_Init(void) {

    /* MDMA controller clock enable */
    __HAL_RCC_MDMA_CLK_ENABLE();
    /* Local variables */

    /* Configure MDMA channel MDMA_Channel0 */
    /* Configure MDMA request hmdma_mdma_channel40_sw_0 on MDMA_Channel0 */
    hmdma_mdma_channel40_sw_0.Instance = MDMA_Channel0;
    hmdma_mdma_channel40_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel40_sw_0.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_mdma_channel40_sw_0.Init.Priority = MDMA_PRIORITY_HIGH;
    hmdma_mdma_channel40_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel40_sw_0.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_mdma_channel40_sw_0.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    hmdma_mdma_channel40_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_mdma_channel40_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    hmdma_mdma_channel40_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel40_sw_0.Init.BufferTransferLength = 1;
    hmdma_mdma_channel40_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_mdma_channel40_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_mdma_channel40_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel40_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel40_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel1;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel41_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel2;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel42_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel3;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel43_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel4;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel44_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel5;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel45_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel6;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel46_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel7;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel47_sw_0) != HAL_OK) {
        Error_Handler();
    }

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel41_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel41_sw_0.Instance = MDMA_Channel8;
    hmdma_mdma_channel41_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel41_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel41_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel41_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel41_sw_0.Init.SourceInc = MDMA_SRC_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestinationInc = MDMA_DEST_INC_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_DOUBLEWORD;
    hmdma_mdma_channel41_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel41_sw_0.Init.BufferTransferLength = 8;
    hmdma_mdma_channel41_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.DestBurst = MDMA_DEST_BURST_8BEATS;
    hmdma_mdma_channel41_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel41_sw_0.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_mdma_channel48_sw_0) != HAL_OK) {
        Error_Handler();
    }
    /* MDMA interrupt initialization */
    /* MDMA_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(MDMA_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}
/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
