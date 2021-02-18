#ifdef POLYCONTROL

#include "hid.hpp"

#define PANELACTIVE 0

// ControlPanel
// PCA9555 -> Bus expander for the Encoder
PCA9555 ioExpander = PCA9555(&hi2c2, 0x00);

// create Encoder objects
rotary encoders[NUMBERENCODERS] = {rotary(1, 0), rotary(4, 3), rotary(7, 6), rotary(10, 9)};
tactileSwitch switches[NUMBERENCODERS] = {tactileSwitch(2), tactileSwitch(5), tactileSwitch(8), tactileSwitch(11)};

// create Controller Touch Objects
AT42QT2120 touchControl = AT42QT2120(&hi2c4, 0, 0);

// LayerPanel
// create Panel Touch Objects
AT42QT2120 touchPanel[2][NUMBER_PANELTOUCHICS] = {
    {AT42QT2120(&hi2c4, 0, 0), AT42QT2120(&hi2c4, 0, 0), AT42QT2120(&hi2c4, 2, 0), AT42QT2120(&hi2c4, 3, 0)},
    {AT42QT2120(&hi2c3, 0, 1), AT42QT2120(&hi2c3, 1, 1), AT42QT2120(&hi2c3, 2, 1), AT42QT2120(&hi2c3, 4, 1)}};

IS31FL3216 ledDriver[2][NUMBER_LEDDRIVER] = {{IS31FL3216(&hi2c4, 0, 4, 0), IS31FL3216(&hi2c4, 0, 5, 0),
                                              IS31FL3216(&hi2c4, 0, 6, 0), IS31FL3216(&hi2c4, 0, 7, 0)},
                                             {IS31FL3216(&hi2c3, 0, 4, 1), IS31FL3216(&hi2c3, 0, 5, 1),
                                              IS31FL3216(&hi2c3, 0, 6, 1), IS31FL3216(&hi2c3, 0, 7, 1)}};

PanelTouch touchEvaluteLayer[2] = {PanelTouch(0), PanelTouch(1)};

// Potentiomer ADC
MAX11128 adc[] = {MAX11128(&hspi1, 12, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin),
                  MAX11128(&hspi2, 12, Panel_2_CS_GPIO_Port, Panel_2_CS_Pin)};

// 2 multiplexer connectec at the same control lines
TS3A5017D multiplexer(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                      Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

// array for functionPoint for Poti mapping
std::function<void(uint16_t amount)> potiFunctionPointer[2][4][16]; // number layer, number multplex, number channels

void initHID() {

    // register flagHandler functions

    // Control
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    // Panels
    FlagHandler::Panel_EOC_ISR = std::bind(processPanelPotis);
    FlagHandler::Panel_0_Touch_ISR = std::bind(processPanelTouch, 0);
    FlagHandler::Panel_1_Touch_ISR = std::bind(processPanelTouch, 1);

    initPotiMapping();

    // ControlBoard controls
    // register encoder
    ioExpander.init();
    encoders[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_1_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_1_CCW, &actionHandler));

    encoders[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_2_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_2_CCW, &actionHandler));

    encoders[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_3_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_3_CCW, &actionHandler));

    encoders[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_4_CCW, &actionHandler));

    switches[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_1_Push, &actionHandler), nullptr);

    switches[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_2_Push, &actionHandler), nullptr);

    switches[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_3_Push, &actionHandler), nullptr);

    switches[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_Push, &actionHandler), nullptr);

    // init Control touch IC

    // touchControl.init();  //TODO temp einmal switchen bitte wenn Control Panel dran
    if (allLayers[0]->LayerState.value == 1) {
        touchControl.init();
    }
    // HAL_Delay(50);

    // Panel Controls:

    // init Panels
    for (size_t i = 0; i < 2; i++) {
        if (allLayers[i]->LayerState.value == 1) {
            for (int x = 0; x < NUMBER_PANELTOUCHICS; x++) {
                if (PANELACTIVE) {
                    touchPanel[i][x].init(); // init Touch ICS
                }
            }
            for (int x = 0; x < NUMBER_LEDDRIVER; x++) {
                if (PANELACTIVE) {

                    ledDriver[i][x].init(); // init LED driver}
                }
            }
            adc[i].init(); // ini analog-digital converter
        }
    }
    multiplexer.enableChannels(); // multiplexer for both layer
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

            touchEvaluteLayer[0].event(touchState, x);
            if (HAL_GPIO_ReadPin(Panel_1_Change_GPIO_Port,
                                 Panel_1_Change_Pin)) { // interrupt cleared?
                return;
            }
        }
        else if (layerID == 1) {

            touchEvaluteLayer[1].event(touchState, x);
            if (HAL_GPIO_ReadPin(Panel_2_Change_GPIO_Port,
                                 Panel_2_Change_Pin)) { // interrupt cleared?
                return;
            }
        }
    }
}

