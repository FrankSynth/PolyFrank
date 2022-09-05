#pragma once
#include "datacore/datacore.hpp"
#include "gfx/guiActionHandler.hpp"
#include "gfx/guiBase.hpp"
#include "modules/modules.hpp"
#include <stdio.h>

#define LEDBRIGHTNESS_OFF 0
#define LEDBRIGHTNESS_LOW 10
#define LEDBRIGHTNESS_MEDIUM 50
#define LEDBRIGHTNESS_MAX 255
#define NUMBERENCODERS 6

typedef enum {
    TOUCH_IO_PIN_0,
    TOUCH_IO_PIN_1,
    TOUCH_IO_PIN_2,
    TOUCH_IO_PIN_3,
    TOUCH_IO_PIN_4,
    TOUCH_IO_PIN_5,
    TOUCH_IO_PIN_6,
    TOUCH_IO_PIN_7,
    TOUCH_IO_PIN_8,
    TOUCH_IO_PIN_9,
    TOUCH_IO_PIN_10,
    TOUCH_IO_PIN_11,
} TOUCH_IO_PIN;

typedef enum { TOUCH_IO_PORT_A, TOUCH_IO_PORT_B, TOUCH_IO_PORT_C, TOUCH_IO_PORT_D } TOUCH_IO_PORT;

void HIDConfig();

void processEncoder();

void processControlTouch();
void processPanelTouch(uint8_t layerID);

void potiMapping();
void processPanelPotis();
void mapPanelPotis(uint16_t activeChannel, uint16_t ID, uint16_t value);

void renderLED();

void patchLEDMappingInit();
void switchLEDMapping();
void setLED(uint8_t layer, uint8_t port, uint8_t pin, uint8_t brigthness);
void setAllLEDs(uint8_t layer, uint8_t port, uint32_t brigthness);

void patchLEDMapping(FOCUSMODE type, uint32_t id, uint8_t pwm);
void quadLEDSetting(uint8_t &LED1, uint8_t &LED2, uint8_t &LED3, uint8_t &LED4, int32_t &value);
void dualLEDSetting(uint8_t &LED1, uint8_t &LED2, int32_t &value);
void singleLEDSetting(uint8_t &LED, int32_t &value);

class PanelTouch {
  public:
    void eventLayer(uint8_t layerID, uint16_t touchState, uint8_t port);
    void eventControl(uint16_t touchState, uint8_t port);
    void evaluateLayer(uint8_t pin, uint8_t port, uint8_t event);
    void evaluateControl(uint8_t pin, uint8_t port, uint8_t event);

    void evaluateInput(Input *pInput, uint8_t event);
    void evaluateOutput(Output *pOutput, uint8_t event);
    void evaluateModul(BaseModule *pModule, uint8_t event);
    void evaluateActionButton(ButtonActionHandle *pButton, uint8_t event);
    void evaluateActionHandle(actionHandle *pAction, uint8_t event);
    void evaluateSetting(Digital *pSwitch, uint8_t event);

    void setInputFocus(Input *pInput);
    void setOutputFocus(Output *pOutput);

    void forceSetOutputFocus(Output *pOutput);
    void forceSetInputFocus(Input *pInput);

    void setModulFocus(BaseModule *pModule);

    Output *activeOutput = nullptr;
    Input *activeInput = nullptr;

    uint16_t pinStateLayer[2][8];
    uint16_t pinStateControl[8];

    uint8_t layerID;
};
