#ifdef POLYCONTROL
#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

class AT42QT2120 : public baseDevice {
  public:
    AT42QT2120() { deviceName = "AT42QT2120"; }

    void configurate(i2cVirtualBus *busInterface) {
        this->busInterface = busInterface;
        configIC();
        readTouchStatus();

        state = DEVICE_READY;
    }

    uint16_t readTouchStatus() {
        // if (state != DEVICE_READY) {
        //     return 0;
        // }

        // state == DEVICE_BUSY;

        // send memory Address 0x02 as start address for read
        uint8_t command = 0x02;
        busInterface->transmit(i2cDeviceAddress, &command, 1);

        // read key status
        uint8_t newKeyStatus[3];
        busInterface->receive(i2cDeviceAddress, newKeyStatus, 3);

        keyStatus = *(uint16_t *)&newKeyStatus[1];

        return keyStatus;
    }

    void configIC() {

        uint8_t command[2];

        command[0] = 0x0B; // write to memory Address 11 (Detection Integrator)
        command[1] = 1;    // set detection integration to 1

        busInterface->transmit(i2cDeviceAddress, command, 2);

        command[0] = 0x0C; // write to memory Address 11 (DTH)
        command[1] = 100;  // set detection integration to 5 *  160ms

        busInterface->transmit(i2cDeviceAddress, command, 2);
    }

    uint16_t keyStatus = 0;

    i2cVirtualBus *busInterface;
    uint8_t i2cDeviceAddress = 0x38;
};

#endif