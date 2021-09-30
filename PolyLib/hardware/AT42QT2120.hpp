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
    }

    AT42QT2120(I2C_HandleTypeDef *I2C_HandleTypeDef) { // without busMultiplexer

        this->i2cHandle = i2cHandle;
        busMultiplexer = 0;
    }

    void init() {

        // hardware init -> need to wait for about 100ms until touch IC is ready. Then clear interrupt status.
        HAL_Delay(100);
        initDone = 1;

        configIC();
        readTouchStatus();
    }

    void init(PCA9548 *busSwitchIC) {

        // hardware init -> need to wait for about 100ms until touch IC is ready. Then clear interrupt status.
        HAL_Delay(100);
        initDone = 1;
        busMultiplexer = 1;
        this->busSwitchIC = busSwitchIC;

        configIC();
        readTouchStatus();
    }

    uint16_t readTouchStatus() {
        if (!initDone) {
            return 0;
        }
        if (busMultiplexer == 1) {
            if (busSwitchIC == nullptr) {
                PolyError_Handler("ERROR | COM | AT42QT I2C BusSwitchIC Nullptr");
            }
            busSwitchIC->switchTarget(i2cBusSwitchAddress);
        }

        uint8_t command = 0x02; // read from memory Address 0x02

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, &command, 1, 50) != HAL_OK) {
            PolyError_Handler("ERROR | COM | AT42QT I2C Transmit Failed");
        }

        uint8_t newKeyStatus[3];

        if (HAL_I2C_Master_Receive(i2cHandle, i2cDeviceAddress, newKeyStatus, 3, 50) != HAL_OK) {
            PolyError_Handler("ERROR | COM | AT42QT I2C Receive Failed");
        }

        keyStatus = *(uint16_t *)&newKeyStatus[1];

        return *(uint16_t *)&newKeyStatus[1];
    }

    void configIC() {

        if (busMultiplexer == 1) {
            if (busSwitchIC == nullptr) {
                PolyError_Handler("ERROR | COM | AT42QT I2C BusSwitchIC Nullptr");
                return;
            }
            busSwitchIC->switchTarget(i2cBusSwitchAddress);
        }

        uint8_t command[2];

        command[0] = 0x0B; // write to memory Address 11 (Detection Integrator)
        command[1] = 1;    // set detection integration to 1

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, command, 2, 50) != HAL_OK) {
            PolyError_Handler("ERROR | COM | AT42QT I2C Transmit Failed");
        }

        command[0] = 0x0C; // write to memory Address 11 (DTH)
        command[1] = 100;  // set detection integration to 5 *  160ms

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, command, 2, 50) != HAL_OK) {
            PolyError_Handler("ERROR | COM | AT42QT I2C Transmit Failed");
        }
    }

    uint16_t keyStatus = 0;

    I2C_HandleTypeDef *i2cHandle;
    uint8_t i2cBusSwitchAddress = 0;
    uint8_t i2cDeviceAddress = 0x38;

    PCA9548 *busSwitchIC = nullptr;

    uint8_t initDone = 0;
    uint8_t busMultiplexer = 0;
    uint8_t layerID = 0;
};

#endif