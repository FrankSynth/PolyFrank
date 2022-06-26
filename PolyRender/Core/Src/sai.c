/**
 ******************************************************************************
 * File Name          : SAI.c
 * Description        : This file provides code for the configuration
 *                      of the SAI instances.
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
#include "sai.h"

#include "dma.h"
#include "gpio.h"
#include "render/renderAudioDef.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SAI_HandleTypeDef hsai_BlockA1;
DMA_HandleTypeDef hdma_sai1_a;

/* SAI1 init function */
void MX_SAI1_Init(void) {

    hsai_BlockA1.Instance = SAI1_Block_A;
    hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
    hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
    hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_HF;
    hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_96K;
    hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
    hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
    if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_24BIT, AUDIOCHANNELS) != HAL_OK) {
        Error_Handler();
    }
}
static uint32_t SAI1_client = 0;

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai) {

    GPIO_InitTypeDef GPIO_InitStruct;
    /* SAI1 */
    if (hsai->Instance == SAI1_Block_A) {
        /* SAI1 clock enable */
        if (SAI1_client == 0) {
            __HAL_RCC_SAI1_CLK_ENABLE();

            /* Peripheral interrupt init*/
            HAL_NVIC_SetPriority(SAI1_IRQn, 3, 0);
            HAL_NVIC_EnableIRQ(SAI1_IRQn);
        }
        SAI1_client++;

        /**SAI1_A_Block_A GPIO Configuration
        PE2     ------> SAI1_MCLK_A
        PE4     ------> SAI1_FS_A
        PE5     ------> SAI1_SCK_A
        PE6     ------> SAI1_SD_A
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        /* Peripheral DMA init*/

        hdma_sai1_a.Instance = DMA1_Stream1;
        hdma_sai1_a.Init.Request = DMA_REQUEST_SAI1_A;
        hdma_sai1_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_sai1_a.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_sai1_a.Init.MemInc = DMA_MINC_ENABLE;
        hdma_sai1_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sai1_a.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_sai1_a.Init.Mode = DMA_CIRCULAR;
        hdma_sai1_a.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        hdma_sai1_a.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        hdma_sai1_a.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_sai1_a.Init.MemBurst = DMA_MBURST_INC4;
        hdma_sai1_a.Init.PeriphBurst = DMA_PBURST_SINGLE;
        if (HAL_DMA_Init(&hdma_sai1_a) != HAL_OK) {
            Error_Handler();
        }

        /* Several peripheral DMA handle pointers point to the same DMA handle.
         Be aware that there is only one channel to perform all the requested DMAs. */
        __HAL_LINKDMA(hsai, hdmarx, hdma_sai1_a);
        __HAL_LINKDMA(hsai, hdmatx, hdma_sai1_a);
    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef *hsai) {

    /* SAI1 */
    if (hsai->Instance == SAI1_Block_A) {
        SAI1_client--;
        if (SAI1_client == 0) {
            /* Peripheral clock disable */
            __HAL_RCC_SAI1_CLK_DISABLE();
            HAL_NVIC_DisableIRQ(SAI1_IRQn);
        }

        /**SAI1_A_Block_A GPIO Configuration
        PE2     ------> SAI1_MCLK_A
        PE4     ------> SAI1_FS_A
        PE5     ------> SAI1_SCK_A
        PE6     ------> SAI1_SD_A
        */
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);

        HAL_DMA_DeInit(hsai->hdmarx);
        HAL_DMA_DeInit(hsai->hdmatx);
    }
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
