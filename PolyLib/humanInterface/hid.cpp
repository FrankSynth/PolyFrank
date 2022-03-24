#ifdef POLYCONTROL

#include "hid.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include <string.h>

#define PANELACTIVE 1

extern PCA9548 i2cBusSwitchLayer[2];
extern PCA9548 i2cBusSwitchControl;

int16_t panelADCStates[2][4][16]; // number layer, number multiplex, number channels

// ControlPanel
// PCA9555 -> Bus expander for the Encoder
PCA9555 ioExpander = PCA9555(&hi2c2, 0x00);

// create Encoder objects
rotary encoders[NUMBERENCODERS] = {rotary(1, 0), rotary(4, 3), rotary(7, 6), rotary(10, 9), rotary(13, 12)};
tactileSwitch switches[NUMBERENCODERS] = {tactileSwitch(2), tactileSwitch(5), tactileSwitch(8), tactileSwitch(11),
                                          tactileSwitch(14)};

// create Controller Touch Objects
AT42QT2120 touchControl[NUMBER_CONTROLTOUCHICS] = {AT42QT2120(&hi2c1, 0), AT42QT2120(&hi2c1, 1)};

// LayerPanel
// create Panel Touch Objects
AT42QT2120 touchPanel[2][NUMBER_PANELTOUCHICS] = {{AT42QT2120(&hi2c4, 0), AT42QT2120(&hi2c4, 1)},
                                                  {AT42QT2120(&hi2c3, 0), AT42QT2120(&hi2c3, 1)}};

IS31FL3216 ledDriver[2][NUMBER_LEDDRIVER] = {{IS31FL3216(&hi2c4, 0, 7)}, {IS31FL3216(&hi2c3, 0, 7)}};

// Objects for evaluating Touch State for the LayerPanel
PanelTouch touch;

// Potentiomer ADC
MAX11128 adc[] = {MAX11128(&hspi1, 12, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin),
                  MAX11128(&hspi2, 12, Panel_2_CS_GPIO_Port, Panel_2_CS_Pin)};

// Both multiplexer connected at the same control lines
TS3A5017D multiplexer(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                      Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

// Poti mapping array of function pointer
std::function<void(uint16_t amount)> potiFunctionPointer[2][4][16]; // number layer, number multiplex, number channels

void initHID() {

    // Control
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    // Panels
    FlagHandler::Panel_EOC_ISR = std::bind(processPanelPotis);
    FlagHandler::Panel_0_Touch_ISR = std::bind(processPanelTouch, 0);
    FlagHandler::Panel_1_Touch_ISR = std::bind(processPanelTouch, 1);

    //// ControlBoard controls ////
    // register encoder
    ioExpander.init();
    // encoders[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_1_CW, &actionHandler),
    //                                   std::bind(&actionMapping::callActionEncoder_1_CCW, &actionHandler));
    encoders[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 4),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 4));

    encoders[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 3),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 3));

    encoders[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 2),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 2));

    encoders[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 1),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 1));

    encoders[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 0),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 0));

    // encoders[5].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_6_CW, &actionHandler),
    //                                    std::bind(&actionMapping::callActionEncoder_6_CCW, &actionHandler));

    //  switches[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_Push, &actionHandler),
    //  nullptr);

    switches[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 4), nullptr);
    switches[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 3), nullptr);

    switches[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 2), nullptr);

    switches[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 1), nullptr);
    switches[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 0), nullptr);

    // init Control touch IC

    touchControl[0].init(&i2cBusSwitchControl);
    touchControl[1].init(&i2cBusSwitchControl);

    //// Panel Control ////

    //// register flagHandler functions ////
    initPotiMapping();

    // init Panels
    // init Layer 0

    if (allLayers[0]->layerState.value == 1) {
        touchPanel[0][0].init(&i2cBusSwitchLayer[0]); // touch IC
        touchPanel[0][1].init(&i2cBusSwitchLayer[0]); // touch IC

        ledDriver[0][0].init(&i2cBusSwitchLayer[0]); // init LED driver

        adc[0].init(); // init DAC
    }
    // init Layer 1

    if (allLayers[1]->layerState.value == 1) {
        touchPanel[1][0].init(&i2cBusSwitchLayer[1]); // touch IC
        touchPanel[1][1].init(&i2cBusSwitchLayer[1]); // touch IC

        ledDriver[1][0].init(&i2cBusSwitchLayer[1]); // init LED driver

        adc[1].init(); // init DAC
    }

    patchLEDMappingInit();

    // activate FlagHandling for HID ICs
    FlagHandler::HID_Initialized = true;
}

