#ifdef POLYCONTROL

#include "hid.hpp"
#include "gfx/gui.hpp"
#include "hardware/device.hpp"
#include "hidStartup.hpp"

#include <functional>

// Control Touch
extern std::vector<AT42QT2120> touchControl;

// Panel Touch
extern std::vector<AT42QT2120> touchPanelA;
extern std::vector<AT42QT2120> touchPanelB;

// IO Expander
extern PCA9555 ioExpander;

// ADC
extern MAX11128 adcA;
extern MAX11128 adcB;

// LED Driver
extern IS32FL3237 ledDriver[2][2];
extern IS31FL3205 ledDriverControl;

extern TS3A5017D multiplexer;

// Encoder
extern rotary encoders[NUMBERENCODERS];
extern tactileSwitch switches[NUMBERENCODERS];

extern GUI ui;

// Touch
PanelTouch touch;

// number layer, number multiplex, number channels
typedef enum { VALUEBELOW, VALUEABOVE, VALUEGRABED } presetGrabState;

potiFunctionStruct potiFunctionPointer[2][4][12];
uint16_t panelADCStates[2][4][12];

presetGrabState panelGrab[2][4][12];

float panelADCInterpolate[2][4][12];

//////////// ENCODER ///////////
void processEncoder() {
    uint32_t state = (GPIOD->IDR & 0x0038) << 13;
    state |= (uint32_t)ioExpander.getPinChangeRAW() & 0xFFFF;

    // println(state);

    for (int x = 0; x < NUMBERENCODERS; x++) {

        encoders[x].process(state);
        switches[x].process(state);
    }
}

//////////// TOUCH ///////////

void processPanelTouch(uint8_t layerID) {
    if (layerID == 0) {
        // println("processPanel1");
        for (unsigned int x = 0; x < touchPanelA.size(); x++) {
            uint16_t touchState = touchPanelA[x].readTouchStatus();

            touch.eventLayer(layerID, touchState, x);
            if (HAL_GPIO_ReadPin(Panel_2_INT_GPIO_Port,
                                 Panel_2_INT_Pin)) { // interrupt cleared?
                FlagHandler::Panel_0_Touch_Interrupt = false;
                return;
            }
        }
    }
    else if (layerID == 1) {
        // println("processPanel2");
        for (unsigned int x = 0; x < touchPanelB.size(); x++) {
            uint16_t touchState = touchPanelB[x].readTouchStatus();

            touch.eventLayer(layerID, touchState, x);
            if (HAL_GPIO_ReadPin(Panel_1_INT_GPIO_Port,
                                 Panel_1_INT_Pin)) { // interrupt cleared?
                FlagHandler::Panel_1_Touch_Interrupt = false;
                return;
            }
        }
    }
}

void processControlTouch() {

    for (unsigned int x = 0; x < touchControl.size(); x++) {
        uint16_t touchState = touchControl[x].readTouchStatus();

        touch.eventControl(touchState, x);

        if (HAL_GPIO_ReadPin(Control_INT_GPIO_Port,
                             Control_INT_Pin)) { // interrupt cleared?
            FlagHandler::Control_Touch_Interrupt = false;

            return;
        }
    }
}

//////////// POTIS ///////////

void pullFrontValues(uint32_t layer) {

    allLayers[layer]->clearPresetLocks();
    clearPotiState(layer);
}

void clearPotiState(uint32_t layer) {
    for (uint32_t multiplex = 0; multiplex < 4; multiplex++) { // for all Channels
        for (uint32_t channel = 0; channel < 12; channel++) {  // for all Channels
            panelADCStates[layer][multiplex][channel] = 0;
        }
    }
}

