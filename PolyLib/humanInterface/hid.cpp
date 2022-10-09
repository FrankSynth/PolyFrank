#ifdef POLYCONTROL

#include "hid.hpp"
#include "gfx/gui.hpp"
#include "hardware/device.hpp"

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

// Touch
PanelTouch touch;

// number layer, number multiplex, number channels

typedef struct {
    std::function<void(uint16_t amount)> function;
    DataElement *data;
} potiFunctionStruct;

potiFunctionStruct potiFunctionPointer[2][4][12];
uint16_t panelADCStates[2][4][12];
float panelADCInterpolate[2][4][12];

void HIDConfig() {

    // Control
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    // Panels
    FlagHandler::Panel_0_RXTX_ISR = std::bind(processPanelPotis, adcA.adcData, 0);
    FlagHandler::Panel_1_RXTX_ISR = std::bind(processPanelPotis, adcB.adcData, 1);

    FlagHandler::Panel_0_EOC_ISR = std::bind(&MAX11128::fetchNewData, &adcA);
    FlagHandler::Panel_1_EOC_ISR = std::bind(&MAX11128::fetchNewData, &adcB);
    FlagHandler::Panel_nextChannel_ISR = std::bind(&TS3A5017D::nextChannel, &multiplexer);

    FlagHandler::Panel_0_Touch_ISR = std::bind(processPanelTouch, 0);
    FlagHandler::Panel_1_Touch_ISR = std::bind(processPanelTouch, 1);

    // register encoder

    // MENU
    encoders[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 0),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 0));

    encoders[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 1),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 1));

    encoders[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 2),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 2));

    encoders[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 3),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 3));
    // SCROLL
    encoders[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 4),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 4));
    // AMOUNT
    encoders[5].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 5),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 5));

    switches[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 0), nullptr);
    switches[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 1), nullptr);
    switches[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 2), nullptr);
    switches[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 3), nullptr);
    switches[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 4), nullptr);
    switches[5].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 5), nullptr);

    // register transmit functions for the led Driver
    FlagHandler::ledDriverA_ISR[0] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[0][0]);
    FlagHandler::ledDriverA_ISR[1] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[0][1]);

    FlagHandler::ledDriverB_ISR[0] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[1][0]);
    FlagHandler::ledDriverB_ISR[1] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[1][1]);

    FlagHandler::ledDriverControl_ISR = std::bind(&IS31FL3205::sendPWMData, &ledDriverControl);

    potiMapping();
    LEDMappingInit();

    // activate FlagHandling for HID ICs
    FlagHandler::HID_Initialized = true;
}

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
        for (unsigned int x = 0; x < touchPanelA.size(); x++) {
            uint16_t touchState = touchPanelA[x].readTouchStatus();

            touch.eventLayer(layerID, touchState, x);
            if (HAL_GPIO_ReadPin(Panel_1_INT_GPIO_Port,
                                 Panel_1_INT_Pin)) { // interrupt cleared?
                FlagHandler::Panel_0_Touch_Interrupt = false;
                return;
            }
        }
    }
    else if (layerID == 1) {

        for (unsigned int x = 0; x < touchPanelB.size(); x++) {
            uint16_t touchState = touchPanelB[x].readTouchStatus();

            touch.eventLayer(layerID, touchState, x);
            if (HAL_GPIO_ReadPin(Panel_2_INT_GPIO_Port,
                                 Panel_2_INT_Pin)) { // interrupt cleared?
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
void processPanelPotis(uint32_t *adcData, uint32_t layer) {
    static int16_t octaveSwitchLastScan[2][2] = {{0, 0}, {0, 0}}; // threshold for jitter reduction  [layer][switch]
    static uint32_t x = 0;

    SCB_InvalidateDCache_by_Addr(adcData, 48); // clear dcache

    uint32_t multiplex = (multiplexer.currentChannel + 3) % 4;

    for (uint32_t channel = 0; channel < 12; channel++) { // for all Channels
        uint16_t potiData = ((adcData[channel] >> 1) & 0xFFF);
        if (((multiplex == 0) & (channel == 1)) || ((multiplex == 0) & (channel == 2))) { // octave switches

            // this filters out the switching artefact of the octave switches
            //  check data stability
            if (std::abs(octaveSwitchLastScan[layer][channel - 1] - potiData) < 2) { // test difference
                x++;
            }
            else {
                x = 0;
            }

            // value stable -> apply new octave switch value
            if (x > 40) {
                if (std::abs(panelADCStates[layer][multiplex][channel] - potiData) >= 50) {
                    panelADCStates[layer][multiplex][channel] = potiData;

                    if (potiFunctionPointer[layer][multiplex][channel].function != nullptr) { // call function
                        potiFunctionPointer[layer][multiplex][channel].function(
                            panelADCStates[layer][multiplex][channel]);
                    }
                }
            }

            octaveSwitchLastScan[layer][channel - 1] = potiData; // store new data
        }
        else {
            // interpolation

            panelADCInterpolate[layer][multiplex][channel] = fast_lerp_f32(
                panelADCInterpolate[layer][multiplex][channel], (float)((adcData[channel] >> 1) & 0xFFF), 0.25);

            uint16_t difference = std::abs(panelADCStates[layer][multiplex][channel] - potiData);
            uint16_t shift = 1;
            // check if value is loaded from preset
            if (potiFunctionPointer[layer][multiplex][channel].data->presetLock)
                shift = 3;

            if (difference >> shift) { // active quickview only on stronger value changes
                if (potiFunctionPointer[layer][multiplex][channel].function != nullptr) {

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

void potiMapping() {
    for (uint32_t i = 0; i < 2; i++) { // register potis for both layer
        if (allLayers[i]->layerState.value == 1) {

            potiFunctionPointer[i][0][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aParSer), std::placeholders::_1),
                &allLayers[i]->steiner.aParSer};
            potiFunctionPointer[i][1][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aSpread), std::placeholders::_1),
                &allLayers[i]->feel.aSpread};
            potiFunctionPointer[i][2][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aDetune), std::placeholders::_1),
                &allLayers[i]->feel.aDetune};
            potiFunctionPointer[i][3][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aGlide), std::placeholders::_1),
                &allLayers[i]->feel.aGlide};

            potiFunctionPointer[i][0][1] = {
                std::bind(&Digital::setValueRange, &(allLayers[i]->oscA.dOctave), std::placeholders::_1, 585, 4083),
                &allLayers[i]->oscA.dOctave};
            potiFunctionPointer[i][1][1] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aEffect), std::placeholders::_1),
                &allLayers[i]->oscA.aEffect};
            potiFunctionPointer[i][2][1] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMorph), std::placeholders::_1),
                &allLayers[i]->oscA.aMorph};
            potiFunctionPointer[i][3][1] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMasterTune), std::placeholders::_1),
                &allLayers[i]->oscA.aMasterTune};

            potiFunctionPointer[i][0][2] = {
                std::bind(&Digital::setValueRange, &(allLayers[i]->oscB.dOctave), std::placeholders::_1, 585, 4083),
                &allLayers[i]->oscB.dOctave};
            potiFunctionPointer[i][1][2] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aEffect), std::placeholders::_1),
                &allLayers[i]->oscB.aEffect};
            potiFunctionPointer[i][2][2] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aMorph), std::placeholders::_1),
                &allLayers[i]->oscB.aMorph};
            potiFunctionPointer[i][3][2] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aTuning), std::placeholders::_1),
                &allLayers[i]->oscB.aTuning};

            potiFunctionPointer[i][0][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aNOISELevel), std::placeholders::_1),
                &allLayers[i]->mixer.aNOISELevel};
            potiFunctionPointer[i][1][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aSUBLevel), std::placeholders::_1),
                &allLayers[i]->mixer.aSUBLevel};
            potiFunctionPointer[i][2][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCBLevel), std::placeholders::_1),
                &allLayers[i]->mixer.aOSCBLevel};
            potiFunctionPointer[i][3][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCALevel), std::placeholders::_1),
                &allLayers[i]->mixer.aOSCALevel};

            potiFunctionPointer[i][1][4] = {
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aLevel), std::placeholders::_1),
                &allLayers[i]->ladder.aLevel};
            potiFunctionPointer[i][2][4] = {
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aResonance), std::placeholders::_1),
                &allLayers[i]->ladder.aResonance};
            potiFunctionPointer[i][3][4] = {
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aCutoff), std::placeholders::_1),
                &allLayers[i]->ladder.aCutoff};

            potiFunctionPointer[i][1][5] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1),
                &allLayers[i]->steiner.aLevel};
            potiFunctionPointer[i][2][5] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1),
                &allLayers[i]->steiner.aResonance};
            potiFunctionPointer[i][3][5] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1),
                &allLayers[i]->steiner.aCutoff};

            potiFunctionPointer[i][1][6] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aAmount), std::placeholders::_1),
                &allLayers[i]->lfoA.aAmount};
            potiFunctionPointer[i][2][6] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aShape), std::placeholders::_1),
                &allLayers[i]->lfoA.aShape};
            potiFunctionPointer[i][3][6] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aFreq), std::placeholders::_1),
                &allLayers[i]->lfoA.aFreq};

            potiFunctionPointer[i][0][7] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aDelay), std::placeholders::_1),
                &allLayers[i]->envF.aDelay};
            potiFunctionPointer[i][1][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aAmount), std::placeholders::_1),
                &allLayers[i]->lfoB.aAmount};
            potiFunctionPointer[i][2][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aShape), std::placeholders::_1),
                &allLayers[i]->lfoB.aShape};
            potiFunctionPointer[i][3][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aFreq), std::placeholders::_1),
                &allLayers[i]->lfoB.aFreq};

            potiFunctionPointer[i][0][8] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aAttack), std::placeholders::_1),
                &allLayers[i]->envF.aAttack};
            potiFunctionPointer[i][1][8] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aDecay), std::placeholders::_1),
                &allLayers[i]->envF.aDecay};
            potiFunctionPointer[i][2][8] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aSustain), std::placeholders::_1),
                &allLayers[i]->envF.aSustain};
            potiFunctionPointer[i][3][8] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aRelease), std::placeholders::_1),
                &allLayers[i]->envF.aRelease};

            potiFunctionPointer[i][0][9] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aSustain), std::placeholders::_1),
                &allLayers[i]->envA.aSustain};
            potiFunctionPointer[i][1][9] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aRelease), std::placeholders::_1),
                &allLayers[i]->envA.aRelease};
            potiFunctionPointer[i][2][9] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aAmount), std::placeholders::_1),
                &allLayers[i]->envA.aAmount};
            potiFunctionPointer[i][3][9] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aAmount), std::placeholders::_1),
                &allLayers[i]->envF.aAmount};

            potiFunctionPointer[i][0][10] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aDelay), std::placeholders::_1),
                &allLayers[i]->envA.aDelay};
            potiFunctionPointer[i][1][10] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aAttack), std::placeholders::_1),
                &allLayers[i]->envA.aAttack};
            potiFunctionPointer[i][2][10] = {
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aDecay), std::placeholders::_1),
                &allLayers[i]->envA.aDecay};
            potiFunctionPointer[i][3][10] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aMaster), std::placeholders::_1),
                &allLayers[i]->out.aMaster};

            potiFunctionPointer[i][0][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aDistort), std::placeholders::_1),
                &allLayers[i]->out.aDistort};
            potiFunctionPointer[i][1][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aPanSpread), std::placeholders::_1),
                &allLayers[i]->out.aPanSpread};
            potiFunctionPointer[i][2][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aPan), std::placeholders::_1),
                &allLayers[i]->out.aPan};
            potiFunctionPointer[i][3][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aVCA), std::placeholders::_1),
                &allLayers[i]->out.aVCA};
        }
    }
}