void processPanelPotis() {

    static uint16_t sampleDataStates[2][4][16]; // number layer, number multiplex, number channels

    static uint16_t treshold = 3; // threshold for jitter reduction

    // store for current sample data for the 4x16 Multiplexed ADC Values

    uint16_t multiplex = multiplexer.currentChannel;

    multiplexer.nextChannel();

    for (size_t id = 0; id < 2; id++) {                           // for every layer
        if (allLayers[id]->LayerState.value == 1) {               // check layer active state
            adc[id].fetchNewData();                               // fetch ADC data
            for (uint16_t channel = 0; channel < 16; channel++) { // for all Channels
                                                                  // channel changed -> get new Value
                if (abs((sampleDataStates[id][multiplex][channel] -
                         (int16_t)((adc[id].adcData[channel] >> 1) & 0xFFF))) >= treshold) {
                    sampleDataStates[id][multiplex][channel] = (int16_t)(adc[id].adcData[channel] >> 1) & 0xFFF;

                    if (potiFunctionPointer[id][multiplex][channel] != nullptr) { // call function
                        potiFunctionPointer[id][multiplex][channel](sampleDataStates[id][multiplex][channel]);
                    }
                }
            }
        }
    }
}

void processControlTouch() {
    eventControlTouch(touchControl.readTouchStatus());
}

void eventControlTouch(uint16_t touchState) {
    static uint16_t oldTouchState;

    touchEvaluteLayer[0].event(touchState, TOUCH_IO_PORT_A); // TODO sobald panel da weg damit

    uint16_t pushEvent = ~oldTouchState & touchState;
    uint16_t releaseEvent = oldTouchState & ~touchState;

    println("pushEvent   :", pushEvent);
    println("releaseEvent   :", releaseEvent);

    // TODO wenn das platinen Layout für die Control Front fertig ist -> touch zuweißung anpassen
    if (pushEvent) {
        if (pushEvent & (1 << 6)) {
            actionHandler.callActionHeader1();
        }
        if (pushEvent & (1 << 7)) {
            actionHandler.callActionHeader2();
        }
        if (pushEvent & (1 << 8)) {
            actionHandler.callActionHeader3();
        }
        if (pushEvent & (1 << 9)) {
            actionHandler.callActionHeader4();
        }
        if (pushEvent & (1 << 5)) {
            actionHandler.callActionLeft1();
            actionHandler.buttonLeft_1.state = PRESSED;
        }
        if (pushEvent & (1 << 3)) {
            actionHandler.callActionLeft2();
            actionHandler.buttonLeft_2.state = PRESSED;
        }
        if (pushEvent & (1 << 1)) {
            actionHandler.callActionLeft3();
            actionHandler.buttonLeft_3.state = PRESSED;
        }
        if (pushEvent & (1 << 4)) {
            actionHandler.callActionRight1();
            actionHandler.buttonRight_1.state = PRESSED;
        }
        if (pushEvent & (1 << 2)) {
            actionHandler.callActionRight2();
            actionHandler.buttonRight_2.state = PRESSED;
        }
        if (pushEvent & (1 << 0)) {
            actionHandler.callActionRight3();
            actionHandler.buttonRight_3.state = PRESSED;
        }
    }

    if (releaseEvent) {
        if (releaseEvent & (1 << 6)) {
        }
        if (releaseEvent & (1 << 7)) {
        }
        if (releaseEvent & (1 << 8)) {
        }
        if (releaseEvent & (1 << 9)) {
        }
        if (releaseEvent & (1 << 5)) {
            actionHandler.buttonLeft_1.state = RELEASED;
        }
        if (releaseEvent & (1 << 3)) {
            actionHandler.buttonLeft_2.state = RELEASED;
        }
        if (releaseEvent & (1 << 1)) {
            actionHandler.buttonLeft_3.state = RELEASED;
        }
        if (releaseEvent & (1 << 4)) {
            actionHandler.buttonRight_1.state = RELEASED;
        }
        if (releaseEvent & (1 << 2)) {
            actionHandler.buttonRight_2.state = RELEASED;
        }
        if (releaseEvent & (1 << 0)) {
            actionHandler.buttonRight_3.state = RELEASED;
        }
    }

    oldTouchState = touchState;
}