void processPanelPotis(uint32_t *adcData, uint32_t layer) {
    static int16_t octaveSwitchLastScan[2][2] = {{0, 0}, {0, 0}}; // threshold for jitter reduction  [layer][switch]
    static int16_t octaveSwitchCounter[2][2] = {{0, 0}, {0, 0}};  // threshold for jitter reduction  [layer][switch]

    SCB_InvalidateDCache_by_Addr(adcData, 48); // clear dcache

    uint32_t multiplex = (multiplexer.currentChannel + 3) % 4;

    for (uint32_t channel = 0; channel < 12; channel++) { // for all Channels
        if (potiFunctionPointer[layer][multiplex][channel].data != nullptr) {

            uint16_t potiData = ((adcData[channel] >> 1) & 0xFFF);
            if (((multiplex == 0) & (channel == 1)) || ((multiplex == 0) & (channel == 2))) { // octave switches

                // this filters out the switching artefact of the octave switches
                //  check data stability
                if (std::abs(octaveSwitchLastScan[layer][channel - 1] - potiData) < 2) { // test difference
                    octaveSwitchCounter[layer][channel - 1]++;
                }
                else {
                    octaveSwitchCounter[layer][channel - 1] = 0;
                }

                // value stable -> apply new octave switch value
                if (octaveSwitchCounter[layer][channel - 1] > 10) {
                    if (std::abs(panelADCStates[layer][multiplex][channel] - potiData) >= 50) {
                        panelADCStates[layer][multiplex][channel] = potiData;

                        potiFunctionPointer[layer][multiplex][channel].function(
                            panelADCStates[layer][multiplex][channel]);
                    }
                }

                octaveSwitchLastScan[layer][channel - 1] = potiData; // store new data
            }
            else {
                // interpolation

                panelADCInterpolate[layer][multiplex][channel] = fast_lerp_f32(
                    panelADCInterpolate[layer][multiplex][channel], (float)((adcData[channel] >> 1) & 0xFFF), 0.25);

                uint16_t difference = std::abs(panelADCStates[layer][multiplex][channel] - potiData);
                // check if value is loaded from preset

                bool updateValue =
                    false; // update value flag to check differen condition -> preset lock, grabbing, noise reduction
                if (potiFunctionPointer[layer][multiplex][channel].data->presetLock) {
                    if (globalSettings.presetValueHandling.value ==
                        1) { // we need to grab the value before we can change it
                        if (potiFunctionPointer[layer][multiplex][channel].data->presetLock) { // we have a lock?
                            bool compare =
                                ((Analog *)(potiFunctionPointer[layer][multiplex][channel].data))->value < potiData;
                            if (panelGrab[layer][multiplex][channel] ==
                                VALUEGRABED) { // lock and valuegrabed? -> preset value loaded
                                if (compare) { // store grab status
                                    panelGrab[layer][multiplex][channel] = VALUEABOVE;
                                }
                                else {
                                    panelGrab[layer][multiplex][channel] = VALUEBELOW;
                                }
                            }
                            else if (panelGrab[layer][multiplex][channel] !=
                                     compare) { // grab status changed? -> value grabbed
                                panelGrab[layer][multiplex][channel] = VALUEGRABED;
                                updateValue = true; // update poti to release lock
                            }
                        }
                    }

                    if (globalSettings.presetValueHandling.value == 0) { // we need to move a poti to overwrite the
                                                                         // value

                        if (difference >> 3) {  // check value change is big enough for preset overwrite
                            updateValue = true; // update poti to release lock
                        }
                    }
                    if (updateValue) {
                        potiFunctionPointer[layer][multiplex][channel].data->presetLock = false;
                    }
                }
                else if (difference >> 1) { // we have no lock-> normal mode
                    updateValue = true;
                }

                if (updateValue) { // active quickview only on stronger value changes

                    if (potiFunctionPointer[layer][multiplex][channel].data->quickview) { // filter quickview potis
                        if (difference >> 3 || (difference >> 1 && (quickViewTimer < quickViewTimeout))) {
                            if ((layerSendMode == SINGLELAYER) ||
                                (layer != 1)) { // only layer 0 when dual mode is active

                                quickView.modul = potiFunctionPointer[layer][multiplex][channel].data->moduleId;
                                quickView.layer = layer;
                                quickViewTimer = 0;
                            }
                        }
                    }
                    panelADCStates[layer][multiplex][channel] =
                        (uint16_t)panelADCInterpolate[layer][multiplex][channel];
                    potiFunctionPointer[layer][multiplex][channel].function(panelADCStates[layer][multiplex][channel]);
                }
            }
        }
    }
}

extern GUI ui;
//////////// LED ///////////

void updateLEDDriverCurrent() {

    ledDriver[0][0].setCurrent(globalSettings.dispLED.value);
    ledDriver[0][1].setCurrent(globalSettings.dispLED.value);
    ledDriver[1][0].setCurrent(globalSettings.dispLED.value);
    ledDriver[1][1].setCurrent(globalSettings.dispLED.value);

    ledDriverControl.setCurrent(globalSettings.dispLED.value);
}

void setLEDFlags() {
    if (FlagHandler::ledDriverATransmit == DRIVER_IDLE) {
        FlagHandler::ledDriverATransmit = DRIVER_START;
        FlagHandler::ledDriverA_Interrupt = true;
    }
    else {
        // println("ERROR | LEDDriverA Transmit not Complete");
    }

    if (FlagHandler::ledDriverBTransmit == DRIVER_IDLE) {
        FlagHandler::ledDriverBTransmit = DRIVER_START;
        FlagHandler::ledDriverB_Interrupt = true;
    }
    else {
        // println("ERROR | LEDDriverB Transmit not Complete");
    }

    if (FlagHandler::ledDriverControlTransmit == DRIVER_IDLE) {
        FlagHandler::ledDriverControl_Interrupt = true;
    }
    else {
        // println("ERROR | LEDDriverControl Transmit not Complete");
    }
}