//////////// LED ///////////
void LEDRender() {

    uint16_t breathing =
        uint8_t((fast_sin_f32(millis() / 1000.f) + 1) / 4.f * (float)LEDBRIGHTNESS_MEDIUM / 2) + LEDBRIGHTNESS_MEDIUM;

    for (uint32_t i = 0; i < 2; i++) { // for both Layer
        // clear Data

        for (uint32_t x = 0; x < 2; x++) {
            ledDriver[i][x].clearPWMData();
        }

        if (allLayers[i]->layerState.value) {
            // LEDModuleRenderbuffer(i);
            if (currentFocus.modul < allLayers[i]->modules.size()) {

                if (currentFocus.type == FOCUSOUTPUT) {

                    if (currentFocus.id < allLayers[i]->modules[currentFocus.modul]->getOutputs().size()) {

                        Output *output = allLayers[i]->modules[currentFocus.modul]->getOutputs()[currentFocus.id];
                        LEDOutput(output, LEDBRIGHTNESS_MAX);

                        for (uint32_t p = 0; p < output->getPatchesInOut().size(); p++) {
                            Input *target = output->getPatchesInOut()[p]->targetIn;
                            LEDInput(target, breathing);
                        }
                    }
                }
                else if (currentFocus.type == FOCUSINPUT) {
                    if (currentFocus.id < allLayers[i]->modules[currentFocus.modul]->getInputs().size()) {

                        Input *input = allLayers[i]->modules[currentFocus.modul]->getInputs()[currentFocus.id];
                        LEDInput(input, LEDBRIGHTNESS_MAX);

                        for (uint32_t p = 0; p < input->getPatchesInOut().size(); p++) {
                            Output *source = input->getPatchesInOut()[p]->sourceOut;
                            LEDOutput(source, breathing);
                        }
                    }
                }
            }

            // Module and Switch LEDs
            LEDModuleSwitch(i);
            LEDModuleOUT(i);
        }
    }
    // TODO ENABLE NEW
    //  if (FlagHandler::ledDriverATransmit == DRIVER_IDLE) {
    //      FlagHandler::ledDriverA_Interrupt = true;
    //  }
    //  else {
    //      println("ERROR | LEDDriverA Transmit not Complete");
    //  }

    // if (FlagHandler::ledDriverBTransmit == DRIVER_IDLE) {
    //     FlagHandler::ledDriverB_Interrupt = true;
    // }
    // else {
    //     println("ERROR | LEDDriverB Transmit not Complete");
    // }

    // if (FlagHandler::ledDriverControlTransmit == DRIVER_IDLE) {
    //     FlagHandler::ledDriverControl_Interrupt = true;
    // }
    // else {
    //     println("ERROR | LEDDriverControl Transmit not Complete");
    // }
}

