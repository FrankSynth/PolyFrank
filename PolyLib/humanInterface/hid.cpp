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

MAX11128 maxA(&hspi1, 16, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin);

//
TS3A5017D multiplexerA = TS3A5017D(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                                   Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

void initHID() {

    // register flagHandler functions
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Panel_0_Touch_ISR = std::bind(processPanelTouch, 0);
    FlagHandler::Panel_1_Touch_ISR = std::bind(processPanelTouch, 1);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    FlagHandler::Panel_0_EOC_ISR = std::bind(processPotentiometer, 0);
    FlagHandler::Panel_1_EOC_ISR = std::bind(processPotentiometer, 1);

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
    maxA.init();

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

void processPotentiometer(uint8_t layerID) { // TODO split event with layerID

    // multiplexerA.nextChannel();
    multiplexerA.setChannel(0);

    maxA.fetchNewData();

    println((maxA.adcData[0] >> 3) & 0x0FFF);
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
    if (event & (1 << 6)) {
        actionHandler.callActionLeft1();
    }
    if (event & (1 << 4)) {
        actionHandler.callActionLeft2();
    }
    if (event & (1 << 2)) {
        actionHandler.callActionLeft3();
    }
    if (event & (1 << 5)) {
        actionHandler.callActionRight1();
    }
    if (event & (1 << 3)) {
        actionHandler.callActionRight2();
    }
    if (event & (1 << 1)) {
        actionHandler.callActionRight3();
    }

    oldTouchState = touchState;
}

#endif
