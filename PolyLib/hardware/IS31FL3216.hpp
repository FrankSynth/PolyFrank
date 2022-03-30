#ifdef POLYCONTROL
#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

class IS31FL3216 : public baseDevice {
  public:
    void configurate(i2cVirtualBus *busInterface, uint8_t i2cAddresse) {

        this->busInterface = busInterface;
        this->i2cDeviceAddress = i2cDeviceCode | (i2cAddresse << 1);

        // enable Chip
        uint8_t command[2];
        command[0] = 0x00;       // write to register 0x00
        command[1] = 0b00000000; // write 0000 0000
        busInterface->transmit(i2cDeviceAddress, command, 2);

        // Set all pins to led-mode
        uint8_t enable[3]; // enable led
        enable[0] = 0x01;  // write to register 0x2
        enable[1] = 0xFF;
        enable[2] = 0xFF;

        busInterface->transmit(i2cDeviceAddress, enable, 3);

        // set LED brightness to 0
        setPWM(150); // set all LEDs to 0

        deviceName = "IS31FL3216";
        state = DEVICE_READY;
    }

    void setPWM(uint8_t pwm) {
        // Switch bus

        uint8_t data[17]; // pwm
        data[0] = 0x10;   // write to register 0x10 - 0x1F

        for (int x = 1; x < 17; x++) { // set brightness for all leds
            data[x] = pwm;
        }

        busInterface->transmit(i2cDeviceAddress, data, 17);

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0xB0
        commandUpdate[1] = 0b00000000; // write 0000 0000

        busInterface->transmit(i2cDeviceAddress, commandUpdate, 2);
    }

    void setPWM(uint8_t pwm, uint8_t pin) {

        if (pin > 16) {
            return;
        }

        uint8_t data[2];
        data[0] = 0x10 + pin; // write to register 0x10 + pin
        data[1] = pwm;

        busInterface->transmit(i2cDeviceAddress, data, 2);

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0x00
        commandUpdate[1] = 0b00000000; // write 0000 0000

        busInterface->transmit(i2cDeviceAddress, commandUpdate, 2);
    }

    void setPWM(uint8_t pwm, uint16_t pinRegister) {

        uint8_t data[17];

        for (int x = 1; x < 17; x++) {
            if (pinRegister & (1 << pinRegister)) {
                data[x] = pwm;
            }
        }

        data[0] = 0x10; // write to register 0x10 + pin

        busInterface->transmit(i2cDeviceAddress, data, 2);

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0x00
        commandUpdate[1] = 0b00000000; // write 0000 0000

        busInterface->transmit(i2cDeviceAddress, commandUpdate, 2);
    }
    void updateLEDs() {

        uint8_t changed = 0;

        for (int x = 0; x < 16; x++) {
            if (data[x + 1] != pwmValue[15 - x]) { // corrent orientation, check for difference
                data[x + 1] = pwmValue[15 - x];
                changed = 1;
            }
        }

        if (changed == 0) {
            return;
        }

        data[0] = 0x10; // write to register 0x10 + pin

        busInterface->transmit(i2cDeviceAddress, data, 17);

        // send update command

        uint8_t commandUpdate[2];
        commandUpdate[0] = 0xB0;       // write to register 0x00
        commandUpdate[1] = 0b00000000; // write 0000 0000

        busInterface->transmit(i2cDeviceAddress, commandUpdate, 2);
        uint32_t zero = 0;
        fastMemset(&zero, (uint32_t *)pwmValue, 4);
    }

    uint8_t data[17];

    uint8_t pwmValue[16];
    i2cVirtualBus *busInterface;

    uint8_t i2cDeviceCode = 0xE8;
    uint8_t i2cDeviceAddress = 0;
};

#endif