void LEDMappingInit() {
    for (uint8_t i = 0; i < 2; i++) {

        println("INFO | LED Mapping ", i);

        /////// Inputs

        // Steiner
        allLayers[i]->steiner.iCutoff.LEDPinID = 0;
        allLayers[i]->steiner.iCutoff.LEDPortID = 0;

        allLayers[i]->steiner.iResonance.LEDPinID = 35;
        allLayers[i]->steiner.iResonance.LEDPortID = 0;

        allLayers[i]->steiner.iLevel.LEDPinID = 34;
        allLayers[i]->steiner.iLevel.LEDPortID = 0;

        // Ladder
        allLayers[i]->ladder.iCutoff.LEDPinID = 31;
        allLayers[i]->ladder.iCutoff.LEDPortID = 0;

        allLayers[i]->ladder.iResonance.LEDPinID = 32;
        allLayers[i]->ladder.iResonance.LEDPortID = 0;

        allLayers[i]->ladder.iLevel.LEDPinID = 33;
        allLayers[i]->ladder.iLevel.LEDPortID = 0;

        // Feel
        allLayers[i]->feel.iGlide.LEDPinID = 11;
        allLayers[i]->feel.iGlide.LEDPortID = 1;

        allLayers[i]->feel.iDetune.LEDPinID = 12;
        allLayers[i]->feel.iDetune.LEDPortID = 1;

        // OSC A
        allLayers[i]->oscA.iFM.LEDPinID = 16;
        allLayers[i]->oscA.iFM.LEDPortID = 0;

        allLayers[i]->oscA.iMorph.LEDPinID = 15;
        allLayers[i]->oscA.iMorph.LEDPortID = 0;

        allLayers[i]->oscA.iEffect.LEDPinID = 14;
        allLayers[i]->oscA.iEffect.LEDPortID = 0;

        // OSC B
        allLayers[i]->oscB.iFM.LEDPinID = 12;
        allLayers[i]->oscB.iFM.LEDPortID = 0;

        allLayers[i]->oscB.iMorph.LEDPinID = 11;
        allLayers[i]->oscB.iMorph.LEDPortID = 0;

        allLayers[i]->oscB.iEffect.LEDPinID = 10;
        allLayers[i]->oscB.iEffect.LEDPortID = 0;

        // ENV
        allLayers[i]->envA.iAmount.LEDPinID = 29;
        allLayers[i]->envA.iAmount.LEDPortID = 1;

        allLayers[i]->envF.iAmount.LEDPinID = 31;
        allLayers[i]->envF.iAmount.LEDPortID = 1;

        // MIX  //check renderbuffer led output
        allLayers[i]->mixer.oscALevelSteiner.LEDPinID = 8;
        allLayers[i]->mixer.oscALevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.oscBLevelSteiner.LEDPinID = 6;
        allLayers[i]->mixer.oscBLevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.subLevelSteiner.LEDPinID = 4;
        allLayers[i]->mixer.subLevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.noiseLevelSteiner.LEDPinID = 2;
        allLayers[i]->mixer.noiseLevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.oscALevelLadder.LEDPinID = 7;
        allLayers[i]->mixer.oscALevelLadder.LEDPortID = 0;

        allLayers[i]->mixer.oscBLevelLadder.LEDPinID = 5;
        allLayers[i]->mixer.oscBLevelLadder.LEDPortID = 0;

        allLayers[i]->mixer.subLevelLadder.LEDPinID = 3;
        allLayers[i]->mixer.subLevelLadder.LEDPortID = 0;

        allLayers[i]->mixer.noiseLevelLadder.LEDPinID = 1;
        allLayers[i]->mixer.noiseLevelLadder.LEDPortID = 0;

        /////// Outputs
        allLayers[i]->lfoA.out.LEDPinID = 24;
        allLayers[i]->lfoA.out.LEDPortID = 0;

        allLayers[i]->lfoB.out.LEDPinID = 22;
        allLayers[i]->lfoB.out.LEDPortID = 1;

        allLayers[i]->envF.out.LEDPinID = 30;
        allLayers[i]->envF.out.LEDPortID = 1;

        allLayers[i]->envA.out.LEDPinID = 28;
        allLayers[i]->envA.out.LEDPortID = 1;

        allLayers[i]->feel.oSpread.LEDPinID = 13;
        allLayers[i]->feel.oSpread.LEDPortID = 1;

        /////// Module Output
        allLayers[i]->lfoA.LEDPinID = 30;
        allLayers[i]->lfoA.LEDPortID = 0;

        allLayers[i]->lfoB.LEDPinID = 22;
        allLayers[i]->lfoB.LEDPortID = 0;

        allLayers[i]->envF.LEDPinID = 19;
        allLayers[i]->envF.LEDPortID = 0;

        allLayers[i]->envA.LEDPinID = 25;
        allLayers[i]->envA.LEDPortID = 1;

        /////// Configs
        allLayers[i]->sub.dOctaveSwitch.configureNumberLEDs(1);
        allLayers[i]->sub.dOctaveSwitch.LEDPinID[0] = 13;
        allLayers[i]->sub.dOctaveSwitch.LEDPortID[0] = 0;

        allLayers[i]->oscB.dSync.configureNumberLEDs(1);
        allLayers[i]->oscB.dSync.LEDPinID[0] = 9;
        allLayers[i]->oscB.dSync.LEDPortID[0] = 0;

        allLayers[i]->mixer.dNOISEDestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPinID[0] = 2;
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPinID[1] = 1;
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPortID[1] = 0;

        allLayers[i]->mixer.dOSCADestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dOSCADestSwitch.LEDPinID[0] = 8;
        allLayers[i]->mixer.dOSCADestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dOSCADestSwitch.LEDPinID[1] = 7;
        allLayers[i]->mixer.dOSCADestSwitch.LEDPortID[1] = 0;

        allLayers[i]->mixer.dOSCBDestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPinID[0] = 6;
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPinID[1] = 5;
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPortID[1] = 0;

        allLayers[i]->mixer.dSUBDestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dSUBDestSwitch.LEDPinID[0] = 4;
        allLayers[i]->mixer.dSUBDestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dSUBDestSwitch.LEDPinID[1] = 3;
        allLayers[i]->mixer.dSUBDestSwitch.LEDPortID[1] = 0;

        allLayers[i]->lfoA.dAlignLFOs.configureNumberLEDs(1);
        allLayers[i]->lfoA.dAlignLFOs.LEDPinID[0] = 25;
        allLayers[i]->lfoA.dAlignLFOs.LEDPortID[0] = 0;

        allLayers[i]->lfoA.dGateTrigger.configureNumberLEDs(1);
        allLayers[i]->lfoA.dGateTrigger.LEDPinID[0] = 26;
        allLayers[i]->lfoA.dGateTrigger.LEDPortID[0] = 0;

        allLayers[i]->lfoB.dAlignLFOs.configureNumberLEDs(1);
        allLayers[i]->lfoB.dAlignLFOs.LEDPinID[0] = 24;
        allLayers[i]->lfoB.dAlignLFOs.LEDPortID[0] = 1;

        allLayers[i]->lfoB.dGateTrigger.configureNumberLEDs(1);
        allLayers[i]->lfoB.dGateTrigger.LEDPinID[0] = 23;
        allLayers[i]->lfoB.dGateTrigger.LEDPortID[0] = 1;

        allLayers[i]->steiner.dMode.configureNumberLEDs(4);
        allLayers[i]->steiner.dMode.LEDPinID[0] = 18;
        allLayers[i]->steiner.dMode.LEDPortID[0] = 1;
        allLayers[i]->steiner.dMode.LEDPinID[1] = 19;
        allLayers[i]->steiner.dMode.LEDPortID[1] = 1;
        allLayers[i]->steiner.dMode.LEDPinID[2] = 20;
        allLayers[i]->steiner.dMode.LEDPortID[2] = 1;
        allLayers[i]->steiner.dMode.LEDPinID[3] = 21;
        allLayers[i]->steiner.dMode.LEDPortID[3] = 1;

        allLayers[i]->ladder.dSlope.configureNumberLEDs(4);
        allLayers[i]->ladder.dSlope.LEDPinID[0] = 14;
        allLayers[i]->ladder.dSlope.LEDPortID[0] = 1;
        allLayers[i]->ladder.dSlope.LEDPinID[1] = 15;
        allLayers[i]->ladder.dSlope.LEDPortID[1] = 1;
        allLayers[i]->ladder.dSlope.LEDPinID[2] = 16;
        allLayers[i]->ladder.dSlope.LEDPortID[2] = 1;
        allLayers[i]->ladder.dSlope.LEDPinID[3] = 17;
        allLayers[i]->ladder.dSlope.LEDPortID[3] = 1;

        //////MIDI

        allLayers[i]->midi.oNote.LEDPinID = 0;
        allLayers[i]->midi.oNote.LEDPortID = 2;

        allLayers[i]->midi.oVelocity.LEDPinID = 1;
        allLayers[i]->midi.oVelocity.LEDPortID = 2;

        allLayers[i]->midi.oMod.LEDPinID = 4;
        allLayers[i]->midi.oMod.LEDPortID = 2;

        allLayers[i]->midi.oPitchbend.LEDPinID = 3;
        allLayers[i]->midi.oPitchbend.LEDPortID = 2;

        allLayers[i]->midi.oAftertouch.LEDPinID = 2;
        allLayers[i]->midi.oAftertouch.LEDPortID = 2;
    }
}