void LEDRender() {

    static bool introFrame = true;
    static elapsedMillis timerIntroScreen;

    ledDriverControl.clearPWMData(); // delete old data

    if (introFrame) {

        LEDIntroFrame();
        setLEDFlags();

        if (timerIntroScreen > 3000)
            introFrame = false;

        return;
    }

    uint16_t breathing =
        uint16_t((fast_sin_f32((float)millis() / 300.f) + 1.f) / 2.f * (float)LEDBRIGHTNESS_PATCHMARKER);

    for (uint32_t i = 0; i < 2; i++) { // for both Layer
        // clear Data

        for (uint32_t x = 0; x < 2; x++) {
            ledDriver[i][x].clearPWMData(); // delete old data
        }
        if (liveData.voiceHandler.livemodeMergeLayer.value == 0 || i == 0) {

            if (allLayers[i]->layerState.value) {
                bool patchDraw = false;

                if (ui.activePanel == &ui.guiPanelFocus && cachedFocus.layer == i) { // show patches?
                    // LEDModuleRenderbuffer(i);
                    if (cachedFocus.modul < allLayers[i]->modules.size()) {
                        if (cachedFocus.type == FOCUSOUTPUT) {
                            patchDraw = true;
                            if (cachedFocus.id < allLayers[i]->modules[cachedFocus.modul]->getOutputs().size()) {

                                Output *output = allLayers[i]->modules[cachedFocus.modul]->getOutputs()[cachedFocus.id];
                                LEDOutput(output, LEDBRIGHTNESS_PATCHOUT);

                                for (uint32_t p = 0; p < output->getPatchesInOut().size(); p++) {
                                    Input *target = output->getPatchesInOut()[p]->targetIn;
                                    if (allLayers[i]->modules[target->moduleId]->moduleType ==
                                        MODULE_MIX) { // mixer double use LED
                                        LEDSetPWM(
                                            ledDriver[target->layerId][target->LEDPortID].pwmData[target->LEDPinID],
                                            breathing);
                                        LEDSetPWM(
                                            ledDriver[target->layerId][target->LEDPortID2].pwmData[target->LEDPinID2],
                                            breathing);
                                    }
                                    else {
                                        LEDInput(target, breathing);
                                    }
                                }
                            }
                        }
                        else if (cachedFocus.type == FOCUSINPUT) {
                            patchDraw = true;
                            if (cachedFocus.id < allLayers[i]->modules[cachedFocus.modul]->getInputs().size()) {
                                Input *input = allLayers[i]->modules[cachedFocus.modul]->getInputs()[cachedFocus.id];
                                LEDInput(input, LEDBRIGHTNESS_PATCHIN);

                                for (uint32_t p = 0; p < input->getPatchesInOut().size(); p++) {
                                    Output *source = input->getPatchesInOut()[p]->sourceOut;
                                    LEDOutput(source, breathing);
                                }
                            }
                        }
                    }
                }
                if (patchDraw == false) {
                    LEDAllInputs(i); // drawInputData to LED
                    LEDModuleSwitch(i);
                    LEDModuleOUT(i);
                }
            }
        }
    }

    setLEDFlags();
}

void LEDSetPWM(uint16_t &pwmArrayEntry, uint16_t pwmRAW) {
    pwmArrayEntry = gamma_lut[pwmRAW];
}
void LEDOutput(Output *output, uint16_t brightness) {
    if (output->LEDPortID == 2) { // MIDI MODULE Control

        LEDSetPWM(ledDriverControl.pwmData[output->LEDPinID], brightness);
    }
    else {
        LEDSetPWM(ledDriver[output->layerId][output->LEDPortID].pwmData[output->LEDPinID], brightness);
    }
}

void LEDInput(Input *input, uint16_t brightness) {
    LEDSetPWM(ledDriver[input->layerId][input->LEDPortID].pwmData[input->LEDPinID], brightness);
}

// MODULE LED OUTPUT
void LEDModule(BaseModule *module) {

    if (module->moduleType == MODULE_LFO) {

        LFO *lfo = (LFO *)module;
        float outputData;

        if (lfo->dGateTrigger)
            outputData = lfo->currentSampleRAW.currentSample[liveData.voiceHandler.lastVoiceID[module->layerId]];
        else
            outputData = lfo->currentSampleRAW.currentSample[0];

        LEDSetPWM(ledDriver[module->layerId][module->LEDPortID].pwmData[module->LEDPinID],
                  (outputData + 1) * (float)(LEDBRIGHTNESS_MODULEOUT / 2));
    }
    else if (module->moduleType == MODULE_ADSR) {
        float outputData =
            ((ADSR *)module)->currentSampleRAW.currentSample[liveData.voiceHandler.lastVoiceID[module->layerId]];
        LEDSetPWM(ledDriver[module->layerId][module->LEDPortID].pwmData[module->LEDPinID],
                  abs(outputData * (float)(LEDBRIGHTNESS_MODULEOUT)));
    }
}
void LEDModule(BaseModule *module, uint16_t brightness) {

    if (module->moduleType == MODULE_LFO) {
        LEDSetPWM(ledDriver[module->layerId][module->LEDPortID].pwmData[module->LEDPinID], brightness);
    }
    else if (module->moduleType == MODULE_ADSR) {
        LEDSetPWM(ledDriver[module->layerId][module->LEDPortID].pwmData[module->LEDPinID], brightness);
    }
}

