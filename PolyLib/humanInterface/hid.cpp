#ifdef POLYCONTROL

#include "hid.hpp"

#define PANELACTIVE 1

// ControlPanel
// PCA9555 -> Bus expander for the Encoder
PCA9555 ioExpander = PCA9555(&hi2c2, 0x00);

// create Encoder objects
rotary encoders[NUMBERENCODERS] = {rotary(1, 0), rotary(4, 3), rotary(7, 6), rotary(10, 9), rotary(13, 12)};
tactileSwitch switches[NUMBERENCODERS] = {tactileSwitch(2), tactileSwitch(5), tactileSwitch(8), tactileSwitch(11),
                                          tactileSwitch(14)};

// create Controller Touch Objects
AT42QT2120 touchControl[2] = {AT42QT2120(&hi2c1, 0), AT42QT2120(&hi2c1, 1)};

// LayerPanel
// create Panel Touch Objects
AT42QT2120 touchPanel[2][NUMBER_PANELTOUCHICS] = {{AT42QT2120(&hi2c4, 0, 0), AT42QT2120(&hi2c4, 1, 0)},
                                                  {AT42QT2120(&hi2c3, 0, 1), AT42QT2120(&hi2c3, 1, 1)}};

// AT42QT2120 touchPanel[2][NUMBER_PANELTOUCHICS] = {
//     {AT42QT2120(&hi2c4, 0, 0), AT42QT2120(&hi2c4, 0, 0), AT42QT2120(&hi2c4, 2, 0), AT42QT2120(&hi2c4, 3, 0)},
//     {AT42QT2120(&hi2c3, 0, 1), AT42QT2120(&hi2c3, 1, 1), AT42QT2120(&hi2c3, 2, 1), AT42QT2120(&hi2c3, 4, 1)}};

// IS31FL3216 ledDriver[2][NUMBER_LEDDRIVER] = {{IS31FL3216(&hi2c4, 0, 4, 0), IS31FL3216(&hi2c4, 0, 5, 0),
//                                               IS31FL3216(&hi2c4, 0, 6, 0), IS31FL3216(&hi2c4, 0, 7, 0)},
//                                              {IS31FL3216(&hi2c3, 0, 4, 1), IS31FL3216(&hi2c3, 0, 5, 1),
//                                               IS31FL3216(&hi2c3, 0, 6, 1), IS31FL3216(&hi2c3, 0, 7, 1)}};

IS31FL3216 ledDriver[2][NUMBER_LEDDRIVER] = {{IS31FL3216(&hi2c4, 0, 7, 0)}, {IS31FL3216(&hi2c3, 0, 7, 1)}};

// Objects for evaluating Touch State for the Panel
PanelTouch touchEvaluteLayer[2] = {PanelTouch(0), PanelTouch(1)};

// Potentiomer ADC
MAX11128 adc[] = {MAX11128(&hspi1, 12, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin),
                  MAX11128(&hspi2, 12, Panel_2_CS_GPIO_Port, Panel_2_CS_Pin)};

