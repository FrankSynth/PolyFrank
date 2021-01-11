#ifdef POLYCONTROL

#include "hid.hpp"

#define NUMBERENCODERS 4
#define NUMBER_PANELTOUCHICS 1

// PCA9555 -> Bus expander for the Encoder
PCA9555 ioExpander = PCA9555(&hi2c2, 0x00);

// create Encoder objects
rotary encoders[NUMBERENCODERS] = {rotary(1, 0), rotary(4, 3), rotary(7, 6), rotary(10, 9)};
tactileSwitch switches[NUMBERENCODERS] = {tactileSwitch(2), tactileSwitch(5), tactileSwitch(8), tactileSwitch(11)};

// create Panel Touch Objects
AT42QT2120 touchPanel[NUMBER_PANELTOUCHICS] = {AT42QT2120(&hi2c4, 0)};

// create Controller Touch Objects
AT42QT2120 touchControl = AT42QT2120(&hi2c4, 0); // TODO anderen Constructor nehmne

// TODO ledDriver zu Display
IS31FL3216 ledDriverA = IS31FL3216(&hi2c4, 0, 7);
// IS31FL3216 ledDriverB = IS31FL3216(&hi2c4, 0, 6);

PanelTouch touchEvaluteLayer0(0);
PanelTouch touchEvaluteLayer1(1);

// Potentiomer ADC

MAX11128 adcA(&hspi1, 16, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin);

