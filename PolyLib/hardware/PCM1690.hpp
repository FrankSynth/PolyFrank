#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "debughelper/debughelper.hpp"
#include "render/renderAudioDef.h"
#include "sai.h"
#include "spi.h"

/**
 * @brief 8ch Audio DAC class
 *
 *
 * - PCM1690 supports multipe spi writes to config registers after single address
 * - uses 16 bit data words
 * - SAI FIFO 8 words - half full trigger
 * - DMA FIFO 4 words burst
 *
 */
class PCM1690 {
  public:
    PCM1690(SAI_HandleTypeDef *sai, SPI_HandleTypeDef *spi, int32_t *renderBuffer) {
        //
        this->sai = sai;
        this->spi = spi;
        this->renderBuffer = renderBuffer;
    }

    void init() {
        uint8_t config[2] = {0, 0};

        // if (HAL_SPI_Transmit(spi, config, 2, 50) != HAL_OK) {
        //     Error_Handler();
        // }

        // cs spi 4
        HAL_Delay(10);
        // disable sleep

        config[0] = 0x41;
        // config[1] = config[1] | (1 << 7);
        config[1] = config[1] | 0b1000; // mode

        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);

        if (HAL_SPI_Transmit(spi, config, 2, 50) != HAL_OK) {
            Error_Handler();
        }

        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
    }

    void startSAI() {
        if (HAL_SAI_Transmit_DMA(sai, (uint8_t *)renderBuffer, SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS) != HAL_OK) {
            Error_Handler();
        }
    }

  private:
    SAI_HandleTypeDef *sai;
    SPI_HandleTypeDef *spi;
    int32_t *renderBuffer;
};