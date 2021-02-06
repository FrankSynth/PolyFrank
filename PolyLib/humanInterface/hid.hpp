#pragma once
#include "Rotary.hpp"
#include "gfx/gui.hpp"
#include "hardware/AT42QT2120.hpp"
#include "hardware/IS31FL3216.hpp"
#include "hardware/MAX11128.hpp"
#include "hardware/PCA9555.hpp"
#include "hardware/TS3A5017D.hpp"

#include "poly.hpp"
#define LEDBRIGHTNESS_LOW 50
#define LEDBRIGHTNESS_MEDIUM 100
#define LEDBRIGHTNESS_MAX 220
#define NUMBERENCODERS 4
#define NUMBER_PANELTOUCHICS 1
#define NUMBER_LEDDRIVER 2

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
void eventControlTouch(uint16_t touchState);

void initPotiMapping();
void processPanelPotis(uint8_t layerID);
void mapPanelPotis(uint16_t activeChannel, uint16_t ID, uint16_t value);

uint16_t patchLEDMapping(FOCUSMODE type, uint32_t id);

class PanelTouch {

  public:
    PanelTouch(uint8_t layerID) { this->layerID = layerID; }
    void event(uint16_t touchState, uint8_t port) {

        uint16_t event = pinState[port] ^ touchState;

        // println("------> eval");
        uint16_t push = event & touchState;
        uint16_t release = event & (~touchState);
        // println("------ > event ", event);

        // println("------ > push ", push);

        // println("------ > release ", release);

        for (int x = 0; x < 12; x++) { // pins per port

            if (push & (1 << x)) {
                // println("--------> eval Push");

                evaluate(x, port, 1);
            }

            else if (release & (1 << x)) {
                // println("--------> eval Release");

                evaluate(x, port, 0);
            }
        }
        // uint8_t newPinState = pinState[port] |= (1 << pin);
        // uint8_t oldPinState = pinState[port] |= (1 << pin);
    }

    // TODO panel touch buttons Mapping
    void evaluate(uint8_t pin, uint8_t port, uint8_t event) {
        if (event) {
            pinState[port] |= 1 << pin; // set pin State to 1
        }
        else {
            pinState[port] &= ~(1 << pin);
        }
        if (port == TOUCH_IO_PORT_A) {
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
                case 10: evaluateOutput(&allLayers[layerID]->lfoA.out, event); break;
                case 11: evaluateInput(&allLayers[layerID]->adsrA.iAttack, event); break;
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

    // TODO löschen eines Patches durch erneut klicken

    void evaluateInput(Input *pInput, uint8_t event) {
        // println("-------------> input");
        if (event) { // push Event
            if (activeOutput != nullptr) {
                allLayers[layerID]->addPatchInOutById(activeOutput->idGlobal, pInput->idGlobal);
                // println("-------------> patch INOUT");
            }
            else {
                // println("-------------> create Focus IN");

                setInputFocus(pInput);
            }
        }
        else { // release Event -> clear active input
            if (pInput == activeInput) {
                // println("-------------> Release Focus IN");

                activeInput = nullptr;
            }
        }
    }
    void evaluateOutput(Output *pOutput, uint8_t event) {
        // println("-------------> output");

        if (event) { // push Event

            if (activeInput != nullptr) {
                allLayers[layerID]->addPatchInOutById(pOutput->idGlobal, activeInput->idGlobal, 1);
                // println("-------------> patch OUTIN");
            }
            else if (activeOutput != nullptr) {

                if (activeOutput != pOutput) {

                    allLayers[layerID]->addPatchOutOutById(pOutput->idGlobal, activeOutput->idGlobal, 1);
                    // println("-------------> patch OUTOUT");
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
    void evaluateSetting(Digital *pSwitch) { pSwitch->nextValue(); }

    void setInputFocus(Input *pInput) {
        if (activeInput == nullptr && activeOutput == nullptr) {
            activeInput = pInput;

            location focus = {pInput->layerId, pInput->moduleId, pInput->id, FOCUSINPUT};
            ui.setFocus(focus);
        }
    }

    void setOutputFocus(Output *pOutput) {
        if (activeInput == nullptr && activeOutput == nullptr) {
            activeOutput = pOutput;

            location focus = {pOutput->layerId, pOutput->moduleId, pOutput->id, FOCUSOUTPUT};
            ui.setFocus(focus);
        }
    }

    Output *activeOutput = nullptr;
    Input *activeInput = nullptr;

    uint8_t layerID;

    // soviele anlegen wie es ports gibt
    uint16_t pinState[8];
};

void updatePatchLED();
void switchLEDMapping();

void patchLEDMapping(FOCUSMODE type, uint32_t id, uint8_t pwm);
