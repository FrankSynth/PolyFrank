#pragma once
#include "datacore/datacore.hpp"
#include "gfx/guiActionHandler.hpp"
#include "gfx/guiBase.hpp"
#include "modules/modules.hpp"
#include <stdio.h>

#include "humanInterface/gamma.hpp"

#define LEDBRIGHTNESS_OFF 0

#define LEDBRIGHTNESS_MAX 4096
#define LEDBRIGHTNESS_MEDIUM LEDBRIGHTNESS_MAX / 2
#define LEDBRIGHTNESS_LOW LEDBRIGHTNESS_MAX / 4

#define LEDBRIGHTNESS_SETTING LEDBRIGHTNESS_MAX
#define LEDBRIGHTNESS_MODULEOUT LEDBRIGHTNESS_MAX
#define LEDBRIGHTNESS_MODULEIN LEDBRIGHTNESS_MAX / 2

#define LEDBRIGHTNESS_PATCHOUT LEDBRIGHTNESS_MAX
#define LEDBRIGHTNESS_PATCHIN LEDBRIGHTNESS_MAX
#define LEDBRIGHTNESS_PATCHMARKER LEDBRIGHTNESS_MAX

#define LEDMAXDISTANCE 60.f

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

typedef struct {
    std::function<void(uint16_t amount)> function;
    DataElement *data;
} potiFunctionStruct;

typedef enum { TOUCH_IO_PORT_A, TOUCH_IO_PORT_B, TOUCH_IO_PORT_C, TOUCH_IO_PORT_D, TOUCH_IO_PORT_E } TOUCH_IO_PORT;

// Encoder
void processEncoder();

// Touch
void processControlTouch();
void processPanelTouch(uint8_t layerID);

// Potentiometer
void processPanelPotis(uint32_t *adcData, uint32_t layer);
void mapPanelPotis(uint16_t activeChannel, uint16_t ID, uint16_t value);
void clearPotiState(uint32_t layer);
void pullFrontValues(uint32_t layer);

void LEDRender();

void LEDModuleOUT(uint32_t layerID);
void LEDModuleRenderbuffer(uint32_t layerID);
void LEDModuleSwitch(uint32_t layerID);

void LEDSingleSetting(Digital *digital);
void LEDDualSetting(Digital *digital, float amount = 1);
void LEDQuadSetting(Digital *digital);

void LEDSetPWM(uint16_t &pwmArrayEntry, uint16_t pwmRAW);
void LEDOutput(Output *output, uint16_t brightness);
void LEDInput(Input *input, uint16_t brightness);
void LEDDigital(Digital *digital, uint8_t id, uint16_t value);
void LEDModule(BaseModule *module);
void LEDModule(BaseModule *module, uint16_t brightness);
void LEDRenderbuffer(RenderBuffer *rBuffer);
void LEDAllInputs(uint32_t layerID);
void resetSystem();

void updateLEDDriverCurrent();

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

    bool clearOnReleaseOut = false;
    bool clearOnReleaseIn = false;
};
