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
    }

    void addBus(busInterface *bus) {
        interfaces.push_back(bus);
        // println("add Bus");
    };
    void addDevice(baseDevice *device) {
        devices.push_back(device);
        // println("add Device");
    };
    void report(std::string &status) { // return status of all devices
        status.clear();

        status += "INFO || Interfaces\r\n";

        // collect interface status
        for (busInterface *interface : interfaces) {
            if (interface->type == UNDEFINED)
                interface->report(status);
            if (interface->type == I2C)
                ((i2cBus *)interface)->report(status);
            if (interface->type == SPI)
                ((spiBus *)interface)->report(status);
            if (interface->type == VIRTUALI2C)
                ((i2cVirtualBus *)interface)->report(status);
        }

        // collect device status
        status += "\nINFO || Devices : \r\n";

        for (baseDevice *device : devices) {
            device->report(status);
        }
    }

    void registerCallback(); // register callbacks

  private:
    std::vector<busInterface *> interfaces;
    std::vector<baseDevice *> devices;
};