void LEDOutput(Output *output, uint16_t brigthness) {
    if (output->LEDPortID == 2) { // MIDI MODULE Control
        ledDriverControl.pwmData[output->LEDPinID] = brigthness;
    }
    else {

        ledDriver[output->layerId][output->LEDPortID].pwmData[output->LEDPinID] = brigthness;
    }
}

void LEDInput(Input *input, uint16_t brigthness) {
    ledDriver[input->layerId][input->LEDPortID].pwmData[input->LEDPinID] = brigthness;
}

// MODULE LED OUTPUT
void LEDModule(BaseModule *module) { // TODO Check ranges?

    // println(module->layerId, "  ", module->LEDPortID, " ", module->LEDPinID, " ");
    //
    ledDriver[module->layerId][module->LEDPortID].pwmData[module->LEDPinID] =
        module->getOutputs()[0]->currentSample[0] + 1 * LEDBRIGHTNESS_MAX / 2;
}

void LEDRenderbuffer(RenderBuffer *rBuffer) { // TODO Check ranges?
    ledDriver[rBuffer->layerId][rBuffer->LEDPortID].pwmData[rBuffer->LEDPinID] = rBuffer->currentSample[0];
}

void  LEDModuleOUT(uint32_t layerID) {
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

void LEDModuleSwitch(uint32_t layerID) {

    LEDSingleSetting(&allLayers[layerID]->sub.dOctaveSwitch);
    LEDSingleSetting(&allLayers[layerID]->oscB.dSync);

    LEDDualSetting(&allLayers[layerID]->mixer.dOSCADestSwitch);
    LEDDualSetting(&allLayers[layerID]->mixer.dOSCBDestSwitch);
    LEDDualSetting(&allLayers[layerID]->mixer.dSUBDestSwitch);
    LEDDualSetting(&allLayers[layerID]->mixer.dNOISEDestSwitch);

    LEDQuadSetting(&allLayers[layerID]->steiner.dMode);
    LEDQuadSetting(&allLayers[layerID]->ladder.dSlope);
}

void LEDDigital(Digital *digital, uint8_t id, uint16_t value) {
    if (digital != nullptr) {
        if (id < digital->LEDPortID.size() && id < digital->LEDPinID.size()) {
            uint32_t pin = digital->LEDPinID[id];
            uint32_t port = digital->LEDPortID[id];
            ledDriver[digital->layerId][port].pwmData[pin] = value;
        }
    }
}

void LEDQuadSetting(Digital *digital) {
    if (digital == nullptr)
        return;
    switch (digital->valueMapped) {
        case 0: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_MAX);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 2, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 3, LEDBRIGHTNESS_OFF);
            break;
        }
        case 1: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_MAX);
            LEDDigital(digital, 2, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 3, LEDBRIGHTNESS_OFF);

            break;
        }
        case 2: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 2, LEDBRIGHTNESS_MAX);
            LEDDigital(digital, 3, LEDBRIGHTNESS_OFF);
            break;
        }
        case 3: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 2, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 3, LEDBRIGHTNESS_MAX);
            break;
        }
        default: break;
    }
}

