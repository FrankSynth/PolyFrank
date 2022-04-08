#pragma once

//////BASE Driver CLASS////////
typedef enum { DEVICE_READY, DEVICE_BUSY, DEVICE_ERROR, DEVICE_NOTINIT } deviceState;

#include <functional>
#include <string>

// base driver class for all device driver
class baseDevice {
  public:
    std::string *report() { // return status for device
        status.clear();
        status = deviceName;

        status += " STATE:  ";

        if (state == DEVICE_NOTINIT) {
            status += "not init";
        }
        else if (state == DEVICE_READY) {
            status += "ready";
        }
        else if (state == DEVICE_BUSY) {
            status += "busy";
        }
        else if (state == DEVICE_ERROR) {
            status += "error";
        }

        status += "\r\n";

        return &status;
    }

    deviceState state = DEVICE_NOTINIT;

    std::string status;
    std::string deviceName;

  private:
    std::function<void()> rxCompleteCallback;
    std::function<void()> txCompleteCallback;
    std::function<void()> errorCallback;
};
