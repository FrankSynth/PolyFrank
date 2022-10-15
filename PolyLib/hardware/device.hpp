#pragma once

// BUS
#include "hardware/bus.hpp"

// DEVICES
// #include "debughelper/debughelper.hpp"

#include "hardware/AT42QT2120.hpp"
#include "hardware/IS3XFL.hpp"
#include "hardware/M95M01.hpp"
#include "hardware/MAX11128.hpp"
#include "hardware/PCA9555.hpp"
#include "hardware/TS3A5017D.hpp"
#include "hardware/rotary.hpp"

//////DEVICES Manager////////
// this class contain all external peripherie. deviceManager is split in BUS and devices. Bus get connnected to the
// interface and device get connected to bus
class devManager {
  public:
    devManager() { // bus initialization

        // reserve memory for status
        status.reserve(500);
    }

    void addBus(busInterface *bus) {
        interfaces.push_back(bus);
        // println("add Bus");
    };
    void addDevice(baseDevice *device) {
        devices.push_back(device);
        // println("add Device");
    };
    std::string *report() { // return status of all devices
        status.clear();

        status += "INFO || Interfaces\r\n";

        // collect interface status
        for (busInterface *interface : interfaces) {
            if (interface->type == UNDEFINED)
                status += *interface->report();
            if (interface->type == I2C)
                status += *((i2cBus *)interface)->report();
            if (interface->type == SPI)
                status += *((spiBus *)interface)->report();
            if (interface->type == VIRTUALI2C)
                status += *((i2cVirtualBus *)interface)->report();
        }

        // collect device status
        status += "\nINFO || Devices : \r\n";

        for (baseDevice *device : devices) {
            status += *device->report();
        }

        return &status; // return status report;
    }

    void registerCallback(); // register callbacks

  private:
    std::vector<busInterface *> interfaces;
    std::vector<baseDevice *> devices;
    std::string status;
};