void processEncoder() {

    uint16_t state = ioExpander.getPinChangeRAW();

    for (int x = 0; x < NUMBERENCODERS; x++) {

        encoders[x].process(state);
        switches[x].process(state);
    }
}

void processPanelTouch(uint8_t layerID) {
    for (int x = 0; x < NUMBER_PANELTOUCHICS; x++) {
        uint16_t touchState = touchPanel[layerID][x].readTouchStatus();

        if (layerID == 0) {

            touch.eventLayer(layerID, touchState, x);
            if (HAL_GPIO_ReadPin(Panel_1_Change_GPIO_Port,
                                 Panel_1_Change_Pin)) { // interrupt cleared?
                FlagHandler::Panel_0_Touch_Interrupt = false;
                return;
            }
        }
        else if (layerID == 1) {

            touch.eventLayer(layerID, touchState, x);
            if (HAL_GPIO_ReadPin(Panel_2_Change_GPIO_Port,
                                 Panel_2_Change_Pin)) { // interrupt cleared?
                FlagHandler::Panel_1_Touch_Interrupt = false;

                return;
            }
        }
    }
}

void processControlTouch() {

    for (int x = 0; x < NUMBER_CONTROLTOUCHICS; x++) {
        uint16_t touchState = touchControl[x].readTouchStatus();

        touch.eventControl(touchState, x);

        if (HAL_GPIO_ReadPin(Control_Touch_Change_GPIO_Port,
                             Control_Touch_Change_Pin)) { // interrupt cleared?
            FlagHandler::Control_Touch_Interrupt = false;

            return;
        }
    }
}

void processPanelPotis() {

    static int16_t treshold = 3; // threshold for jitter reduction

    // store for current sample data for the 4x16 Multiplexed ADC Values

    uint16_t multiplex = multiplexer.currentChannel;

    multiplexer.nextChannel();

    for (uint16_t id = 0; id < 2; id++) {                         // for every layer
        if (allLayers[id]->layerState.value == 1) {               // check layer active state
            adc[id].fetchNewData();                               // fetch ADC data
            for (uint16_t channel = 0; channel < 16; channel++) { // for all Channels

                if (abs((panelADCStates[id][multiplex][channel] -
                         (int16_t)((adc[id].adcData[channel] >> 1) & 0xFFF))) >= treshold) {
                    panelADCStates[id][multiplex][channel] = (int16_t)(adc[id].adcData[channel] >> 1) & 0xFFF;

                    if (potiFunctionPointer[id][multiplex][channel] != nullptr) { // call function
                        potiFunctionPointer[id][multiplex][channel](panelADCStates[id][multiplex][channel]);
                    }
                    // println("ID :", id, "Multiplex :", multiplex, "Channel  :", channel);
                }
            }
        }
    }
}
void resetPanelPotis() {
    memset(panelADCStates, 0, 2 * 2 * 16 * 4);
}