void LEDRenderbuffer(RenderBuffer *rBuffer) { // TODO Check ranges?
    LEDSetPWM(ledDriver[rBuffer->layerId][rBuffer->LEDPortID].pwmData[rBuffer->LEDPinID], rBuffer->currentSample[0]);
}

void LEDModuleOUT(uint32_t layerID) {
    LEDModule(&allLayers[layerID]->lfoA);
    LEDModule(&allLayers[layerID]->lfoB);
    LEDModule(&allLayers[layerID]->envA);
    LEDModule(&allLayers[layerID]->envF);
}

void LEDModuleRenderbuffer(uint32_t layerID) {
    LEDRenderbuffer(&allLayers[layerID]->mixer.oscALevelLadder);
    LEDRenderbuffer(&allLayers[layerID]->mixer.oscALevelSteiner);
    LEDRenderbuffer(&allLayers[layerID]->mixer.oscBLevelLadder);
    LEDRenderbuffer(&allLayers[layerID]->mixer.oscBLevelSteiner);
    LEDRenderbuffer(&allLayers[layerID]->mixer.noiseLevelLadder);
    LEDRenderbuffer(&allLayers[layerID]->mixer.noiseLevelSteiner);
    LEDRenderbuffer(&allLayers[layerID]->mixer.subLevelLadder);
    LEDRenderbuffer(&allLayers[layerID]->mixer.subLevelSteiner);
}

#include "humanInterface/gamma.hpp"

void LEDAllInputs(uint32_t layerID) {

    for (BaseModule *m : allLayers[layerID]->getModules()) {
        for (Analog *a : m->getPotis()) {
            if (a->input != nullptr) {
                if (a->input->LEDPortID != 0xFF) {
                    // active LED
                    float value = a->input->renderBuffer->currentSample[liveData.voiceHandler.lastVoiceID[layerID]];

                    if (a->min < 0) { // we excpect symmetrical values
                        value = value / a->max;
                        value = testFloat(abs(value), -1, 1);
                    }
                    else {
                        value = (value - a->min) / (a->max - a->min);
                        value = testFloat(value, 0, 1);
                    }
                    LEDInput(a->input, (float)(LEDBRIGHTNESS_MODULEIN)*value);
                }
            }
        }
    }
}

void LEDModuleSwitch(uint32_t layerID) {

    LEDSingleSetting(&allLayers[layerID]->oscA.dOctaveSwitchSub);
    LEDSingleSetting(&allLayers[layerID]->oscB.dSync);

    LEDSingleSetting(&allLayers[layerID]->lfoA.dAlignLFOs);
    LEDSingleSetting(&allLayers[layerID]->lfoA.dGateTrigger);

    LEDSingleSetting(&allLayers[layerID]->lfoB.dAlignLFOs);
    LEDSingleSetting(&allLayers[layerID]->lfoB.dGateTrigger);

    Analog *analog;
    float value;

    analog = &allLayers[layerID]->mixer.aOSCALevel;
    value = analog->input->renderBuffer->currentSample[liveData.voiceHandler.lastVoiceID[layerID]];
    value = (value - analog->min) / (analog->max - analog->min);
    value = testFloat(value, 0, 1);
    LEDDualSetting(&allLayers[layerID]->mixer.dOSCADestSwitch, value);

    analog = &allLayers[layerID]->mixer.aOSCBLevel;
    value = analog->input->renderBuffer->currentSample[liveData.voiceHandler.lastVoiceID[layerID]];
    value = (value - analog->min) / (analog->max - analog->min);
    value = testFloat(value, 0, 1);
    LEDDualSetting(&allLayers[layerID]->mixer.dOSCBDestSwitch, value);

    analog = &allLayers[layerID]->mixer.aSUBLevel;
    value = analog->input->renderBuffer->currentSample[liveData.voiceHandler.lastVoiceID[layerID]];
    value = (value - analog->min) / (analog->max - analog->min);
    value = testFloat(value, 0, 1);
    LEDDualSetting(&allLayers[layerID]->mixer.dSUBDestSwitch, value);

    analog = &allLayers[layerID]->mixer.aNOISELevel;
    value = analog->input->renderBuffer->currentSample[liveData.voiceHandler.lastVoiceID[layerID]];
    value = (value - analog->min) / (analog->max - analog->min);
    value = testFloat(value, 0, 1);
    LEDDualSetting(&allLayers[layerID]->mixer.dNOISEDestSwitch, value);

    LEDQuadSetting(&allLayers[layerID]->steiner.dMode);
    LEDQuadSetting(&allLayers[layerID]->ladder.dSlope);
}

void LEDDigital(Digital *digital, uint8_t id, uint16_t value) {
    if (id < digital->LEDPinID.size()) {
        uint32_t pin = digital->LEDPinID[id];
        uint32_t port = digital->LEDPortID[id];

        LEDSetPWM(ledDriver[digital->layerId][port].pwmData[pin], value);
    }
}

