#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "sai.h"
#include "spi.h"

#define SAIDMABUFFERSIZE 16 // 32bit

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
    PCM1690(SAI_HandleTypeDef *sai, SPI_HandleTypeDef *spi, uint32_t *renderBuffer) {
        //
        this->sai = sai;
        this->spi = spi;
        this->renderBuffer = renderBuffer;
    }

    void init() {}

    void startSAI() {
        if (HAL_SAI_Transmit_DMA(sai, (uint8_t *)renderBuffer, SAIDMABUFFERSIZE) != HAL_OK) {
            Error_Handler();
        }
    }

  private:
    SAI_HandleTypeDef *sai;
    SPI_HandleTypeDef *spi;
    uint32_t *renderBuffer;
};