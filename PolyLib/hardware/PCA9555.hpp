
#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

class PCA9555 : public baseDevice {
  public:
    PCA9555() { deviceName = "PCA9555"; }

    void configurate(i2cBus *busInterface, uint8_t i2cAddress) {

        this->busInterface = busInterface;
        i2cDeviceAddress = i2cDeviceCode | i2cAddress << 1;

        // set all pins to INPUT PULLUP
        uint8_t initData[3];

        initData[0] = 0x07;       // COMMANDBYTE //write Pin Configuration
        initData[1] = 0b11111111; // MODE PORT 1             1-> Input
        initData[2] = 0b11111111; // MODE PORT 0

        busInterface->transmit(i2cDeviceAddress, initData, 3);

        initData[0] = 0x05; // COMMANDBYTE //write Pin Configuration
        initData[1] = 0x00; // MODE PORT 1             1-> Input
        initData[2] = 0x00; // MODE PORT 0

        busInterface->transmit(i2cDeviceAddress, initData, 3);

        readInputs();
        PinState = newPinState;

        state = DEVICE_READY;
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
        return __REV16(PinState);
    }

    void readInputs() {
        uint8_t command = 0x01; // COMMANDBYTE // read Port 1 and 0

        busInterface->transmit(i2cDeviceAddress, &command, 1);
        busInterface->receive(i2cDeviceAddress, (uint8_t *)&newPinState, 2);
    }

    uint16_t newPinState = 0;
    uint16_t PinState = 0;
    i2cBus *busInterface;
    uint8_t i2cDeviceCode = 0x40; // default Code
    uint8_t i2cDeviceAddress = 0;
};
