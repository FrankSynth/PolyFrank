#ifdef POLYCONTROL
#pragma once

#include "debughelper/debughelper.hpp"
#include "hardware/PCA9548.hpp"
#include "i2c.h"

class IS31FL3216 {
  public:
    IS31FL3216(I2C_HandleTypeDef *i2cHandle, uint8_t i2cAddresse, uint8_t i2cBusSwitchAddress) {

        this->i2cHandle = i2cHandle;
        this->i2cBusSwitchAddress = i2cBusSwitchAddress;
        this->i2cDeviceAddress = i2cDeviceCode | (i2cAddresse << 1);
    }

    void init() {

        // Switch bus
        i2cBusSwitch.switchTarget(i2cBusSwitchAddress);

        // enable Chip

        uint8_t command[2];
        command[0] = 0x00;       // write to register 0x00
        command[1] = 0b00000000; // write 0000 0000

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, command, 2, 50) != HAL_OK) {
            Error_Handler();
        }

        // Set all pins to led-mode
        uint8_t enable[3]; // enable led
        enable[0] = 0x01;  // write to register 0x2
        enable[1] = 0xFF;
        enable[2] = 0xFF;

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, enable, 3, 50) != HAL_OK) {
            Error_Handler();
        }

        // set LED brigthness to 0
        setPWM(0); // set all LEDs to 0
    }

    void setPWM(uint8_t pwm) {
        // Switch bus
        i2cBusSwitch.switchTarget(i2cBusSwitchAddress);

        uint8_t data[17]; // pwm
        data[0] = 0x10;   // write to register 0x10 - 0x1F

        for (int x = 1; x < 17; x++) { // set brigthness for all leds
            data[x] = pwm;
        }

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, data, 17, 50) != HAL_OK) {
            Error_Handler();
        }

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0xB0
        commandUpdate[1] = 0b00000000; // write 0000 0000

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, commandUpdate, 2, 50) != HAL_OK) {
            Error_Handler();
        }
    }

    void setPWM(uint8_t pwm, uint8_t pin) {

        // Switch bus
        i2cBusSwitch.switchTarget(i2cBusSwitchAddress);

        if (pin > 16) {
            return;
        }

        uint8_t data[2];
        data[0] = 0x10 + pin; // write to register 0x10 + pin
        data[1] = pwm;

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, data, 2, 50) != HAL_OK) {
            Error_Handler();
        }

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0x00
        commandUpdate[1] = 0b00000000; // write 0000 0000

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, commandUpdate, 2, 50) != HAL_OK) {
            Error_Handler();
        }
    }

    void setPWM(uint8_t pwm, uint16_t pinRegister) {

        // Switch bus
        i2cBusSwitch.switchTarget(i2cBusSwitchAddress);

        uint8_t data[17];

        for (int x = 1; x < 17; x++) {
            if (pinRegister & (1 << pinRegister)) {
                data[x] = pwm;
            }
        }

        data[0] = 0x10; // write to register 0x10 + pin

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, data, 2, 50) != HAL_OK) {
            Error_Handler();
        }

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0x00
        commandUpdate[1] = 0b00000000; // write 0000 0000

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddress, commandUpdate, 2, 50) != HAL_OK) {
            Error_Handler();
        }
    }

    // uint16_t setLEDs() {}

    I2C_HandleTypeDef *i2cHandle;
    uint8_t i2cBusSwitchAddress = 0;
    uint8_t i2cDeviceCode = 0xE8;
    uint8_t i2cDeviceAddress = 0;
};

#endif