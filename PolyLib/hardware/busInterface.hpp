#pragma once

#include <functional>
#include <string>
#include <vector>

typedef enum { SPI, I2C, VIRTUALI2C, UNDEFINED } busType;

typedef enum { BUS_NOTINIT, BUS_READY, BUS_BUSY, BUS_ERROR } busState;

// Base bus Class
class busInterface {
  public:
    virtual std::string *report() {
        status = "undefined bus interface \n\r";
        return &status;
    };
    void callRxComplete() { state = BUS_READY; };
    void callTxComplete() { state = BUS_READY; };
    void callError() { state = BUS_ERROR; };

    std::function<void()> rxCompleteFunction = nullptr;
    std::function<void()> txCompleteFunction = nullptr;
    std::function<void()> BUS_errorFunction = nullptr;

    std::string status;
    busState state = BUS_NOTINIT;

    busType type = UNDEFINED;

    uint32_t rxCounter;
    uint32_t txCounter;
};