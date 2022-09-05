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
extern std::vector<IS31FL3216> ledDriverA;
extern std::vector<IS31FL3216> ledDriverB;

extern TS3A5017D multiplexer;

// Encoder
extern rotary encoders[NUMBERENCODERS];
extern tactileSwitch switches[NUMBERENCODERS];

// Touch
PanelTouch touch;

// number layer, number multiplex, number channels
std::function<void(uint16_t amount)> potiFunctionPointer[2][4][16];
int16_t panelADCStates[2][4][16];

void HIDConfig() {

    // Control
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    // Panels
    FlagHandler::Panel_EOC_ISR = std::bind(processPanelPotis);
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

    potiMapping();
    patchLEDMappingInit();

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
void processPanelPotis() {

    static int16_t treshold = 2;                                  // threshold for jitter reduction
    static int16_t octaveSwitchLastScan[2][2] = {{0, 0}, {0, 0}}; // threshold for jitter reduction  [layer][switch]
    static int x = 0;
    // store for current sample data for the 4x16 Multiplexed ADC Values

    uint16_t multiplex = multiplexer.currentChannel;

    multiplexer.nextChannel();

    if (allLayers[0]->layerState.value == 1) {                // check layer active state
        adcA.fetchNewData();                                  // fetch ADC data
        for (uint32_t channel = 0; channel < 16; channel++) { // for all Channels

            if ((multiplex == 0 & channel == 1) || (multiplex == 0 & channel == 2)) { // octave switches

                // this filters out the switching artefact of the octave switches
                //  check data stability
                if (std::abs(octaveSwitchLastScan[0][channel - 1] - (int16_t)((adcA.adcData[channel] >> 1) & 0xFFF)) <
                    2) { // test difference

                    if (x != 40)
                        x++;
                }
                else {
                    x = 0;
                }

                // value stable -> apply new octave switch value
                if (x == 40) {

                    if (std::abs((panelADCStates[0][multiplex][channel] -
                                  (int16_t)((adcA.adcData[channel] >> 1) & 0xFFF))) >= 50) {

                        panelADCStates[0][multiplex][channel] = (int16_t)((adcA.adcData[channel] >> 1) & 0xFFF);

                        // println("setValue : ", panelADCStates[0][multiplex][channel]);

                        if (potiFunctionPointer[0][multiplex][channel] != nullptr) { // call function
                            potiFunctionPointer[0][multiplex][channel](panelADCStates[0][multiplex][channel]);
                        }
                    }
                }

                octaveSwitchLastScan[0][channel - 1] = (int16_t)((adcA.adcData[channel] >> 1) & 0xFFF); // store new
                                                                                                        // data
            }

            else if (std::abs((panelADCStates[0][multiplex][channel] -
                               (int16_t)((adcA.adcData[channel] >> 1) & 0xFFF))) >= treshold) {

                panelADCStates[0][multiplex][channel] = (int16_t)((adcA.adcData[channel] >> 1) & 0xFFF);

                if (potiFunctionPointer[0][multiplex][channel] != nullptr) { // call function
                    potiFunctionPointer[0][multiplex][channel](panelADCStates[0][multiplex][channel]);
                }
            }
        }
    }
    if (allLayers[1]->layerState.value == 1) {                // check layer active state
        adcB.fetchNewData();                                  // fetch ADC data
        for (uint32_t channel = 0; channel < 16; channel++) { // for all Channels

            if ((multiplex == 0 & channel == 1) || (multiplex == 0 & channel == 2)) { // octave switches

                // this filters out the switching artefact of the octave switches
                //  check data stability
                if (std::abs(octaveSwitchLastScan[1][channel - 1] - (int16_t)((adcB.adcData[channel] >> 1) & 0xFFF)) <
                    2) { // test difference

                    if (x != 40)
                        x++;
                }
                else {
                    x = 0;
                }

                // value stable -> apply new octave switch value
                if (x == 40) {

                    if (std::abs((panelADCStates[1][multiplex][channel] -
                                  (int16_t)((adcB.adcData[channel] >> 1) & 0xFFF))) >= 50) {

                        panelADCStates[1][multiplex][channel] = (int16_t)((adcB.adcData[channel] >> 1) & 0xFFF);

                        // println("setValue : ", panelADCStates[1][multiplex][channel]);

                        if (potiFunctionPointer[1][multiplex][channel] != nullptr) { // call function
                            potiFunctionPointer[1][multiplex][channel](panelADCStates[1][multiplex][channel]);
                        }
                    }
                }

                octaveSwitchLastScan[1][channel - 1] = (int16_t)((adcB.adcData[channel] >> 1) & 0xFFF); // store new
                                                                                                        // data
            }

            else if (std::abs((panelADCStates[1][multiplex][channel] -
                               (int16_t)((adcB.adcData[channel] >> 1) & 0xFFF))) >= treshold) {

                panelADCStates[1][multiplex][channel] = (int16_t)((adcB.adcData[channel] >> 1) & 0xFFF);

                if (potiFunctionPointer[1][multiplex][channel] != nullptr) { // call function
                    potiFunctionPointer[1][multiplex][channel](panelADCStates[1][multiplex][channel]);
                }
            }
        }
    }
}
// void resetPanelPotis() {
//     memset(panelADCStates, 0, 2 * 2 * 16 * 4);
// }

void potiMapping() {
    for (uint32_t i = 0; i < 2; i++) { // register potis for both layer
        if (allLayers[i]->layerState.value == 1) {

            potiFunctionPointer[i][0][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aParSer), std::placeholders::_1);
            potiFunctionPointer[i][1][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aSpread), std::placeholders::_1);
            potiFunctionPointer[i][2][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aDetune), std::placeholders::_1);
            potiFunctionPointer[i][3][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aGlide), std::placeholders::_1);

            potiFunctionPointer[i][0][1] = std::bind(&Digital::setValueRange, &(allLayers[i]->oscA.dOctave),
                                                     std::placeholders::_1, 585, 4083); // octave switch input range
            potiFunctionPointer[i][1][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aBitcrusher), std::placeholders::_1);
            potiFunctionPointer[i][2][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMorph), std::placeholders::_1);
            potiFunctionPointer[i][3][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMasterTune), std::placeholders::_1);

            potiFunctionPointer[i][0][2] = std::bind(&Digital::setValueRange, &(allLayers[i]->oscB.dOctave),
                                                     std::placeholders::_1, 585, 4083); // octave switch input range
            potiFunctionPointer[i][1][2] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aBitcrusher), std::placeholders::_1);
            potiFunctionPointer[i][2][2] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aMorph), std::placeholders::_1);
            potiFunctionPointer[i][3][2] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aTuning), std::placeholders::_1);

            potiFunctionPointer[i][0][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aNOISELevel), std::placeholders::_1);
            potiFunctionPointer[i][1][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aSUBLevel), std::placeholders::_1);
            potiFunctionPointer[i][2][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCBLevel), std::placeholders::_1);
            potiFunctionPointer[i][3][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCALevel), std::placeholders::_1);

            potiFunctionPointer[i][1][4] =
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aLevel), std::placeholders::_1);
            potiFunctionPointer[i][2][4] =
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aResonance), std::placeholders::_1);
            potiFunctionPointer[i][3][4] =
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aCutoff), std::placeholders::_1);

            potiFunctionPointer[i][1][5] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1);
            potiFunctionPointer[i][2][5] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1);
            potiFunctionPointer[i][3][5] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1);

            potiFunctionPointer[i][1][6] =
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aAmount), std::placeholders::_1);
            potiFunctionPointer[i][2][6] =
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aShape), std::placeholders::_1);
            potiFunctionPointer[i][3][6] =
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aFreq), std::placeholders::_1);

            potiFunctionPointer[i][0][7] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aDelay), std::placeholders::_1);
            potiFunctionPointer[i][1][7] =
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aAmount), std::placeholders::_1);
            potiFunctionPointer[i][2][7] =
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aShape), std::placeholders::_1);
            potiFunctionPointer[i][3][7] =
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aFreq), std::placeholders::_1);

            potiFunctionPointer[i][0][8] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aAttack), std::placeholders::_1);
            potiFunctionPointer[i][1][8] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aDecay), std::placeholders::_1);
            potiFunctionPointer[i][2][8] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aSustain), std::placeholders::_1);
            potiFunctionPointer[i][3][8] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aRelease), std::placeholders::_1);

            potiFunctionPointer[i][0][9] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aSustain), std::placeholders::_1);
            potiFunctionPointer[i][1][9] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aRelease), std::placeholders::_1);
            potiFunctionPointer[i][2][9] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aAmount), std::placeholders::_1);
            potiFunctionPointer[i][3][9] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envF.aAmount), std::placeholders::_1);

            potiFunctionPointer[i][0][10] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aDelay), std::placeholders::_1);
            potiFunctionPointer[i][1][10] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aAttack), std::placeholders::_1);
            potiFunctionPointer[i][2][10] =
                std::bind(&Analog::setValueInversePoti, &(allLayers[i]->envA.aDecay), std::placeholders::_1);
            potiFunctionPointer[i][3][10] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aMaster), std::placeholders::_1);

            potiFunctionPointer[i][0][11] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aDistort), std::placeholders::_1);
            potiFunctionPointer[i][1][11] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aPanSpread), std::placeholders::_1);
            potiFunctionPointer[i][2][11] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aPan), std::placeholders::_1);
            potiFunctionPointer[i][3][11] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aVCA), std::placeholders::_1);
        }
    }
}