void LEDQuadSetting(Digital *digital) {
    if (digital == nullptr)
        return;
    switch (digital->valueMapped) {
        case 0: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_SETTING);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 2, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 3, LEDBRIGHTNESS_OFF);
            break;
        }
        case 1: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_SETTING);
            LEDDigital(digital, 2, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 3, LEDBRIGHTNESS_OFF);

            break;
        }
        case 2: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 2, LEDBRIGHTNESS_SETTING);
            LEDDigital(digital, 3, LEDBRIGHTNESS_OFF);
            break;
        }
        case 3: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 2, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 3, LEDBRIGHTNESS_SETTING);
            break;
        }
        default: break;
    }
}

void LEDDualSetting(Digital *digital, float amount) {
    if (digital == nullptr)
        return;

    switch (digital->valueMapped) {
        case 0: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_SETTING * amount);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            break;
        }
        case 1: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_SETTING * amount);
            break;
        }
        case 2: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_SETTING * amount);
            LEDDigital(digital, 1, LEDBRIGHTNESS_SETTING * amount);
            break;
        }
        case 3: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            break;
        }
        default: break;
    }
}
void LEDSingleSetting(Digital *digital) {
    if (digital == nullptr)
        return;

    switch (digital->valueMapped) {
        case 0: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            break;
        }
        case 1: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_SETTING);
            break;
        }
        default: break;
    }
}

//////////// PanelTouch ///////////

RAM1 uint8_t saveSlots[2][3][LAYER_STORESIZE];
RAM1 uint8_t tempSaveStorage[LAYER_STORESIZE];

void PanelTouch::evaluateFunctionButtons(uint32_t buttonID, uint32_t event) {
    if (event) { // Push Event
        switch (globalSettings.functionButtons.value) {
            case 0: {                       // Fast Save Slots
                if (globalSettings.shift) { // When shift is pressed

                    if (saveSlotState[cachedFocus.layer][buttonID] == SLOTFREE) { // First time only store
                        saveSlotState[cachedFocus.layer][buttonID] = SLOTUSED;
                        allLayers[cachedFocus.layer]->getLayerConfiguration(
                            (int32_t *)saveSlots[cachedFocus.layer][buttonID],
                            false); // store current Layer
                    }
                    else if (saveSlotState[cachedFocus.layer][buttonID] == SLOTUSED) {
                        allLayers[cachedFocus.layer]->getLayerConfiguration((int32_t *)tempSaveStorage,
                                                                            false); // store current Layer
                        allLayers[cachedFocus.layer]->setLayerConfigration(
                            (int32_t *)saveSlots[cachedFocus.layer][buttonID], false);

                        memcpy(saveSlots[cachedFocus.layer][buttonID], &tempSaveStorage,
                               LAYER_STORESIZE); // Copy preset in Storage}
                    }
                }
            }

            default: break;
        }
    }
};

void PanelTouch::eventLayer(uint8_t layerID, uint16_t touchState, uint8_t port) {

    if (liveData.voiceHandler.livemodeMergeLayer.value == 1 && layerID == 1)
        return;

    this->layerID = layerID;

    uint16_t event = pinStateLayer[layerID][port] ^ touchState;
    uint16_t push = event & touchState;
    uint16_t release = event & (~touchState);

    for (int x = 0; x < 12; x++) { // pins per port

        if (push & (1 << x)) {

            evaluateLayer(x, port, 1);
            // println("LAYER::   pin:  ", x, "  port:  ", port, "  event:   ", 1);
        }

        else if (release & (1 << x)) {

            evaluateLayer(x, port, 0);
            // println("LAYER::   pin:  ", x, "  port:  ", port, "  event:   ", 0);
        }
    }
}

