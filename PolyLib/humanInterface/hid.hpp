#pragma once
#include "Rotary.hpp"
#include "gfx/gui.hpp"
#include "hardware/AT42QT2120.hpp"
#include "hardware/IS31FL3216.hpp"
#include "hardware/MAX11128.hpp"
#include "hardware/PCA9548.hpp"
#include "hardware/PCA9555.hpp"
#include "hardware/TS3A5017D.hpp"

#include "poly.hpp"
#define LEDBRIGHTNESS_OFF 0
#define LEDBRIGHTNESS_LOW 10
#define LEDBRIGHTNESS_MEDIUM 50
#define LEDBRIGHTNESS_MAX 255
#define NUMBERENCODERS 5
#define NUMBER_PANELTOUCHICS 2
#define NUMBER_CONTROLTOUCHICS 2
#define NUMBER_LEDDRIVER 1

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

void initHID();
void processEncoder();

void processControlTouch();

void processPanelTouch(uint8_t layerID);
void eventPanelTouch(uint16_t touchState, uint8_t port);
void eventControlTouch(uint16_t touchStateA);

void initPotiMapping();
void processPanelPotis();
void resetPanelPotis();
void mapPanelPotis(uint16_t activeChannel, uint16_t ID, uint16_t value);

void patchLEDMappingInit();

void quadLEDSetting(uint8_t &LED1, uint8_t &LED2, uint8_t &LED3, uint8_t &LED4, int32_t &value);
void dualLEDSetting(uint8_t &LED1, uint8_t &LED2, int32_t &value);
void singleLEDSetting(uint8_t &LED, int32_t &value);

class PanelTouch {

  public:
    void eventLayer(uint8_t layerID, uint16_t touchState, uint8_t port) {
        this->layerID = layerID;

        uint16_t event = pinStateLayer[layerID][port] ^ touchState;
        uint16_t push = event & touchState;
        uint16_t release = event & (~touchState);

        for (int x = 0; x < 12; x++) { // pins per port

            if (push & (1 << x)) {

                evaluateLayer(x, port, 1);
            }

            else if (release & (1 << x)) {

                evaluateLayer(x, port, 0);
            }
        }
    }

    void eventControl(uint16_t touchState, uint8_t port) {

        uint16_t event = pinStateControl[port] ^ touchState;
        uint16_t push = event & touchState;
        uint16_t release = event & (~touchState);

        for (int x = 0; x < 12; x++) { // pins per port

            if (push & (1 << x)) {
                evaluateControl(x, port, 1);
            }

            else if (release & (1 << x)) {

                evaluateControl(x, port, 0);
            }
        }
    }
    void evaluateLayer(uint8_t pin, uint8_t port, uint8_t event) {
        if (event) {
            pinStateLayer[layerID][port] |= 1 << pin; // set pin State to 1
        }
        else {
            pinStateLayer[layerID][port] &= ~(1 << pin);
        }
        if (port == TOUCH_IO_PORT_A) {
            switch (pin) {
                case 0: evaluateModul((BaseModule *)&(allLayers[layerID]->oscA), event); break;
                case 1: evaluateModul((BaseModule *)&(allLayers[layerID]->oscB), event); break;
                case 2: evaluateModul((BaseModule *)&(allLayers[layerID]->sub), event); break;
                case 3: evaluateModul((BaseModule *)&(allLayers[layerID]->noise), event); break;
                case 4: evaluateModul((BaseModule *)&(allLayers[layerID]->envA), event); break;
                case 5: evaluateModul((BaseModule *)&(allLayers[layerID]->envF), event); break;
                case 6: evaluateOutput((Output *)&(allLayers[layerID]->envA.out), event); break;
                case 7: evaluateInput((Input *)&(allLayers[layerID]->ladder.iCutoff), event); break;
                case 8: evaluateModul((BaseModule *)&(allLayers[layerID]->lfoA), event); break;
                case 9: evaluateModul((BaseModule *)&(allLayers[layerID]->lfoB), event); break;
                case 10: evaluateOutput((Output *)&(allLayers[layerID]->envF.out), event); break;
                case 11: evaluateInput((Input *)&(allLayers[layerID]->steiner.iCutoff), event); break;
            }

            return;
        }
        if (port == TOUCH_IO_PORT_B) {
            switch (pin) {
                case 0: evaluateModul((BaseModule *)&(allLayers[layerID]->ladder), event); break;
                case 1: evaluateModul((BaseModule *)&(allLayers[layerID]->steiner), event); break;
                case 2: evaluateOutput((Output *)&(allLayers[layerID]->lfoA.out), event); break;
                case 3: evaluateInput((Input *)&(allLayers[layerID]->distort.iDistort), event); break;
                case 4: evaluateModul((BaseModule *)&(allLayers[layerID]->distort), event); break;
                case 5: evaluateOutput((Output *)&(allLayers[layerID]->imperfect.oSpread), event); break;
                case 6: evaluateOutput((Output *)&(allLayers[layerID]->lfoB.out), event); break;
                case 7: evaluateModul((BaseModule *)&(allLayers[layerID]->globalModule), event); break;
                case 8: evaluateInput((Input *)&(allLayers[layerID]->globalModule.iPan), event); break;
                case 9: evaluateSetting((Digital *)&(allLayers[layerID]->ladder.dSlope), event); break;
                case 10: evaluateSetting((Digital *)&(allLayers[layerID]->oscA.dVcfDestSwitch), event); break;
                case 11: evaluateInput((Input *)&(allLayers[layerID]->globalModule.iVCA), event); break;
            }

            return;
        }
        if (port == TOUCH_IO_PORT_C) {
            switch (pin) {
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                case 4: break;
                case 5: break;
                case 6: break;
                case 7: break;
                case 8: break;
                case 9: break;
                case 10: break;
                case 11: break;
            }

            return;
        }
        if (port == TOUCH_IO_PORT_D) {
            switch (pin) {
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                case 4: break;
                case 5: break;
                case 6: break;
                case 7: break;
                case 8: break;
                case 9: break;
                case 10: break;
                case 11: break;
            }

            return;
        }
    }