//////////// LED ///////////
void renderLED() {
    for (uint32_t i = 0; i < 2; i++) { // for both Layer
        if (allLayers[i]->layerState.value) {

            uint8_t pulseBrightness =
                uint8_t((fast_sin_f32(millis() / 1000.f) + 1) / 2.f * (float)LEDBRIGHTNESS_MEDIUM);

            for (uint32_t x = 0; x < ledDriverA.size(); x++) {
                setAllLEDs(i, x, LEDBRIGHTNESS_OFF);
            }

            if (currentFocus.modul < allLayers[i]->modules.size()) {

                if (currentFocus.type == FOCUSOUTPUT) {

                    if (currentFocus.id < allLayers[i]->modules[currentFocus.modul]->getOutputs().size()) {

                        Output *output = allLayers[i]->modules[currentFocus.modul]->getOutputs()[currentFocus.id];
                        setLED(i, output->LEDPortID, output->LEDPinID, LEDBRIGHTNESS_MAX);

                        for (uint32_t p = 0; p < output->getPatchesInOut().size(); p++) {
                            Input *target = output->getPatchesInOut()[p]->targetIn;
                            setLED(i, target->LEDPortID, target->LEDPinID, pulseBrightness);
                        }
                    }
                }
                else if (currentFocus.type == FOCUSINPUT) {
                    if (currentFocus.id < allLayers[i]->modules[currentFocus.modul]->getInputs().size()) {

                        Input *input = allLayers[i]->modules[currentFocus.modul]->getInputs()[currentFocus.id];
                        setLED(i, input->LEDPortID, input->LEDPinID, LEDBRIGHTNESS_MAX);

                        for (uint32_t p = 0; p < input->getPatchesInOut().size(); p++) {
                            Output *source = input->getPatchesInOut()[p]->sourceOut;
                            setLED(i, source->LEDPortID, source->LEDPinID, pulseBrightness);
                        }
                    }
                }
            }
            // mapping Switch settings to LEDs
            switchLEDMapping();
        }
    }
    // update LEDDriver Outputs
    for (uint32_t i = 0; i < ledDriverA.size(); i++) {
        ledDriverA[i].updateLEDs();
    }
    for (uint32_t i = 0; i < ledDriverB.size(); i++) {
        ledDriverB[i].updateLEDs();
    }
}

