#ifdef POLYCONTROL
#pragma once

#include "debughelper/debughelper.hpp"
#include "hardware/PCA9548.hpp"
#include "i2c.h"

class AT42QT2120 {
  public:
    AT42QT2120(I2C_HandleTypeDef *i2cHandle, uint8_t i2cBusSwitchAddress) {

        this->i2cHandle = i2cHandle;
        this->i2cBusSwitchAddress = i2cBusSwitchAddress; // combine default address with custom adress

        busMultiplexer = 1;
    }

    AT42QT2120(I2C_HandleTypeDef *I2C_HandleTypeDef) { // without busMultiplexer

        this->i2cHandle = i2cHandle;
        this->i2cBusSwitchAddress = i2cBusSwitchAddress; // combine default address with custom adress
        busMultiplexer = 0;
    }

    void init() {

        // hardware init.. need to wait for about 100ms until touch IC is ready. Then clear interrupt status.
        HAL_Delay(25);
        initDone = 1;

        readTouchStatus();
    }

    uint16_t readTouchStatus() {
        if (!initDone) {
            return 0;
        }

        if (busMultiplexer) {
            i2cBusSwitch.switchTarget(i2cBusSwitchAddress);
        }

        uint8_t command = 0x02; // read from memory Address 0x02

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, &command, 1, 50) != HAL_OK) {
            Error_Handler();
        }

        uint8_t newKeyStatus[3];

        if (HAL_I2C_Master_Receive(i2cHandle, i2cDeviceAddress, newKeyStatus, 3, 50) != HAL_OK) {
            Error_Handler();
        }

        keyStatus = *(uint16_t *)&newKeyStatus[1];

        return *(uint16_t *)&newKeyStatus[1];

        // TODO error status byte handling?
    }

    uint16_t keyStatus = 0;

    I2C_HandleTypeDef *i2cHandle;
    uint8_t i2cBusSwitchAddress = 0;
    uint8_t i2cDeviceAddress = 0x38;

    uint8_t initDone = 0;
    uint8_t busMultiplexer = 0;
};

#endif