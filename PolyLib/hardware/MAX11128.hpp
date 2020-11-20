#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "spi.h"

// Power management = 00

class MAX11128 {
  public:
    MAX11128(SPI_HandleTypeDef *spi, uint8_t nChannels, GPIO_TypeDef *cs_pinPort, uint16_t cs_pin) {
        this->spi = spi;
        this->cs_pin = cs_pin;
        this->cs_pinPort = cs_pinPort;
        this->nChannels = nChannels;
    }

    void init() {

        // config register

        uint16_t crConfigRegAdr = 0b10000 << 11;
        uint16_t crRefSel = 0b0 << 10;
        uint16_t crAvgOn = 0b1 << 9;      // enable avg
        uint16_t crAvgAmount = 0b11 << 7; // avg 32 times
        uint16_t crAvgScans = 0b00 << 3;  // not used, only for repeat mode
        uint16_t crEcho = 0b0 << 2;       // echo commands

        uint16_t commandConfigRegister = crConfigRegAdr | crRefSel | crAvgOn | crAvgAmount | crAvgScans | crEcho;

        // scan control
        uint16_t scCntlRegAdr = 0b0 << 15;
        uint16_t scScan = 0b0011 << 11;          // scan N channels and store results
        uint16_t scChSel = (nChannels - 1) << 7; // (number of channels to scan) - 1
        uint16_t scReset = 0b00 << 5;            // no reset
        uint16_t scPM = 0b00 << 3;               // no pwr off
        uint16_t scChanID = 0b0 << 2;
        uint16_t scSWCNV = 0b1 << 1; // smaple with rising cs

        uint16_t commandScanControl = scCntlRegAdr | scScan | scChSel | scReset | scPM | scChanID | scSWCNV;
        standardSampleCommand = commandScanControl;

        // unipolar register
        uint16_t upUniPolRegAdr = 0b10001 << 11;

        // rest is 0, for single ended conversion
        uint16_t commandUniPolar = upUniPolRegAdr;

        // bipolar register
        uint16_t upBiPolRegAdr = 0b10010 << 11;

        uint16_t commandBiPolar = upBiPolRegAdr;

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&commandConfigRegister, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit ");
            Error_Handler();
        }
        println("MAX11128 SPI config SUCCESS ");
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        microsecondsDelay(1);

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&commandUniPolar, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit ");
            Error_Handler();
        }
        println("MAX11128 SPI config SUCCESS ");
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        microsecondsDelay(1);

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&commandBiPolar, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit ");
            Error_Handler();
        }
        println("MAX11128 SPI config SUCCESS ");
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        microsecondsDelay(1);

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&standardSampleCommand, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit");
            Error_Handler();
        }
        println("MAX11128 SPI config SUCCESS ");
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
    }

    void fetchNewData() {

        for (uint16_t i = 0; i < nChannels; i++) {

            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
            if (HAL_SPI_Receive(spi, (uint8_t *)&(adcData[i]), 1, 50) != HAL_OK) {
                HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
                println("Error MAX11128 SPI Receive");
                Error_Handler();
                return;
            }
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            // println("MAX11128 SPI Receive SUCCESS");
            microsecondsDelay(1);
        }

        fast_copy_f32((uint32_t *)adcData, (uint32_t *)data, 8);

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&standardSampleCommand, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit new command");
            Error_Handler();
            return;
        }

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
    }

    uint16_t data[16];

    uint16_t adcData[16];

  private:
    SPI_HandleTypeDef *spi;
    uint8_t nChannels;
    uint16_t cs_pin;
    GPIO_TypeDef *cs_pinPort;
    uint16_t standardSampleCommand;
};