void patchLEDMappingInit() {
    for (uint8_t i = 0; i < 2; i++) {
        allLayers[i]->envA.out.LEDPinID = 10;
        allLayers[i]->envA.out.LEDPortID = 0;

        allLayers[i]->envF.out.LEDPinID = 9;
        allLayers[i]->envF.out.LEDPortID = 0;

        allLayers[i]->lfoA.out.LEDPinID = 8;
        allLayers[i]->lfoA.out.LEDPortID = 0;

        allLayers[i]->lfoB.out.LEDPinID = 7;
        allLayers[i]->lfoB.out.LEDPortID = 0;

        allLayers[i]->ladder.iCutoff.LEDPinID = 11;
        allLayers[i]->ladder.iCutoff.LEDPortID = 0;

        allLayers[i]->steiner.iCutoff.LEDPinID = 12;
        allLayers[i]->steiner.iCutoff.LEDPortID = 0;

        allLayers[i]->out.iPan.LEDPinID = 14;
        allLayers[i]->out.iPan.LEDPortID = 0;

        allLayers[i]->out.iVCA.LEDPinID = 15;
        allLayers[i]->out.iVCA.LEDPortID = 0;

        allLayers[i]->out.iDistort.LEDPinID = 13;
        allLayers[i]->out.iDistort.LEDPortID = 0;

        allLayers[i]->feel.oSpread.LEDPinID = 4;
        allLayers[i]->feel.oSpread.LEDPortID = 0;
    }
}

void setLED(uint8_t layer, uint8_t port, uint8_t pin, uint8_t brigthness) {

    if (port == 0xFF || pin == 0xFF)
        return;
    if (layer == 0)
        ledDriverA[port].pwmValue[pin] = brigthness;
    if (layer == 1)
        ledDriverB[port].pwmValue[pin] = brigthness;
}