void LEDDualSetting(Digital *digital) {
    if (digital == nullptr)
        return;

    switch (digital->valueMapped) {
        case 0: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_MAX);
            LEDDigital(digital, 1, LEDBRIGHTNESS_OFF);
            break;
        }
        case 1: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_OFF);
            LEDDigital(digital, 1, LEDBRIGHTNESS_MAX);
            break;
        }
        case 2: {
            LEDDigital(digital, 0, LEDBRIGHTNESS_MAX);
            LEDDigital(digital, 1, LEDBRIGHTNESS_MAX);
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
    // LEDDigital(digital, 0, digital->valueMapped ? LEDBRIGHTNESS_MAX : LEDBRIGHTNESS_OFF);
}

//////////// PanelTouch ///////////

void PanelTouch::eventLayer(uint8_t layerID, uint16_t touchState, uint8_t port) {
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

    if (port == TOUCH_IO_PORT_A) {
        switch (pin) {
            case 0: evaluateSetting((Digital *)&(allLayers[layerID]->oscB.dSync), event); break;
            case 1: evaluateInput((Input *)&(allLayers[layerID]->oscB.iEffect), event); break;
            case 2: evaluateInput((Input *)&(allLayers[layerID]->oscB.iMorph), event); break;
            case 3: evaluateInput((Input *)&(allLayers[layerID]->oscB.iFM), event); break;
            case 4: evaluateModul((BaseModule *)&(allLayers[layerID]->oscB), event); break;
            case 5: evaluateSetting((Digital *)&(allLayers[layerID]->sub.dOctaveSwitch), event); break;
            case 6: evaluateInput((Input *)&(allLayers[layerID]->oscA.iEffect), event); break;
            case 7: evaluateInput((Input *)&(allLayers[layerID]->oscA.iMorph), event); break;
            case 8: evaluateInput((Input *)&(allLayers[layerID]->oscA.iFM), event); break;
            case 9: evaluateInput((Input *)&(allLayers[layerID]->feel.iGlide), event); break;
            case 10: evaluateModul((BaseModule *)&(allLayers[layerID]->feel), event); break;
            case 11: evaluateModul((BaseModule *)&(allLayers[layerID]->oscA), event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_B) {
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

        return;
    }
    if (port == TOUCH_IO_PORT_C) {
        switch (pin) {
            case 0: evaluateSetting((Digital *)&(allLayers[layerID]->lfoB.dAlignLFOs), event); break;
            case 1: evaluateSetting((Digital *)&(allLayers[layerID]->lfoB.dGateTrigger), event); break;
            case 2: evaluateInput((Input *)&(allLayers[layerID]->lfoB.iAmount), event); break;
            case 3: evaluateInput((Input *)&(allLayers[layerID]->lfoB.iShape), event); break;
            case 4: evaluateInput((Input *)&(allLayers[layerID]->lfoB.iFreq), event); break;
            case 5: evaluateModul((BaseModule *)&(allLayers[layerID]->lfoA), event); break;
            case 6: evaluateSetting((Digital *)&(allLayers[layerID]->lfoA.dAlignLFOs), event); break;
            case 7: evaluateSetting((Digital *)&(allLayers[layerID]->lfoA.dGateTrigger), event); break;
            case 8: evaluateOutput((Output *)&(allLayers[layerID]->lfoA.out), event); break;
            case 9: break;
            case 10: break;
            case 11: evaluateOutput((Output *)&(allLayers[layerID]->lfoB.out), event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_D) {
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
            case 10: evaluateInput((Input *)&(allLayers[layerID]->ladder.iLevel), event); break;
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

    // TODO DELETE
    if (port == TOUCH_IO_PORT_A) {
        switch (pin) {
            case 0: break;
            case 1: break;
            case 2: evaluateActionButton(&actionHandler.buttonLeft[2], event); break;
            case 3: evaluateActionButton(&actionHandler.buttonLeft[1], event); break;
            case 4: evaluateActionButton(&actionHandler.buttonLeft[0], event); break;
            case 5: evaluateActionHandle(&actionHandler.buttonHeader[0], event); break;
            case 6: evaluateActionHandle(&actionHandler.buttonHeader[1], event); break;
            case 7: evaluateActionHandle(&actionHandler.buttonHeader[2], event); break;
            case 8: evaluateActionHandle(&actionHandler.buttonHeader[3], event); break;
            case 9: evaluateActionButton(&actionHandler.buttonRight[0], event); break;
            case 10: evaluateActionButton(&actionHandler.buttonRight[1], event); break;
            case 11: evaluateActionButton(&actionHandler.buttonRight[2], event); break;
        }

        return;
    }
    if (port == TOUCH_IO_PORT_B) {
        switch (pin) {
            case 0: globalSettings.setShift(event);
            case 1: break;
            case 2: evaluateModul(&allLayers[currentFocus.layer]->oscA, event); break;
            case 3: break;
            case 4: evaluateModul(&allLayers[currentFocus.layer]->oscB, event); break;
            case 6: evaluateModul(&allLayers[currentFocus.layer]->mixer, event); break;
            case 7: break;
            case 8: evaluateModul(&allLayers[currentFocus.layer]->envA, event); break;
            case 9: break;
            case 10:
                if (event)
                    nextLayer();
                break;
            case 11: break;
        }
    }
    return;

    // // TODO ENABLE NEW
    // if (port == TOUCH_IO_PORT_A) {
    //     switch (pin) {
    //         case 0: evaluateOutput((Output *)&(allLayers[layerID]->midi.oVelocity), event); break;
    //         case 1: evaluateOutput((Output *)&(allLayers[layerID]->midi.oNote), event); break;
    //         case 2: evaluateModul(&allLayers[currentFocus.layer]->midi, event); break;
    //         case 3: evaluateOutput((Output *)&(allLayers[layerID]->midi.oAftertouch), event); break;
    //         case 4: evaluateOutput((Output *)&(allLayers[layerID]->midi.oPitchbend), event); break;
    //         case 5: break; // TODO F1
    //         case 6: break; // TODO F2
    //         case 7: break; // TODO F3
    //         case 8: break;
    //         case 9: evaluateActionButton(&actionHandler.buttonRight[2], event); break;
    //         case 10: evaluateActionButton(&actionHandler.buttonRight[1], event); break;
    //         case 11: evaluateActionButton(&actionHandler.buttonRight[0], event); break;
    //     }

    //     return;
    // }
    // if (port == TOUCH_IO_PORT_B) {
    //     switch (pin) {
    //         case 0: evaluateActionButton(&actionHandler.buttonLeft[0], event); break;
    //         case 1: evaluateActionButton(&actionHandler.buttonLeft[1], event); break;
    //         case 2: evaluateActionButton(&actionHandler.buttonLeft[2], event); break;
    //         case 3: evaluateActionHandle(&actionHandler.buttonHeader[0], event); break;
    //         case 4: evaluateActionHandle(&actionHandler.buttonHeader[1], event); break;
    //         case 5: evaluateActionHandle(&actionHandler.buttonHeader[2], event); break;
    //         case 6: evaluateActionHandle(&actionHandler.buttonHeader[3], event); break;
    //         case 7: break;
    //         case 8: break;
    //         case 9: globalSettings.setShift(event); break;
    //         case 10:
    //             if (event)
    //                 nextLayer();
    //             break;
    //         case 11: evaluateOutput((Output *)&(allLayers[layerID]->midi.oMod), event); break;
    //     }

    //     return;
    // }
}

void PanelTouch::evaluateInput(Input *pInput, uint8_t event) {
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
                    allLayers[layerID]->addPatchInOutById(activeOutput->idGlobal, pInput->idGlobal, 0);
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
void PanelTouch::evaluateOutput(Output *pOutput, uint8_t event) {

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
                    allLayers[layerID]->addPatchInOutById(pOutput->idGlobal, activeInput->idGlobal, 0);
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

        newFocus = {pOutput->layerId, pOutput->moduleId, pOutput->id, FOCUSOUTPUT};
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

#endif
