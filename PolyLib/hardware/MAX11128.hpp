#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

// Power management = 00

class MAX11128 : public baseDevice {
  public:
    void configurate(spiBus *busInterface, uint8_t nChannels, GPIO_TypeDef *gpioPort, uint16_t gpioPin,
                     uint32_t *command, uint32_t *adcData) {
        this->busInterface = busInterface;
        this->gpioPin = gpioPin;
        this->gpioPort = gpioPort;
        this->nChannels = nChannels;

        this->command = command;
        this->adcData = adcData;

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
        HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);
        // Reset the ADC for a clean start!
        // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);
        busInterface->transmit((uint8_t *)&resetCommand, 1);
        // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);

        microsecondsDelay(50);

        // Send Config Register
        // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);
        busInterface->transmit((uint8_t *)&commandConfigRegister32, 1);
        // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);

        microsecondsDelay(10);

        // start Init sample Command.. data receive will be triggered by EOC interrupt.
        // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);
        busInterface->transmit((uint8_t *)&standardSampleCommand, 1);
        // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);

        HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
    }

    void fetchNewData();

    uint32_t *command;
    uint32_t *adcData;

  private:
    spiBus *busInterface;
    uint8_t nChannels;
    uint16_t gpioPin;
    GPIO_TypeDef *gpioPort;
    uint32_t standardSampleCommand;
};