void setAllLEDs(uint8_t layer, uint8_t port, uint32_t brigthness) {
    if (layer == 0)
        memset(ledDriverA[port].pwmValue, brigthness, 16);
    if (layer == 1)
        memset(ledDriverB[port].pwmValue, brigthness, 16);
}

void switchLEDMapping() {
    if (allLayers[0]->layerState.value == 1) {
        dualLEDSetting(ledDriverA[0].pwmValue[5], ledDriverA[0].pwmValue[6],
                       allLayers[0]->mixer.dOSCADestSwitch.valueMapped);

        quadLEDSetting(ledDriverA[0].pwmValue[0], ledDriverA[0].pwmValue[1], ledDriverA[0].pwmValue[2],
                       ledDriverA[0].pwmValue[3], allLayers[0]->ladder.dSlope.valueMapped);
    }

    if (allLayers[1]->layerState.value == 1) {
        dualLEDSetting(ledDriverB[0].pwmValue[5], ledDriverB[0].pwmValue[6],
                       allLayers[1]->mixer.dOSCADestSwitch.valueMapped);

        quadLEDSetting(ledDriverB[0].pwmValue[0], ledDriverB[0].pwmValue[1], ledDriverB[0].pwmValue[2],
                       ledDriverB[0].pwmValue[3], allLayers[1]->ladder.dSlope.valueMapped);
    }
}

void quadLEDSetting(uint8_t &LED1, uint8_t &LED2, uint8_t &LED3, uint8_t &LED4, int32_t &value) {
    switch (value) {
        case 0:
            LED1 = LEDBRIGHTNESS_MAX;
            LED2 = LEDBRIGHTNESS_OFF;
            LED3 = LEDBRIGHTNESS_OFF;
            LED4 = LEDBRIGHTNESS_OFF;

            break;
        case 1:
            LED1 = LEDBRIGHTNESS_OFF;
            LED2 = LEDBRIGHTNESS_MAX;
            LED3 = LEDBRIGHTNESS_OFF;
            LED4 = LEDBRIGHTNESS_OFF;

            break;
        case 2:
            LED1 = LEDBRIGHTNESS_OFF;
            LED2 = LEDBRIGHTNESS_OFF;
            LED3 = LEDBRIGHTNESS_MAX;
            LED4 = LEDBRIGHTNESS_OFF;
            break;
        case 3:
            LED1 = LEDBRIGHTNESS_OFF;
            LED2 = LEDBRIGHTNESS_OFF;
            LED3 = LEDBRIGHTNESS_OFF;
            LED4 = LEDBRIGHTNESS_MAX;

            break;
    }
}

void dualLEDSetting(uint8_t &LED1, uint8_t &LED2, int32_t &value) {
    switch (value) {
        case 0:
            LED1 = LEDBRIGHTNESS_MAX;
            LED2 = LEDBRIGHTNESS_OFF;

            break;
        case 1:
            LED1 = LEDBRIGHTNESS_OFF;
            LED2 = LEDBRIGHTNESS_MAX;

            break;
        case 2:
            LED1 = LEDBRIGHTNESS_MAX;
            LED2 = LEDBRIGHTNESS_MAX;
            break;
        case 3:
            LED1 = LEDBRIGHTNESS_OFF;
            LED2 = LEDBRIGHTNESS_OFF;

            break;
    }
}
void singleLEDSetting(uint8_t &LED, int32_t &value) {
    LED = value ? LEDBRIGHTNESS_MAX : LEDBRIGHTNESS_OFF;
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
            case 3: evaluateInput((Input *)&(allLayers[layerID]->out.iDistort), event); break;
            case 4: evaluateModul((BaseModule *)&(allLayers[layerID]->mixer), event); break;
            case 5: evaluateOutput((Output *)&(allLayers[layerID]->feel.oSpread), event); break;
            case 6: evaluateOutput((Output *)&(allLayers[layerID]->lfoB.out), event); break;
            case 7: evaluateModul((BaseModule *)&(allLayers[layerID]->out), event); break;
            case 8: evaluateInput((Input *)&(allLayers[layerID]->out.iPan), event); break;
            case 9: evaluateSetting((Digital *)&(allLayers[layerID]->ladder.dSlope), event); break;
            case 10: evaluateSetting((Digital *)&(allLayers[layerID]->mixer.dOSCADestSwitch), event); break;
            case 11: evaluateInput((Input *)&(allLayers[layerID]->out.iVCA), event); break;
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

void PanelTouch::evaluateControl(uint8_t pin, uint8_t port, uint8_t event) {
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
