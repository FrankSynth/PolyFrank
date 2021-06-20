#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include "gpio.h"
#include <stdint.h>

class TS3A5017D {
  public:
    TS3A5017D(uint16_t maxChannel, GPIO_TypeDef *enablePinPort, uint16_t enablePin, GPIO_TypeDef *adrAPinPort,
              uint16_t adrAPin, GPIO_TypeDef *adrBPinPort, uint16_t adrBPin) {

        this->enablePinPort = enablePinPort;
        this->enablePin = enablePin;
        this->adrAPinPort = adrAPinPort;
        this->adrAPin = adrAPin;
        this->adrBPinPort = adrBPinPort;
        this->adrBPin = adrBPin;

        this->maxChannel = maxChannel;
    }
    TS3A5017D(uint16_t maxChannel, GPIO_TypeDef *adrAPinPort, uint16_t adrAPin, GPIO_TypeDef *adrBPinPort,
              uint16_t adrBPin) {

        // this->enablePinPort = enablePinPort;
        // this->enablePin = enablePin;
        this->adrAPinPort = adrAPinPort;
        this->adrAPin = adrAPin;
        this->adrBPinPort = adrBPinPort;
        this->adrBPin = adrBPin;

        this->maxChannel = maxChannel;
    }

    void enableChannels() { HAL_GPIO_WritePin(enablePinPort, enablePin, GPIO_PIN_RESET); }
    void disableChannels() { HAL_GPIO_WritePin(enablePinPort, enablePin, GPIO_PIN_SET); }

    void nextChannel() {
        currentChannel = changeIntLoop(currentChannel, 1, 0, maxChannel - 1);
        HAL_GPIO_WritePin(adrAPinPort, adrAPin, 0x1 & currentChannel ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(adrBPinPort, adrBPin, 0x2 & currentChannel ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    inline void setChannel(uint16_t channel) {
        currentChannel = testInt(channel, 0, maxChannel - 1);
        HAL_GPIO_WritePin(adrAPinPort, adrAPin, 0x1 & currentChannel ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(adrBPinPort, adrBPin, 0x2 & currentChannel ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    uint16_t currentChannel;

  private:
    GPIO_TypeDef *enablePinPort;
    uint16_t enablePin;
    GPIO_TypeDef *adrAPinPort;
    uint16_t adrAPin;
    GPIO_TypeDef *adrBPinPort;
    uint16_t adrBPin;

    uint16_t maxChannel;
};