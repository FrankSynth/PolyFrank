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
        uint16_t scScan = 0b0011 << 11; // scan N channels and store results
        uint16_t scChSel = 0b1111 << 7; // (number of channels to scan) - 1
        uint16_t scReset = 0b00 << 5;   // no reset
        uint16_t scPM = 0b00 << 3;      // no pwr off
        uint16_t scChanID = 0b1 << 2;
        uint16_t scSWCNV = 0b1 << 1; // smaple with rising cs

        uint16_t commandScanControl = scCntlRegAdr | scScan | scChSel | scReset | scPM | scChanID | scSWCNV;

        uint32_t commandConfigRegister32 = (uint32_t)commandConfigRegister << 1;
        standardSampleCommand = commandScanControl << 1;

        uint32_t resetCommand = 0b00000000001000000; // reset all

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&resetCommand, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit");
            Error_Handler();
        }
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        microsecondsDelay(50);

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&commandConfigRegister32, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit ");
            Error_Handler();
        }
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        microsecondsDelay(50);

        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        if (HAL_SPI_Transmit(spi, (uint8_t *)&standardSampleCommand, 1, 50) != HAL_OK) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
            println("Error MAX11128 SPI Transmit");
            Error_Handler();
        }
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        microsecondsDelay(50);

        // HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        // if (HAL_SPI_Transmit(spi, (uint8_t *)&commandUniPolar, 1, 50) != HAL_OK) {
        //     HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        //     println("Error MAX11128 SPI Transmit ");
        //     Error_Handler();
        // }
        // HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        // microsecondsDelay(50);

        // HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        // if (HAL_SPI_Transmit(spi, (uint8_t *)&commandBiPolar, 1, 50) != HAL_OK) {
        //     HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        //     println("Error MAX11128 SPI Transmit ");
        //     Error_Handler();
        // }
        // HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        // microsecondsDelay(50);

        println("MAX: init Done");
    }

    void fetchNewData() {

        uint32_t command = standardSampleCommand;

        for (uint16_t x = 0; x < nChannels; x++) {
            adcData[x] = 0;
            empty[x] = 0;
        }

        empty[15] = standardSampleCommand;

        uint16_t i = 0;

        for (i = 0; i < nChannels; i++) {

            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
            if (HAL_SPI_TransmitReceive(spi, (uint8_t *)&(empty[i]), (uint8_t *)&(adcData[i]), 1, 50) != HAL_OK) {
                HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
                println("Error MAX11128 SPI Receive");
                Error_Handler();
                return;
            }
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        }

        // data = adcData;
    }
    uint32_t empty[16];
    uint32_t data[16];

    uint32_t adcData[16];

  private:
    SPI_HandleTypeDef *spi;
    uint8_t nChannels;
    uint16_t cs_pin;
    GPIO_TypeDef *cs_pinPort;
    uint16_t standardSampleCommand;
};