void initPotiMapping() { // TODO fill mapping

    for (size_t i = 0; i < 2; i++) { // register potis for both layer
        potiFunctionPointer[i][0][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1);
        potiFunctionPointer[i][1][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1);
        potiFunctionPointer[i][2][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1);
        potiFunctionPointer[i][3][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->globalModule.aVCA), std::placeholders::_1);
    }
}
void updatePatchLED() {

    if (PANELACTIVE) {

        static location focusCompare = {0, 0, 0, FOCUSLAYER};

        // nothing changed?
        if (currentFocus.id == focusCompare.id && currentFocus.layer == focusCompare.layer &&
            currentFocus.modul == focusCompare.modul && currentFocus.type == focusCompare.type) {
            return;
        }

        if (currentFocus.type == FOCUSOUTPUT) {
            Output *output = allLayers[currentFocus.layer]->modules[currentFocus.modul]->getOutputs()[currentFocus.id];

            uint16_t sourceID = output->idGlobal;
            patchLEDMapping(FOCUSOUTPUT, sourceID, LEDBRIGHTNESS_MAX);

            for (uint8_t i = 0; i < output->getPatchesInOut().size(); i++) {
                uint16_t targetID = output->getPatchesInOut()[i]->targetIn->idGlobal;
                patchLEDMapping(FOCUSINPUT, targetID, LEDBRIGHTNESS_MEDIUM);
            }

            // for (uint8_t i = 0; i < output->getPatchesOutOut().size(); i++) {
            //     uint16_t targetID = output->getPatchesInOut()[i]->targetIn->idGlobal;
            //     patchLEDMapping(FOCUSOUTPUT, targetID, LEDBRIGHTNESS_MEDIUM);
            // }
        }
        else if (currentFocus.type == FOCUSINPUT) {

            Input *input = allLayers[currentFocus.layer]->modules[currentFocus.modul]->getInputs()[currentFocus.id];
            uint16_t inputID = input->idGlobal;
            patchLEDMapping(FOCUSINPUT, inputID, LEDBRIGHTNESS_MAX);

            for (uint8_t i = 0; i < input->getPatchesInOut().size(); i++) {
                uint16_t sourceID = input->getPatchesInOut()[i]->sourceOut->idGlobal;
                patchLEDMapping(FOCUSOUTPUT, sourceID, LEDBRIGHTNESS_MEDIUM);
            }
        }

        // mapping Switch settings to LEDs
        switchLEDMapping();

        // update LEDDriver Outputs
        for (int i = 0; i < 2; i++) {                  // for both Layer
            if (allLayers[i]->LayerState.value == 1) { // check layer State
                for (int x = 0; x < NUMBER_LEDDRIVER; x++) {
                    ledDriver[i][x].updateLEDs();
                    ledDriver[i][x].updateLEDs();
                }
            }
        }
        focusCompare = currentFocus;
    }
}

