#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

class DUALBU22210 : public baseDevice {

  public:
    void configurate(spiBus *busInterface, volatile uint16_t *outBuffer) {
        this->busInterface = busInterface;
        this->outBuffer = outBuffer;

        deviceName = "DUALBU22210";
        state = DEVICE_READY;
    }

    // send out 2*10 packages of dac data,

    void enableOutputs() {

        enableOutputCommand[0] = 0xB000;
        enableOutputCommand[1] = 0xB000;
        busInterface->transmit((uint8_t *)enableOutputCommand, 1);
    }

    void send() { busInterface->transmit((uint8_t *)outBuffer, 10, true); }

    void preparePackage() {

        for (uint32_t x = 0; x < 20; x++) {
            outBuffer[x] = data[x] | addressPrep[x];
        }
    }

    uint16_t enableOutputCommand[2];

    uint16_t data[20];
    spiBus *busInterface;

  private:
    volatile uint16_t *outBuffer;
    static const uint16_t addressPrep[20];
};