void initPotiMapping() {

    for (uint16_t i = 0; i < 2; i++) { // register potis for both layer
        if (allLayers[i]->layerState.value == 1) {

            potiFunctionPointer[i][0][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aCutoff), std::placeholders::_1);

            potiFunctionPointer[i][1][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aMorph), std::placeholders::_1);

            potiFunctionPointer[i][2][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMorph), std::placeholders::_1);

            potiFunctionPointer[i][3][0] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aMaster), std::placeholders::_1);

            potiFunctionPointer[i][0][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCBLevel), std::placeholders::_1);

            potiFunctionPointer[i][1][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCALevel), std::placeholders::_1);

            potiFunctionPointer[i][2][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aGlide), std::placeholders::_1);

            potiFunctionPointer[i][3][1] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1);

            potiFunctionPointer[i][0][2] =
                std::bind(&Digital::setValue, &(allLayers[i]->oscA.dOctave), std::placeholders::_1);

            potiFunctionPointer[i][1][2] =
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aSpread), std::placeholders::_1);

            potiFunctionPointer[i][2][2] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1);

            potiFunctionPointer[i][3][2] =
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aResonance), std::placeholders::_1);

            potiFunctionPointer[i][0][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->out.aDistort), std::placeholders::_1);

            potiFunctionPointer[i][1][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1);

            potiFunctionPointer[i][2][3] =
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aLevel), std::placeholders::_1);

            potiFunctionPointer[i][3][3] =
                std::bind(&Digital::setValue, &(allLayers[i]->oscB.dOctave), std::placeholders::_1);
        }

        // if (allLayers[i]->layerState.value == 1) {

        //     potiFunctionPointer[i][0][0] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->ladder.aLevel), std::placeholders::_1);
        //     potiFunctionPointer[i][1][0] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1);
        //     potiFunctionPointer[i][2][0] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->out.aVCA), std::placeholders::_1);
        //     potiFunctionPointer[i][3][0] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->out.aMaster), std::placeholders::_1);

        //     potiFunctionPointer[i][0][1] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1);
        //     potiFunctionPointer[i][1][1] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->distort.aDistort), std::placeholders::_1);
        //     potiFunctionPointer[i][2][1] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->feel.aGlide), std::placeholders::_1);
        //     potiFunctionPointer[i][3][1] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->out.aDetune), std::placeholders::_1);

        //     potiFunctionPointer[i][0][2] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->feel.aSpread), std::placeholders::_1);
        //     potiFunctionPointer[i][1][2] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->oscA.aLevel), std::placeholders::_1);
        //     potiFunctionPointer[i][2][2] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->oscB.aLevel), std::placeholders::_1);
        //     potiFunctionPointer[i][3][2] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->ladder.aResonance), std::placeholders::_1);

        //     potiFunctionPointer[i][0][3] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMorph), std::placeholders::_1);
        //     potiFunctionPointer[i][1][3] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->oscB.aMorph), std::placeholders::_1);
        //     potiFunctionPointer[i][2][3] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->ladder.aCutoff), std::placeholders::_1);
        //     potiFunctionPointer[i][3][3] =
        //         std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1);
        // }
    }
}
void renderLED() {
    for (int i = 0; i < 2; i++) { // for both Layer

        if (allLayers[i]->layerState.value) {

            // static location focusCompare = {0, 0, 0, FOCUSLAYER};
            uint8_t pulseBrightness =
                uint8_t((fast_sin_f32(millis() / 1000.f) + 1) / 2.f * (float)LEDBRIGHTNESS_MEDIUM);

            for (int x = 0; x < NUMBER_LEDDRIVER; x++) {
                setAllLEDs(i, x, LEDBRIGHTNESS_OFF);
            }

            // nothing changed?
            // if (currentFocus.id == focusCompare.id && currentFocus.layer == focusCompare.layer &&
            //     currentFocus.modul == focusCompare.modul && currentFocus.type == focusCompare.type) {
            // }
            // else {

            if (currentFocus.type == FOCUSOUTPUT) {
                Output *output =
                    allLayers[currentFocus.layer]->modules[currentFocus.modul]->getOutputs()[currentFocus.id];

                setLED(currentFocus.layer, output->LEDPortID, output->LEDPinID, LEDBRIGHTNESS_MAX);

                for (uint8_t i = 0; i < output->getPatchesInOut().size(); i++) {
                    Input *target = output->getPatchesInOut()[i]->targetIn;
                    setLED(currentFocus.layer, target->LEDPortID, target->LEDPinID, pulseBrightness);
                }
            }
            else if (currentFocus.type == FOCUSINPUT) {

                Input *input = allLayers[currentFocus.layer]->modules[currentFocus.modul]->getInputs()[currentFocus.id];
                setLED(currentFocus.layer, input->LEDPortID, input->LEDPinID, LEDBRIGHTNESS_MAX);

                for (uint8_t i = 0; i < input->getPatchesInOut().size(); i++) {
                    Output *source = input->getPatchesInOut()[i]->sourceOut;
                    setLED(currentFocus.layer, source->LEDPortID, source->LEDPinID, pulseBrightness);
                }
            }
            // }

            // mapping Switch settings to LEDs
            switchLEDMapping();

            // update LEDDriver Outputs
            for (int x = 0; x < NUMBER_LEDDRIVER; x++) {
                ledDriver[i][x].updateLEDs();
            }
        }
        // focusCompare = currentFocus;
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

void setLED(uint8_t layer, uint8_t port, uint8_t pin, uint32_t brigthness) {
    if (port == 0xFF)
        return;
    ledDriver[layer][port].pwmValue[pin] = brigthness;
}

void setAllLEDs(uint8_t layer, uint8_t port, uint32_t brigthness) {
    memset(ledDriver[layer][port].pwmValue, brigthness, 16);
}

void switchLEDMapping() {

    for (uint8_t i = 0; i < 2; i++) {
        if (allLayers[i]->layerState.value == 1) {
            dualLEDSetting(ledDriver[i][0].pwmValue[5], ledDriver[i][0].pwmValue[6],
                           allLayers[i]->mixer.dOSCADestSwitch.valueMapped);

            quadLEDSetting(ledDriver[i][0].pwmValue[0], ledDriver[i][0].pwmValue[1], ledDriver[i][0].pwmValue[2],
                           ledDriver[i][0].pwmValue[3], allLayers[i]->ladder.dSlope.valueMapped);
        }
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

#endif
