#ifdef POLYCONTROL
#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

#include "datacore/datalocation.hpp"

#define IS32REG_PWM 0x01
#define IS31REG_PWM 0x07

#define ISXXREG_CONTROL 0x00
#define ISXXREG_UPDATE 0x49
#define ISXXREG_PHASE 0x70
#define IS32REG_SCALE 0x4A
#define IS31REG_SCALE 0x4D

#define ISXXREG_GLBCURRENT 0x6E
#define ISXXREG_SPREAD 0x78
#define ISXXREG_RESET 0x7F

#define IS32LEDDATASIZE 36 * 2 + 2 // first byter start address | last byte update register

/// https://www.mouser.de/datasheet/2/198/IS32FL3237_DS-1950357.pdf

// RAM1 uint8_t data[IS32LEDDATASIZE]; // data + 1 command byte

class IS3XFL : public baseDevice {
  public:
    uint8_t *data;
    uint16_t *pwmData; // PWM Data (1byte offset)

    i2cBus *busInterface;

    uint8_t i2cDeviceCode = 0x68;
    uint8_t i2cDeviceAddress = 0;
};

class IS32FL3237 : public IS3XFL {
  public:
    IS32FL3237() { deviceName = "IS32FL3237"; }
    void configurate(i2cBus *busInterface, uint8_t i2cAddresse, uint8_t *databuffer) {
        //
        this->busInterface = busInterface;
        this->i2cDeviceAddress = i2cDeviceCode | (i2cAddresse << 1);

        data = databuffer;
        pwmData = (uint16_t *)&(databuffer[1]);

        data[0] = IS32REG_PWM;

        state = DEVICE_READY;

        // enable Chip
        sendCommand(ISXXREG_RESET, 0x00);
        sendCommand(ISXXREG_CONTROL, 0b00000111); // 16MHz 16Bit
        sendCommand(ISXXREG_PHASE, 0b11111111);   // enable phase delay
        sendLEDScaling(0xFF);

        setCurrent(10); // set current
    }
    void setCurrent(uint8_t current) { // configurate max current
        sendCommand(ISXXREG_GLBCURRENT, current);
    }
    void resetRegister() { // configurate max current
        sendCommand(ISXXREG_RESET, 0x00);
    }

    void sendCommand(uint8_t address, uint8_t data) {
        uint8_t command[2];

        command[0] = address; //
        command[1] = data;
        busInterface->transmit(i2cDeviceAddress, command, 2);
        // println("INFO | LED sendCommand");
    }
    void sendPWMData() {
        // send all PWM data, if update not successfull? -> check last byte ist 0x00 for update trigger
        busInterface->transmit(i2cDeviceAddress, data, IS32LEDDATASIZE, true);

        // println("INFO | LED sendPWMData");
    }

    void sendLEDScaling(uint8_t ledScale) {
        // send all PWM data, if update not successfull? -> check last byte ist 0x00 for update trigger

        uint8_t scaling[37];

        for (uint32_t i = 1; i < 37; i++) {
            scaling[i] = ledScale;
            /* code */
        }

        scaling[0] = IS32REG_SCALE;

        busInterface->transmit(i2cDeviceAddress, scaling, 37, false);
    }

    void clearPWMData() {
        uint32_t clearData = 0x00;

        memset((uint32_t *)pwmData, clearData, 36 * 2);
    }

    i2cBus *busInterface;
};

#define IS31LEDDATASIZE 12 * 2 + 1 // first byter start address

/// https://www.mouser.de/datasheet/2/198/IS32FL3237_DS-1950357.pdf

// RAM1 uint8_t data[LEDDATASIZE]; // data + 1 command byte

class IS31FL3205 : public IS3XFL {
  public:
    IS31FL3205() { deviceName = "IS31FL3205"; }

    void configurate(i2cBus *busInterface, uint8_t i2cAddresse, uint8_t *databuffer) {
        //
        this->busInterface = busInterface;
        this->i2cDeviceAddress = i2cDeviceCode | (i2cAddresse << 1);

        data = databuffer;
        pwmData = (uint16_t *)&data[1];

        data[0] = IS31REG_PWM;

        state = DEVICE_READY;

        // enable Chip
        sendCommand(ISXXREG_RESET, 0x00);
        sendCommand(ISXXREG_CONTROL, 0b00000111); // 16MHz 16Bit
        sendCommand(ISXXREG_PHASE, 0b11111111);   // enable phase delay
        sendLEDScaling(0xFF);

        setCurrent(10); // set current

        // // set LED brightness to 0
        for (uint32_t i = 0; i < IS31LEDDATASIZE; i++) // clear databuffer
            databuffer[i] = 0x00;
        { /* code */
        }
    }
    void setCurrent(uint8_t current) { // configurate max current
        sendCommand(ISXXREG_GLBCURRENT, current);
    }
    void resetRegister() { // configurate max current
        sendCommand(ISXXREG_RESET, 0x00);
    }

    void sendCommand(uint8_t address, uint8_t data) {
        uint8_t command[2];

        command[0] = address; //
        command[1] = data;
        busInterface->transmit(i2cDeviceAddress, command, 2);
    }

    void sendLEDScaling(uint8_t ledScale) {
        // send all PWM data, if update not successfull? -> check last byte ist 0x00 for update trigger

        uint8_t scaling[13];

        for (uint32_t i = 1; i < 13; i++) {
            scaling[i] = ledScale;
            /* code */
        }

        scaling[0] = IS32REG_SCALE;

        busInterface->transmit(i2cDeviceAddress, scaling, 13, false);
    }
    void sendPWMData() {
        // send all PWM data, if update not successfull? -> check last byte ist 0x00 for update trigger
        data[0] = IS31REG_PWM;

        busInterface->transmit(i2cDeviceAddress, data, IS31LEDDATASIZE, false);

        uint8_t command[2];

        command[0] = ISXXREG_UPDATE; //
        command[1] = 0x00;
        busInterface->transmit(i2cDeviceAddress, command, 2, false);
    }
    void clearPWMData() {
        uint32_t clearData = 0x00;

        memset((uint32_t *)pwmData, clearData, 24);
    }
    uint8_t *data;
    uint16_t *pwmData; // PWM Data (1byte offset)

    i2cBus *busInterface;

    uint8_t i2cDeviceCode = 0x68;
    uint8_t i2cReadDeviceAddress = 0;
    uint8_t i2cDeviceAddress = 0;
};

#endif