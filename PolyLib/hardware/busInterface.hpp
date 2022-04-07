#pragma once

#include <functional>
#include <string>
#include <vector>

typedef enum { SPI, I2C, VIRTUALI2C, UNDEFINED } busType;

typedef enum {
    BUS_OK,
    BUS_READY,
    BUS_BUSY,
    BUS_SEND,
    BUS_RECEIVE,
    BUS_SENDRECEIVE,
    BUS_ERROR,
    BUS_TIMEOUT,
    BUS_NOTINIT
} busState;

// Base bus Class
class busInterface {
  public:
    virtual std::string *report() {
        status = "undefined bus interface \n\r";
        return &status;
    };
    inline void callRxComplete() { state = BUS_READY; };
    inline void callTxComplete() { state = BUS_READY; };
    inline void callError() { state = BUS_ERROR; };

    // std::function<void()> rxCompleteFunction = nullptr;
    // std::function<void()> txCompleteFunction = nullptr;
    // std::function<void()> BUS_errorFunction = nullptr;

    std::string status;
    volatile busState state = BUS_NOTINIT;

    busType type = UNDEFINED;

    uint32_t rxCounter;
    uint32_t txCounter;
};