//
TS3A5017D multiplexerA = TS3A5017D(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                                   Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

// array for functionPoint for Poti mapping
std::function<void(uint16_t amount)> potiFunctionPointerA0[16];
std::function<void(uint16_t amount)> potiFunctionPointerA1[16];
std::function<void(uint16_t amount)> potiFunctionPointerA2[16];
std::function<void(uint16_t amount)> potiFunctionPointerA3[16];

void initHID() {

    //

    // register flagHandler functions
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Panel_0_Touch_ISR = std::bind(processPanelTouch, 0);
    FlagHandler::Panel_1_Touch_ISR = std::bind(processPanelTouch, 1);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    FlagHandler::Panel_0_EOC_ISR = std::bind(processPanelPotis, 0);
    FlagHandler::Panel_1_EOC_ISR = std::bind(processPanelPotis, 1);

    ioExpander.init();

    // register encoder
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

    HAL_Delay(1000);

    // TODO ledDriver zu Display
    // ledDriverA.init();

    // init Panel touch ICS
    for (int x = 0; x < NUMBER_PANELTOUCHICS; x++) {
        touchPanel[x].init();
    }
    // init Control touch IC
    touchControl.init();

    // ledDriverB.init();

    // init ADC, Multiplexer

    initPotiMapping();

    adcA.init();
    multiplexerA.enableChannels();
}

void processEncoder() {

    uint16_t state = ioExpander.getPinChangeRAW();

    for (int x = 0; x < NUMBERENCODERS; x++) {

        encoders[x].process(state);
        switches[x].process(state);
    }
}

void processPanelTouch(uint8_t layerID) { // TODO split event with layerID

    // println("Process Panel Touch");

    for (int x = 0; x < NUMBER_PANELTOUCHICS; x++) {

        uint16_t touchState = touchPanel[x].readTouchStatus();
        // println("-->TouchState: ", touchState);

        if (layerID == 0) {

            // println("---> layer ID 0");

            touchEvaluteLayer0.event(touchState, x);
            if (HAL_GPIO_ReadPin(Panel_1_Change_GPIO_Port, Panel_1_Change_Pin)) { // interrupt cleared?
                return;
            }
        }

        else if (layerID == 1) {

            touchEvaluteLayer1.event(touchState, x);
            if (HAL_GPIO_ReadPin(Panel_2_Change_GPIO_Port, Panel_2_Change_Pin)) { // interrupt cleared?
                return;
            }
        }
    }
}

void processPanelPotis(uint8_t layerID) { // TODO split event with layerID

    // threshold for jitter reduction
    static uint16_t treshold = 3;

    // store for current sample data for the 4x16 Multiplexed ADC Values
    static int16_t sampleDataStateA0[16];
    static int16_t sampleDataStateA1[16];
    static int16_t sampleDataStateA2[16];
    static int16_t sampleDataStateA3[16];

    static int16_t *sampleDataStates[] = {sampleDataStateA0, sampleDataStateA1, sampleDataStateA2, sampleDataStateA3};

    uint16_t activeChannel = multiplexerA.currentChannel;
    int16_t *activeSampleDataState = sampleDataStates[activeChannel];

    multiplexerA.nextChannel();
    adcA.fetchNewData();

    // TODO umbau auf SIMD instuction
    for (uint16_t i = 0; i < 1; i++) { // TODO increase to 16
        if (abs((activeSampleDataState[i] - (int16_t)((adcA.adcData[i] >> 1) & 0xFFF))) >= treshold) {
            activeSampleDataState[i] = (int16_t)(adcA.adcData[i] >> 1) & 0xFFF;

            mapPanelPotis(activeChannel, i, activeSampleDataState[i]);
        }
    }
}

void processControlTouch() {

    // println("Process Control Touch");

    eventControlTouch(touchControl.readTouchStatus());

    if (HAL_GPIO_ReadPin(Panel_1_Change_GPIO_Port, Panel_1_Change_Pin)) { // interrupt cleared?
        return;
    }
}

void eventControlTouch(uint16_t touchState) {
    static uint16_t oldTouchState;

    if (!touchState) { // all touch buttons released
        oldTouchState = touchState;
        return;
    }

    uint16_t event = oldTouchState ^ touchState;

    event = event & touchState;

    // TODO wenn das platinen Layout für die Control Front fertig ist -> touch zuweißung anpassen

    if (event & (1 << 6)) {
        actionHandler.callActionHeader1();
    }
    if (event & (1 << 7)) {
        actionHandler.callActionHeader2();
    }
    if (event & (1 << 8)) {
        actionHandler.callActionHeader3();
    }
    if (event & (1 << 9)) {
        actionHandler.callActionHeader4();
    }
    if (event & (1 << 5)) {
        actionHandler.callActionLeft1();
    }
    if (event & (1 << 3)) {
        actionHandler.callActionLeft2();
    }
    if (event & (1 << 1)) {
        actionHandler.callActionLeft3();
    }
    if (event & (1 << 4)) {
        actionHandler.callActionRight1();
    }
    if (event & (1 << 2)) {
        actionHandler.callActionRight2();
    }
    if (event & (1 << 0)) {
        actionHandler.callActionRight3();
    }

    oldTouchState = touchState;
}

void mapPanelPotis(uint16_t activeChannel, uint16_t ID, uint16_t value) { // TODO interpolation

    // println("value : ", value);

    if (activeChannel == 0) {
        if (potiFunctionPointerA0[ID] != nullptr) {
            potiFunctionPointerA0[ID](value);
        }
        return;
    }

    if (activeChannel == 1) {
        if (potiFunctionPointerA1[ID] != nullptr) {
            potiFunctionPointerA1[ID](value);
        }
        return;
    }

    if (activeChannel == 2) {
        if (potiFunctionPointerA2[ID] != nullptr) {
            potiFunctionPointerA2[ID](value);
        }
        return;
    }

    if (activeChannel == 3) {
        if (potiFunctionPointerA3[ID] != nullptr) {
            potiFunctionPointerA3[ID](value);
        }
        return;
    }
}

void initPotiMapping() { // TODO fill mapping

    // potiFunctionPointerA3[0] = std::bind(&Analog::setValue, &(allLayers[0]->oscA.aBitcrusher),
    // std::placeholders::_1); potiFunctionPointerA1[0] = std::bind(&Analog::setValue, &(allLayers[0]->oscA.aDetune),
    // std::placeholders::_1); potiFunctionPointerA0[0] = std::bind(&Analog::setValue, &(allLayers[0]->oscA.aFM),
    // std::placeholders::_1);
    // potiFunctionPointerA2[0] = std::bind(&Analog::setValue, &(allLayers[0]->oscA.aLevel), std::placeholders::_1);

    // potiFunctionPointerA0[0] = std::bind(&Analog::setValue, &(allLayers[0]->adsrA.aAttack), std::placeholders::_1);
    // potiFunctionPointerA1[0] = std::bind(&Analog::setValue, &(allLayers[0]->adsrA.aDecay), std::placeholders::_1);
    // potiFunctionPointerA2[0] = std::bind(&Analog::setValue, &(allLayers[0]->adsrA.aSustain), std::placeholders::_1);
    // potiFunctionPointerA3[0] = std::bind(&Analog::setValue, &(allLayers[0]->adsrA.aRelease), std::placeholders::_1);
}

#endif
