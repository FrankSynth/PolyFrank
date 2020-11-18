
#pragma once

#include "debughelper/debughelper.hpp"
#include "i2c.h"
#include "tim.h"

class PCA9555 {
  public:
    PCA9555(I2C_HandleTypeDef *i2cHandle, uint8_t i2cAddress) {

        this->i2cHandle = i2cHandle;
        this->i2cAddress = i2cAddress; // combine default address with custom adress

        i2cDeviceAddressing = i2cDeviceCode | i2cAddress << 1;
    }

    void init() {
        println("Expander Init");

        // set all pins to INPUT PULLUP

        uint8_t initData[3];

        // gain 2x  -> output = gain * internal reference (2048mV)

        initData[0] = 0x07;       // COMMANDBYTE //write Pin Configuration
        initData[1] = 0b11111111; // MODE PORT 1             1-> Input
        initData[2] = 0b11111111; // MODE PORT 0

        // timeout 50us

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddressing, initData, 3, 50) != HAL_OK) {
            Error_Handler();
            println("I2C Dac Transmit Error");
        }
        readInputs();
        PinState = newPinState;
    }

    uint8_t getPinChange() {

        readInputs();
        uint8_t pinChanged = 32 - __CLZ(__REV16(PinState ^ newPinState));

        PinState = newPinState;
        return pinChanged; // return number of leading zeros
    }

    uint16_t getPinChangeRAW() {

        readInputs();

        PinState = newPinState;

        return __REV16(PinState); // return number of leading zeros
    }

    void readInputs() {
        // uint8_t initData[2];

        uint8_t command = 0x01; // COMMANDBYTE // read Port 1 and 0

        // timeout 50us

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddressing, &command, 1, 50) != HAL_OK) {
            Error_Handler();
            println("I2C Dac Transmit Error");
        }

        if (HAL_I2C_Master_Receive(i2cHandle, i2cDeviceAddressing, (uint8_t *)&newPinState, 2, 50) != HAL_OK) {
            Error_Handler();
        }
    }

    uint16_t newPinState = 0;
    uint16_t PinState = 0;
    I2C_HandleTypeDef *i2cHandle;
    uint8_t i2cAddress = 0;       // default address
    uint8_t i2cDeviceCode = 0x40; // default Code
    uint8_t i2cDeviceAddressing = 0;
};