void patchLEDMapping(FOCUSMODE type, uint32_t id,
                     uint8_t pwm) { // type 0 -> Output | type 1 -> Input
    // TODO mapping fertig machen sobald platine klar

    uint16_t mappedID = 0xFFFF;
    if (type == FOCUSOUTPUT) {
        if (allLayers[currentFocus.layer]->lfoA.out.idGlobal == id) {
            mappedID = 0;
        }
        if (allLayers[currentFocus.layer]->lfoB.out.idGlobal == id) {
            mappedID = 1;
        }
        if (allLayers[currentFocus.layer]->adsrA.out.idGlobal == id) {
            mappedID = 2;
        }
        if (allLayers[currentFocus.layer]->adsrB.out.idGlobal == id) {
            mappedID = 3;
        }
    }

    if (type == FOCUSINPUT) {
        if (allLayers[currentFocus.layer]->lfoA.iFreq.idGlobal == id) {
            mappedID = 5;
        }
        if (allLayers[currentFocus.layer]->lfoB.iFreq.idGlobal == id) {
            mappedID = 6;
        }
        if (allLayers[currentFocus.layer]->adsrA.iAttack.idGlobal == id) {
            mappedID = 8;
        }
        if (allLayers[currentFocus.layer]->adsrB.iAmount.idGlobal == id) {
            mappedID = 9;
        }
    }

    if (mappedID != 0xFFFF) {
        ledDriver[currentFocus.layer][mappedID % 16].pwmValue[(uint8_t)(mappedID - ((mappedID % 16) * 16))] = pwm;
    }
}

void switchLEDMapping() {
    // TODO mapping fertig machen sobald platine klar hier gerade nur beispiele
    // static IS31FL3216 *ledDriver;

    // single led

    // for (uint8_t i = 0; i < 2; i++) {

    //     // Zwischen den Layern wechseln
    //     if (i == 0) {
    //         ledDriver = ledDriverA;
    //     }
    //     else {
    //         ledDriver = ledDriverB;
    //     }

    // switch (allLayers[i]->test.dSelectFilter.value) {
    //     case 0: ledDriver[0].pwmValue[1] = 0; break;
    //     case 1: ledDriver[0].pwmValue[1] = 255; break;
    // };
    // // dual LED
    // switch (allLayers[i]->test.dSelectFilter.value) {
    //     case 0:
    //         ledDriver[0].pwmValue[1] = 255;
    //         ledDriver[0].pwmValue[2] = 0;
    //         break;

    //     case 1:
    //         ledDriver[0].pwmValue[1] = 0;
    //         ledDriver[0].pwmValue[2] = 255;
    //         break;
    // };
    // // Quad LED
    // switch (allLayers[i]->test.dSelectFilter.value) {
    //     case 0:
    //         ledDriver[0].pwmValue[1] = 255;
    //         ledDriver[0].pwmValue[2] = 0;
    //         ledDriver[0].pwmValue[3] = 0;
    //         ledDriver[0].pwmValue[4] = 0;
    //         break;
    //     case 1:
    //         ledDriver[0].pwmValue[1] = 0;
    //         ledDriver[0].pwmValue[2] = 255;
    //         ledDriver[0].pwmValue[3] = 0;
    //         ledDriver[0].pwmValue[4] = 0;
    //         break;
    //     case 2:
    //         ledDriver[0].pwmValue[1] = 0;
    //         ledDriver[0].pwmValue[2] = 0;
    //         ledDriver[0].pwmValue[3] = 255;
    //         ledDriver[0].pwmValue[4] = 0;
    //         break;
    //     case 3:
    //         ledDriver[0].pwmValue[1] = 0;
    //         ledDriver[0].pwmValue[2] = 0;
    //         ledDriver[0].pwmValue[3] = 0;
    //         ledDriver[0].pwmValue[4] = 255;
    //         break;
    // };
    //}
}

#endif