void PanelTouch::eventControl(uint16_t touchState, uint8_t port) {
    uint16_t event = pinStateControl[port] ^ touchState;
    uint16_t push = event & touchState;
    uint16_t release = event & (~touchState);

    for (int x = 0; x < 12; x++) { // pins per port

        if (push & (1 << x)) {
            evaluateControl(x, port, 1);

            // println("CONTROL::   pin:  ", x, "  port:  ", port, "  event:   ", 1);
        }

        else if (release & (1 << x)) {

            evaluateControl(x, port, 0);
            // println("CONTROL::   pin:  ", x, "  port:  ", port, "  event:   ", 0);
        }
    }
}
void PanelTouch::evaluateLayer(uint8_t pin, uint8_t port, uint8_t event) {
    if (event) {
        pinStateLayer[layerID][port] |= 1 << pin; // set pin State to 1
    }
    else {
        pinStateLayer[layerID][port] &= ~(1 << pin);
    }

    if (port == TOUCH_IO_PORT_B) {
        switch (pin) {
            case 0: evaluateSetting((Digital *)&(allLayers[layerID]->oscB.dSync), event); break;
            case 1: evaluateInput((Input *)&(allLayers[layerID]->oscB.iEffect), event); break;
            case 2: evaluateInput((Input *)&(allLayers[layerID]->oscB.iMorph), event); break;
            case 3: evaluateInput((Input *)&(allLayers[layerID]->oscB.iFM), event); break;
            case 4: evaluateModul((BaseModule *)&(allLayers[layerID]->oscB), event); break;
            case 5: evaluateSetting((Digital *)&(allLayers[layerID]->oscA.dOctaveSwitchSub), event); break;
            case 6: evaluateInput((Input *)&(allLayers[layerID]->oscA.iEffect), event); break;
            case 7: evaluateInput((Input *)&(allLayers[layerID]->oscA.iMorph), event); break;
            case 8: evaluateInput((Input *)&(allLayers[layerID]->oscA.iFM), event); break;
            case 9: evaluateInput((Input *)&(allLayers[layerID]->feel.iGlide), event); break;
            case 10: evaluateModul((BaseModule *)&(allLayers[layerID]->feel), event); break;
            case 11: evaluateModul((BaseModule *)&(allLayers[layerID]->oscA), event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_C) {
        if (activeOutput == nullptr) {
            switch (pin) {
                case 0: evaluateInput((Input *)&(allLayers[layerID]->steiner.iResonance), event); break;
                case 1: evaluateInput((Input *)&(allLayers[layerID]->steiner.iCutoff), event); break;
                case 2: evaluateSetting((Digital *)&(allLayers[layerID]->steiner.dMode), event); break;
                case 3: evaluateSetting((Digital *)&(allLayers[layerID]->mixer.dNOISEDestSwitch), event); break;
                case 4: evaluateSetting((Digital *)&(allLayers[layerID]->mixer.dSUBDestSwitch), event); break;
                case 5: evaluateSetting((Digital *)&(allLayers[layerID]->mixer.dOSCBDestSwitch), event); break;
                case 6: evaluateSetting((Digital *)&(allLayers[layerID]->mixer.dOSCADestSwitch), event); break;
                case 7: evaluateModul((BaseModule *)&(allLayers[layerID]->steiner), event); break;
                case 8: evaluateModul((BaseModule *)&(allLayers[layerID]->mixer), event); break;
                case 9: evaluateInput((Input *)&(allLayers[layerID]->feel.iDetune), event); break;
                case 10: evaluateOutput((Output *)&(allLayers[layerID]->feel.oSpread), event); break;
                case 11: break;
            }
        }
        else {
            switch (pin) {
                case 0: evaluateInput((Input *)&(allLayers[layerID]->steiner.iResonance), event); break;
                case 1: evaluateInput((Input *)&(allLayers[layerID]->steiner.iCutoff), event); break;
                case 2: evaluateSetting((Digital *)&(allLayers[layerID]->steiner.dMode), event); break;
                case 3: evaluateInput((Input *)&(allLayers[layerID]->mixer.iNOISELevel), event); break;
                case 4: evaluateInput((Input *)&(allLayers[layerID]->mixer.iSUBLevel), event); break;
                case 5: evaluateInput((Input *)&(allLayers[layerID]->mixer.iOSCBLevel), event); break;
                case 6: evaluateInput((Input *)&(allLayers[layerID]->mixer.iOSCALevel), event); break;
                case 7: evaluateModul((BaseModule *)&(allLayers[layerID]->steiner), event); break;
                case 8: evaluateModul((BaseModule *)&(allLayers[layerID]->mixer), event); break;
                case 9: evaluateInput((Input *)&(allLayers[layerID]->feel.iDetune), event); break;
                case 10: evaluateOutput((Output *)&(allLayers[layerID]->feel.oSpread), event); break;
                case 11: break;
            }
        }

        return;
    }
    if (port == TOUCH_IO_PORT_D) {
        switch (pin) {
            case 0: evaluateSetting((Digital *)&(allLayers[layerID]->lfoB.dAlignLFOs), event); break;
            case 1: evaluateSetting((Digital *)&(allLayers[layerID]->lfoB.dGateTrigger), event); break;
            case 2: evaluateInput((Input *)&(allLayers[layerID]->lfoB.iAmount), event); break;
            case 3: evaluateInput((Input *)&(allLayers[layerID]->lfoB.iShape), event); break;
            case 4: evaluateInput((Input *)&(allLayers[layerID]->lfoB.iFreq), event); break;
            case 5: evaluateModul((BaseModule *)&(allLayers[layerID]->lfoB), event); break;
            case 6: evaluateSetting((Digital *)&(allLayers[layerID]->lfoA.dAlignLFOs), event); break;
            case 7: evaluateSetting((Digital *)&(allLayers[layerID]->lfoA.dGateTrigger), event); break;
            case 8: evaluateOutput((Output *)&(allLayers[layerID]->lfoA.out), event); break;
            case 9: break;
            case 10: break;
            case 11: evaluateOutput((Output *)&(allLayers[layerID]->lfoB.out), event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_A) {
        switch (pin) {
            case 0: evaluateInput((Input *)&(allLayers[layerID]->lfoA.iAmount), event); break;
            case 1: evaluateInput((Input *)&(allLayers[layerID]->lfoA.iShape), event); break;
            case 2: evaluateInput((Input *)&(allLayers[layerID]->lfoA.iFreq), event); break;
            case 3: evaluateModul((BaseModule *)&(allLayers[layerID]->lfoA), event); break;
            case 4: evaluateModul((BaseModule *)&(allLayers[layerID]->ladder), event); break;
            case 5: evaluateInput((Input *)&(allLayers[layerID]->ladder.iCutoff), event); break;
            case 6: evaluateInput((Input *)&(allLayers[layerID]->ladder.iResonance), event); break;
            case 7: evaluateInput((Input *)&(allLayers[layerID]->ladder.iLevel), event); break;
            case 8: evaluateSetting((Digital *)&(allLayers[layerID]->ladder.dSlope), event); break;
            case 9: break;
            case 10: evaluateInput((Input *)&(allLayers[layerID]->steiner.iLevel), event); break;
            case 11: evaluateModul((BaseModule *)&(allLayers[layerID]->out), event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_E) {
        switch (pin) {
            case 0: evaluateInput((Input *)&(allLayers[layerID]->out.iPan), event); break;
            case 1: evaluateInput((Input *)&(allLayers[layerID]->out.iVCA), event); break;
            case 2: evaluateOutput((Output *)&(allLayers[layerID]->envA.out), event); break;
            case 3: evaluateInput((Input *)&(allLayers[layerID]->envA.iAmount), event); break;
            case 4: break;
            case 5: evaluateOutput((Output *)&(allLayers[layerID]->envF.out), event); break;
            case 6: evaluateInput((Input *)&(allLayers[layerID]->envF.iAmount), event); break;
            case 7: break;
            case 8: break;
            case 9: break;
            case 10: evaluateModul((BaseModule *)&(allLayers[layerID]->envF), event); break;
            case 11: evaluateModul((BaseModule *)&(allLayers[layerID]->envA), event); break;
        }

        return;
    }
}

void PanelTouch::evaluateControl(uint8_t pin, uint8_t port, uint8_t event) {
    if (event) {
        pinStateControl[port] |= 1 << pin; // set pin State to 1
    }
    else {
        pinStateControl[port] &= ~(1 << pin);
    }

    // TODO ENABLE NEW
    if (port == TOUCH_IO_PORT_B) {
        switch (pin) {
            case 0: evaluateOutput((Output *)&(allLayers[layerID]->midi.oVelocity), event); break;
            case 1: evaluateOutput((Output *)&(allLayers[layerID]->midi.oNote), event); break;
            case 2: evaluateModul(&allLayers[cachedFocus.layer]->midi, event); break;
            case 3: evaluateOutput((Output *)&(allLayers[layerID]->midi.oAftertouch), event); break;
            case 4: evaluateOutput((Output *)&(allLayers[layerID]->midi.oPitchbend), event); break;
            case 5: evaluateFunctionButtons(0, event); break;
            case 6: evaluateFunctionButtons(1, event); break;
            case 7: evaluateFunctionButtons(2, event); break;
            case 8: break;
            case 9: evaluateActionButton(&actionHandler.buttonRight[0], event); break;
            case 10: evaluateActionButton(&actionHandler.buttonRight[1], event); break;
            case 11: evaluateActionButton(&actionHandler.buttonRight[2], event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_A) {
        switch (pin) {
            case 0: evaluateActionButton(&actionHandler.buttonLeft[2], event); break;
            case 1: evaluateActionButton(&actionHandler.buttonLeft[1], event); break;
            case 2: evaluateActionButton(&actionHandler.buttonLeft[0], event); break;
            case 3: evaluateActionHandle(&actionHandler.buttonHeader[0], event); break;
            case 4: evaluateActionHandle(&actionHandler.buttonHeader[1], event); break;
            case 5: evaluateActionHandle(&actionHandler.buttonHeader[2], event); break;
            case 6: evaluateActionHandle(&actionHandler.buttonHeader[3], event); break;
            case 7: break;
            case 8: break;
            case 9: globalSettings.setShift(event); break;
            case 10:
                if (event)
                    nextLayer();
                break;
            case 11: evaluateOutput((Output *)&(allLayers[layerID]->midi.oMod), event); break;
        }

        return;
    }
}

void PanelTouch::evaluateInput(Input *pInput, uint8_t event) {
    if (event) { // push Event
        if (activeOutput != nullptr) {
            if (tempOutputLocation.layer != 0xff) {
                newFocus = tempOutputLocation;
                tempOutputLocation.layer = 0xff;
            }

            clearOnReleaseOut = false; // we doing a patch remove clear flag

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
                    allLayers[layerID]->addPatchInOutById(activeOutput->idGlobal, pInput->idGlobal, 0);
                    ui.guiPanelFocus.scrollToLastEntry();
                }
            }
        }
        else {
            if (cachedFocus.type == FOCUSINPUT && ui.activePanel == &ui.guiPanelFocus) {
                if (pInput->layerId == cachedFocus.layer && pInput->idGlobal == allLayers[cachedFocus.layer]
                                                                                    ->modules[cachedFocus.modul]
                                                                                    ->inputs[cachedFocus.id]
                                                                                    ->idGlobal) { // already selected
                    clearOnReleaseIn = true;
                }
            }
            setInputFocus(pInput);
        }
    }
    else { // release Event -> clear active input
        if (pInput == activeInput) {
            activeInput = nullptr;
        }
        if (clearOnReleaseIn) {

            clearOnReleaseIn = false;
            newFocus.type = FOCUSMODULE;
        }
    }
}

void PanelTouch::evaluateOutput(Output *pOutput, uint8_t event) {
    if (event) { // push Event

        if (activeInput != nullptr) {
            clearOnReleaseIn = false; // we doing a patch remove clear flag

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
                    allLayers[layerID]->addPatchInOutById(pOutput->idGlobal, activeInput->idGlobal, 0);
                    ui.guiPanelFocus.scrollToLastEntry();
                }
            }
        }

        else {
            if (cachedFocus.type == FOCUSOUTPUT && ui.activePanel == &ui.guiPanelFocus) {
                if (pOutput->layerId == cachedFocus.layer && pOutput->idGlobal == allLayers[cachedFocus.layer]
                                                                                      ->modules[cachedFocus.modul]
                                                                                      ->outputs[cachedFocus.id]
                                                                                      ->idGlobal) { // already selected
                    clearOnReleaseOut = true;
                }
            }
            setOutputFocus(pOutput);
        }
    }

    else { // release Event -> clear active output
        if (pOutput == activeOutput) {

            activeOutput = nullptr;
            if (tempOutputLocation.layer != 0xff) {
                if (externalPatchAction == true) {
                    externalPatchAction = false;
                }
                else {
                    newFocus = tempOutputLocation;
                }
                tempOutputLocation.layer = 0xff;
            }
        }
        if (clearOnReleaseOut) {

            clearOnReleaseOut = false;

            newFocus.type = FOCUSMODULE;
        }
    }
}

void PanelTouch::evaluateModul(BaseModule *pModule, uint8_t event) {
    if (event) { // push Event
        activeOutput = nullptr;
        activeInput = nullptr;
        setModulFocus(pModule);
    }
}
void PanelTouch::evaluateActionButton(ButtonActionHandle *pButton, uint8_t event) {
    if (event) { // push Event
        pButton->state = PRESSED;
        if ((pButton->handle.functionPointer != nullptr) && (globalSettings.shift || pButton->unlock))
            pButton->handle.functionPointer();
    }
    else {
        pButton->state = RELEASED;
    }
}

void PanelTouch::evaluateActionHandle(actionHandle *pAction, uint8_t event) {
    if (event) { // push Event
        if (pAction->functionPointer != nullptr)
            pAction->functionPointer();
    }
}

void PanelTouch::evaluateSetting(Digital *pSwitch, uint8_t event) {
    if (event) {
        pSwitch->nextValueLoop();
    }
}

void PanelTouch::setInputFocus(Input *pInput) {
    if (activeInput == nullptr && activeOutput == nullptr) {
        activeInput = pInput;
        newFocus = {pInput->layerId, pInput->moduleId, pInput->id, FOCUSINPUT};
    }
}

void PanelTouch::setOutputFocus(Output *pOutput) {
    if (activeInput == nullptr && activeOutput == nullptr) {
        activeOutput = pOutput;
        tempOutputLocation = {pOutput->layerId, pOutput->moduleId, pOutput->id, FOCUSOUTPUT};
    }
}
void PanelTouch::forceSetOutputFocus(Output *pOutput) {
    activeInput = nullptr;
    activeOutput = pOutput;

    newFocus = {pOutput->layerId, pOutput->moduleId, pOutput->id, FOCUSOUTPUT};
}

void PanelTouch::forceSetInputFocus(Input *pInput) {
    activeOutput = nullptr;
    activeInput = pInput;

    newFocus = {pInput->layerId, pInput->moduleId, pInput->id, FOCUSINPUT};
}

void PanelTouch::setModulFocus(BaseModule *pModule) {
    newFocus = {pModule->layerId, pModule->id, 0, FOCUSMODULE};
}

void PanelTouch::externPatchInput(Input *pInput) {
    if (activeOutput != nullptr && pInput != nullptr) {
        allLayers[layerID]->addPatchInOutById(activeOutput->idGlobal, pInput->idGlobal, 0);
        externalPatchAction = true;
    }
}
void PanelTouch::externPatchRemove(Input *pInput) {
    if (activeOutput != nullptr && pInput != nullptr) {
        allLayers[layerID]->removePatchInOutById(activeOutput->idGlobal, pInput->idGlobal);
        externalPatchAction = true;
    }
}

void resetSystem() {
    allLayers[0]->resetLayer();
    allLayers[0]->clearPresetLocks();
    clearPotiState(0);
    allLayers[1]->resetLayer();
    allLayers[1]->clearPresetLocks();
    clearPotiState(1);

    liveData.resetLiveConfig();
}

#endif
