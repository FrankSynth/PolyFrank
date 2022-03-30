#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

// Power management = 00

class MAX11128 : public baseDevice {
  public:
    void configurate(spiBus *busInterface, uint8_t i2cAddress, uint8_t nChannels, GPIO_TypeDef *cs_pinPort,
                     uint16_t cs_pin) {
        this->busInterface = busInterface;
        this->cs_pin = cs_pin;
        this->cs_pinPort = cs_pinPort;
        this->nChannels = nChannels;

        setup();

        deviceName = "MAX11128";
        state = DEVICE_READY;
    }

    void setup() {

        // config register

        uint16_t crConfigRegAdr = 0b10000 << 11;
        uint16_t crRefSel = 0b0 << 10;
        uint16_t crAvgOn = 0b1 << 9;      // enable avg
        uint16_t crAvgAmount = 0b11 << 7; // avg  32 times
        uint16_t crAvgScans = 0b00 << 3;  // not used, only for repeat mode
        uint16_t crEcho = 0b0 << 2;       // echo commands

        uint16_t commandConfigRegister = crConfigRegAdr | crRefSel | crAvgOn | crAvgAmount | crAvgScans | crEcho;

        // scan control
        uint16_t scCntlRegAdr = 0b0 << 15;
        uint16_t scScan = 0b0011 << 11; // scan N channels and store results
        uint16_t scChSel = 0b1011 << 7; // (number of channels to scan) - 1
        uint16_t scReset = 0b00 << 5;   // no reset
        uint16_t scPM = 0b00 << 3;      // no pwr off
        uint16_t scChanID = 0b1 << 2;
        uint16_t scSWCNV = 0b1 << 1; // smaple with rising cs

        uint16_t commandScanControl = scCntlRegAdr | scScan | scChSel | scReset | scPM | scChanID | scSWCNV;

        uint32_t commandConfigRegister32 = (uint32_t)commandConfigRegister << 1;
        standardSampleCommand = commandScanControl << 1;

        command[nChannels - 1] = standardSampleCommand;

        uint32_t resetCommand = ((uint32_t)0x840) << 1; // reset command

        // Reset the ADC for a clean start!
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        busInterface->transmit((uint8_t *)&resetCommand, 1);
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        microsecondsDelay(50);

        // Send Config Register
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        busInterface->transmit((uint8_t *)&commandConfigRegister32, 1);
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);

        microsecondsDelay(10);

        // start Init sample Command.. data receive will be triggered by EOC interrupt.
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
        busInterface->transmit((uint8_t *)&standardSampleCommand, 1);
        HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
    }

    void fetchNewData() {

        // reset sampleCommand.. SPI TransmitReceive will corrupt the transmit buffer!
        static uint32_t zero = 0;
        fastMemset(&zero, command, nChannels - 1);

        command[nChannels - 1] = standardSampleCommand;

        // receive new samples and send sample command
        for (uint16_t i = 0; i < nChannels; i++) {
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_RESET);
            busInterface->transmitReceive((uint8_t *)&(command[i]), (uint8_t *)&(adcData[i]), 1);
            HAL_GPIO_WritePin(cs_pinPort, cs_pin, GPIO_PIN_SET);
        }
    }

    uint32_t command[16];
    uint32_t data[16];

    uint32_t adcData[16];

  private:
    spiBus *busInterface;
    uint8_t nChannels;
    uint16_t cs_pin;
    GPIO_TypeDef *cs_pinPort;
    uint32_t standardSampleCommand;
};