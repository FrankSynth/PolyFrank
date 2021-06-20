#ifdef POLYCONTROL
#pragma once

#include "debughelper/debughelper.hpp"
#include "i2c.h"

class PCA9548 {
  public:
    PCA9548(I2C_HandleTypeDef *i2cHandle, uint8_t i2cAddress) {

        this->i2cHandle = i2cHandle;

        i2cDeviceAddress = i2cDeviceCode | i2cAddress << 1;
    }

    void switchTarget(uint8_t targetAddress) {

        if (activeAddress != (1 << targetAddress)) {
            activeAddress = (1 << targetAddress);
            // println("activeAddress: ", targetAddress);

            if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, &activeAddress, 1, 50) != HAL_OK) {
                Error_Handler();
                PolyError_Handler("ERROR | COM | PCA9548 I2C Transmit Failed");
            }
        }
    }

    uint8_t activeAddress = 0x00;

    I2C_HandleTypeDef *i2cHandle;
    uint8_t i2cDeviceCode = 0xE0; // default Code
    uint8_t i2cDeviceAddress = 0;
};

extern PCA9548 i2cBusSwitch[2];
extern PCA9548 i2cBusSwitchControl;

#endif