    void evaluateControl(uint8_t pin, uint8_t port, uint8_t event) {
        if (event) {
            pinStateControl[port] |= 1 << pin; // set pin State to 1
        }
        else {
            pinStateControl[port] &= ~(1 << pin);
        }
        if (port == TOUCH_IO_PORT_A) {
            switch (pin) {
                case 0: break;
                case 1: break;
                case 2: evaluateActionButton(&actionHandler.buttonLeft_3, event); break;
                case 3: evaluateActionButton(&actionHandler.buttonLeft_2, event); break;
                case 4: evaluateActionButton(&actionHandler.buttonLeft_1, event); break;
                case 5: evaluateActionHandle(&actionHandler.buttonHeader_1, event); break;
                case 6: evaluateActionHandle(&actionHandler.buttonHeader_2, event); break;
                case 7: evaluateActionHandle(&actionHandler.buttonHeader_3, event); break;
                case 8: evaluateActionHandle(&actionHandler.buttonHeader_4, event); break;
                case 9: evaluateActionButton(&actionHandler.buttonRight_1, event); break;
                case 10: evaluateActionButton(&actionHandler.buttonRight_2, event); break;
                case 11: evaluateActionButton(&actionHandler.buttonRight_3, event); break;
            }

            return;
        }
        if (port == TOUCH_IO_PORT_B) {
            switch (pin) {
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                case 4: break;
                case 6: evaluateOutput((Output *)&(allLayers[currentFocus.layer]->midi.oAftertouch), event); break;
                case 7: break;
                case 8: evaluateOutput((Output *)&(allLayers[currentFocus.layer]->midi.oPitchbend), event); break;
                case 9: break;
                case 10: evaluateOutput((Output *)&(allLayers[currentFocus.layer]->midi.oMod), event); break;
                case 11: break;
            }

            return;
        }
    }