// Both multiplexer connected at the same control lines
TS3A5017D multiplexer(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                      Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

// Poti mapping array of function pointer
std::function<void(uint16_t amount)> potiFunctionPointer[2][4][16]; // number layer, number multiplex, number channels

void initHID() {

    //// register flagHandler functions ////
    initPotiMapping();

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

    encoders[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_4_CCW, &actionHandler));

    encoders[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_3_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_3_CCW, &actionHandler));

    encoders[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_2_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_2_CCW, &actionHandler));

    encoders[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_1_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_1_CCW, &actionHandler));

    //  switches[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_Push, &actionHandler),
    //  nullptr);

    switches[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_Push, &actionHandler), nullptr);

    switches[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_3_Push, &actionHandler), nullptr);

    switches[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_2_Push, &actionHandler), nullptr);
    switches[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_1_Push, &actionHandler), nullptr);

    // init Control touch IC

    touchControl[0].init(&i2cBusSwitchControl);
    touchControl[1].init(&i2cBusSwitchControl);

    //// Panel Control ////

    elapsedMillis timeout;
    timeout = 0;

    // init Panels
    for (size_t i = 0; i < 2; i++) {
        if (allLayers[i]->LayerState.value == 1) {
            for (int x = 0; x < NUMBER_PANELTOUCHICS; x++) {
                if (PANELACTIVE) {
                    // println("init Touch IC");
                    touchPanel[i][x].init(); // init Touch ICS
                }
            }
            for (int x = 0; x < NUMBER_LEDDRIVER; x++) {
                if (PANELACTIVE) {
                    // println("init LED Driver");
                    ledDriver[i][x].init(); // init LED driver}
                }
            }
            timeout = 0;
            while (adc[i].init()) { // init analog-digital converter
                if (timeout > 100) {
                    PolyError_Handler("ERROR | INIT | Panel ADC init failed -> timeout");
                }
            }
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

    static int16_t sampleDataStates[2][4][16]; // number layer, number multiplex, number channels

    static int16_t treshold = 3; // threshold for jitter reduction

    // store for current sample data for the 4x16 Multiplexed ADC Values

    uint16_t multiplex = multiplexer.currentChannel;

    multiplexer.nextChannel();

    for (uint16_t id = 0; id < 2; id++) {                         // for every layer
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
    eventControlTouch(touchControl[0].readTouchStatus());

    // TODO Temporary solution
    touchEvaluteLayer[0].event(touchControl[1].readTouchStatus(),
                               TOUCH_IO_PORT_C); // send to panel touch command evaluation
}

void eventControlTouch(uint16_t touchStateA) {
    static uint16_t oldTouchStateA;

    uint16_t pushEventA = ~oldTouchStateA & touchStateA;
    uint16_t releaseEventA = oldTouchStateA & ~touchStateA;

    // TODO wenn das platinen Layout für die Control Front fertig ist -> touch zuweißung anpassen
    if (pushEventA) {
        if (pushEventA & (1 << 5)) {
            actionHandler.callActionHeader1();
        }
        if (pushEventA & (1 << 6)) {
            actionHandler.callActionHeader2();
        }
        if (pushEventA & (1 << 7)) {
            actionHandler.callActionHeader3();
        }
        if (pushEventA & (1 << 8)) {
            actionHandler.callActionHeader4();
        }
        if (pushEventA & (1 << 4)) {
            actionHandler.callActionLeft1();
            actionHandler.buttonLeft_1.state = PRESSED;
        }
        if (pushEventA & (1 << 3)) {
            actionHandler.callActionLeft2();
            actionHandler.buttonLeft_2.state = PRESSED;
        }
        if (pushEventA & (1 << 2)) {
            actionHandler.callActionLeft3();
            actionHandler.buttonLeft_3.state = PRESSED;
        }
        if (pushEventA & (1 << 9)) {
            actionHandler.callActionRight1();
            actionHandler.buttonRight_1.state = PRESSED;
        }
        if (pushEventA & (1 << 10)) {
            actionHandler.callActionRight2();
            actionHandler.buttonRight_2.state = PRESSED;
        }
        if (pushEventA & (1 << 11)) {
            actionHandler.callActionRight3();
            actionHandler.buttonRight_3.state = PRESSED;
        }
    }

    if (releaseEventA) {
        if (releaseEventA & (1 << 5)) {
        }
        if (releaseEventA & (1 << 6)) {
        }
        if (releaseEventA & (1 << 7)) {
        }
        if (releaseEventA & (1 << 8)) {
        }
        if (releaseEventA & (1 << 4)) {
            actionHandler.buttonLeft_1.state = RELEASED;
        }
        if (releaseEventA & (1 << 3)) {
            actionHandler.buttonLeft_2.state = RELEASED;
        }
        if (releaseEventA & (1 << 2)) {
            actionHandler.buttonLeft_3.state = RELEASED;
        }
        if (releaseEventA & (1 << 9)) {
            actionHandler.buttonRight_1.state = RELEASED;
        }
        if (releaseEventA & (1 << 10)) {
            actionHandler.buttonRight_2.state = RELEASED;
        }
        if (releaseEventA & (1 << 11)) {
            actionHandler.buttonRight_3.state = RELEASED;
        }
    }

    oldTouchStateA = touchStateA;
}

void initPotiMapping() { // TODO fill mapping

    for (uint16_t i = 0; i < 2; i++) { // register potis for both layer
        potiFunctionPointer[i][0][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->ladder.aLevel), std::placeholders::_1);
        potiFunctionPointer[i][1][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1);
        potiFunctionPointer[i][2][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->globalModule.aVCA), std::placeholders::_1);
        potiFunctionPointer[i][3][0] =
            std::bind(&Analog::setValue, &(allLayers[i]->globalModule.aSpread), std::placeholders::_1);

        potiFunctionPointer[i][0][1] =
            std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1);
        potiFunctionPointer[i][1][1] =
            std::bind(&Analog::setValue, &(allLayers[i]->distort.aDistort), std::placeholders::_1);
        potiFunctionPointer[i][2][1] =
            std::bind(&Analog::setValue, &(allLayers[i]->globalModule.aGlide), std::placeholders::_1);
        potiFunctionPointer[i][3][1] =
            std::bind(&Analog::setValue, &(allLayers[i]->globalModule.aDetune), std::placeholders::_1);

        potiFunctionPointer[i][0][2] = std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aFreq), std::placeholders::_1);
        potiFunctionPointer[i][1][2] =
            std::bind(&Analog::setValue, &(allLayers[i]->oscA.aLevel), std::placeholders::_1);
        potiFunctionPointer[i][2][2] =
            std::bind(&Analog::setValue, &(allLayers[i]->oscB.aLevel), std::placeholders::_1);
        potiFunctionPointer[i][3][2] =
            std::bind(&Analog::setValue, &(allLayers[i]->ladder.aResonance), std::placeholders::_1);

        potiFunctionPointer[i][0][3] =
            std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMorph), std::placeholders::_1);
        potiFunctionPointer[i][1][3] =
            std::bind(&Analog::setValue, &(allLayers[i]->oscB.aMorph), std::placeholders::_1);
        potiFunctionPointer[i][2][3] =
            std::bind(&Analog::setValue, &(allLayers[i]->ladder.aCutoff), std::placeholders::_1);
        potiFunctionPointer[i][3][3] =
            std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1);
    }
}
void updatePatchLED() {

    if (PANELACTIVE) {

        static location focusCompare = {0, 0, 0, FOCUSLAYER};

        // nothing changed?
        if (currentFocus.id == focusCompare.id && currentFocus.layer == focusCompare.layer &&
            currentFocus.modul == focusCompare.modul && currentFocus.type == focusCompare.type) {
        }
        else {

            if (currentFocus.type == FOCUSOUTPUT) {
                Output *output =
                    allLayers[currentFocus.layer]->modules[currentFocus.modul]->getOutputs()[currentFocus.id];

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
        }

        // mapping Switch settings to LEDs
        switchLEDMapping();

        // update LEDDriver Outputs
        for (int i = 0; i < 2; i++) {                  // for both Layer
            if (allLayers[i]->LayerState.value == 1) { // check layer State
                for (int x = 0; x < NUMBER_LEDDRIVER; x++) {
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

    // single led

    for (uint8_t i = 0; i < 1; i++) { // todo number layers

        //     // Zwischen den Layern wechseln

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
        switch (allLayers[i]->oscA.dVcfDestSwitch.valueMapped) {
            case 0:
                ledDriver[i][0].pwmValue[5] = 255;
                ledDriver[i][0].pwmValue[6] = 0;

                break;
            case 1:
                ledDriver[i][0].pwmValue[5] = 0;
                ledDriver[i][0].pwmValue[6] = 255;

                break;
            case 2:
                ledDriver[i][0].pwmValue[5] = 255;
                ledDriver[i][0].pwmValue[6] = 255;
                break;
            case 3:
                ledDriver[i][0].pwmValue[5] = 0;
                ledDriver[i][0].pwmValue[6] = 0;

                break;
        }

        // fast ledselect, only  possible if LEDs are in a pin row
        (uint32_t &)(ledDriver[i][0].pwmValue[0]) = 0x000000FF << (allLayers[i]->ladder.dSlope.valueMapped * 8);

        ledDriver[i][0].pwmValue[13] = allLayers[i]->adsrA.dLatch.valueMapped ? 255 : 0;
        ledDriver[i][0].pwmValue[14] = allLayers[i]->adsrB.dLatch.valueMapped ? 255 : 0;

        println(allLayers[0]->ladder.dSlope.valueMapped);
    }
}

#endif