    void evaluateInput(Input *pInput, uint8_t event) {
        if (event) { // push Event
            if (activeOutput != nullptr) {
                // MIDI Outputs specialCase -> get LayerID from target and reassign module with corrected layerID
                if ((activeOutput->moduleId == allLayers[0]->midi.id) && (activeOutput->layerId != pInput->layerId)) {
                    forceSetOutputFocus(
                        allLayers[pInput->layerId]->modules[activeOutput->moduleId]->getOutputs()[activeOutput->id]);
                }
                // filter cross Patches between layer
                if (pInput->layerId == activeOutput->layerId) {
                    // patch exists?

                    if (pInput->findPatchInOut(activeOutput->idGlobal, pInput->idGlobal)) {
                        allLayers[layerID]->removePatchInOutById(activeOutput->idGlobal, pInput->idGlobal);
                    }
                    else {
                        allLayers[layerID]->addPatchInOutById(activeOutput->idGlobal, pInput->idGlobal, 0.2);
                    }
                }
            }
            else {

                setInputFocus(pInput);
            }
        }
        else { // release Event -> clear active input
            if (pInput == activeInput) {
                activeInput = nullptr;
            }
        }
    }
    void evaluateOutput(Output *pOutput, uint8_t event) {

        if (event) { // push Event

            if (activeInput != nullptr) {
                // MIDI Outputs specialCase -> get LayerID from target and reassign module with corrected layerID
                if ((activeInput->moduleId == allLayers[0]->midi.id) && (activeInput->layerId != pOutput->layerId)) {
                    forceSetInputFocus(
                        allLayers[pOutput->layerId]->modules[activeInput->moduleId]->getInputs()[activeInput->id]);
                }
                // filter cross Patches between layer
                if (layerID == activeInput->layerId) {
                    // patch exists?
                    if (pOutput->findPatchInOut(pOutput->idGlobal, activeInput->idGlobal)) {
                        allLayers[layerID]->removePatchInOutById(pOutput->idGlobal, activeInput->idGlobal);
                    }
                    else {
                        allLayers[layerID]->addPatchInOutById(pOutput->idGlobal, activeInput->idGlobal, 1);
                    }
                }
            }

            else {
                setOutputFocus(pOutput);
                // println("-------------> create Focus OUT");
            }
        }

        else { // release Event -> clear active output
            if (pOutput == activeOutput) {
                // println("-------------> Release Focus OUT");

                activeOutput = nullptr;
            }
        }
    }

    void evaluateModul(BaseModule *pModule, uint8_t event) {

        if (event) { // push Event
            activeOutput = nullptr;
            activeInput = nullptr;
            setModulFocus(pModule);
        }
    }
    void evaluateActionButton(ButtonActionHandle *pButton, uint8_t event) {

        if (event) { // push Event
            pButton->state = PRESSED;
            if (pButton->handle.functionPointer != nullptr)
                pButton->handle.functionPointer();
        }
        else {
            pButton->state = RELEASED;
        }
    }

    void evaluateActionHandle(actionHandle *pAction, uint8_t event) {

        if (event) { // push Event
            if (pAction->functionPointer != nullptr)
                pAction->functionPointer();
        }
    }

    void evaluateSetting(Digital *pSwitch, uint8_t event) {
        if (event) {
            pSwitch->nextValueLoop();
        }
    }

    void setInputFocus(Input *pInput) {
        if (activeInput == nullptr && activeOutput == nullptr) {
            activeInput = pInput;

            newFocus = {pInput->layerId, pInput->moduleId, pInput->id, FOCUSINPUT};
        }
    }

    void setOutputFocus(Output *pOutput) {
        if (activeInput == nullptr && activeOutput == nullptr) {
            activeOutput = pOutput;

            newFocus = {pOutput->layerId, pOutput->moduleId, pOutput->id, FOCUSOUTPUT};
        }
    }
    void forceSetOutputFocus(Output *pOutput) {
        activeInput = nullptr;
        activeOutput = pOutput;

        newFocus = {pOutput->layerId, pOutput->moduleId, pOutput->id, FOCUSOUTPUT};
    }

    void forceSetInputFocus(Input *pInput) {
        activeOutput = nullptr;
        activeInput = pInput;

        newFocus = {pInput->layerId, pInput->moduleId, pInput->id, FOCUSINPUT};
    }

    void setModulFocus(BaseModule *pModule) { newFocus = {pModule->layerId, pModule->id, 0, FOCUSMODULE}; }

    Output *activeOutput = nullptr;
    Input *activeInput = nullptr;

    // soviele anlegen wie es ports gibt
    uint16_t pinStateLayer[2][8];
    uint16_t pinStateControl[8];

    uint8_t layerID;
};

void renderLED();
void switchLEDMapping();
void setLED(uint8_t layer, uint8_t port, uint8_t pin, uint32_t brigthness);
void setAllLEDs(uint8_t layer, uint8_t port, uint32_t brigthness);

void patchLEDMapping(FOCUSMODE type, uint32_t id, uint8_t pwm);
