#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"

#include "midiInterface/MIDIInterface.h"
#include "usbd_midi_if.hpp"

// GUI
extern midi::MidiInterface<midiUSB::COMusb> mididevice;
extern GUI ui;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern ADC_HandleTypeDef hadc3;

extern devManager deviceManager;

// List of all busObjects
spiBus spiBusEEPROM;
spiBus spiBusLayer;
spiBus spiBusPanel;

i2cBus i2cBusTouch;
i2cBus i2cBusIOExp;
i2cBus i2cBusPanel1;
i2cBus i2cBusPanel2;

i2cVirtualBus i2cBusPanel1A;
i2cVirtualBus i2cBusPanel1B;
i2cVirtualBus i2cBusPanel1H;

i2cVirtualBus i2cBusTouchA;
i2cVirtualBus i2cBusTouchB;

// List of all devices
// Control Touch devices
std::vector<AT42QT2120> touchControl;

//  Panel Touch devices
std::vector<AT42QT2120> touchPanelA;
std::vector<AT42QT2120> touchPanelB;

// I2C BusMultiplexer
PCA9548 busMultiplexerControl;
PCA9548 busMultiplexerPanelA;

// IO Expander
PCA9555 ioExpander;

//  ADC
MAX11128 adcA;
MAX11128 adcB;
TS3A5017D multiplexer(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                      Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

// LED Driver
std::vector<IS31FL3216> ledDriverA;
std::vector<IS31FL3216> ledDriverB;

M95M01 eeprom;

// Encoder
rotary encoders[NUMBERENCODERS] = {rotary(1, 0), rotary(4, 3), rotary(7, 6), rotary(10, 9), rotary(13, 12)};
tactileSwitch switches[NUMBERENCODERS] = {tactileSwitch(2), tactileSwitch(5), tactileSwitch(8), tactileSwitch(11),
                                          tactileSwitch(14)};

// ADC

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABufferLayerA[2 * INTERCHIPBUFFERSIZE];

// USB
midi::MidiInterface<midiUSB::COMusb> mididevice(MIDIComRead);

// Layer
ID layerId;
Layer layerA(layerId.getNewId());
Layer layerB(layerId.getNewId());

uint8_t sendRequestUIData();

// InterChip Com
COMinterChip layerCom;

void midiConfig();
void temperature();
void deviceConfig();

void PolyControlInit() {

    // Layer
    allLayers.push_back(&layerA);
    allLayers.push_back(&layerB);

    initPoly();

    // Enable Layer Board
    HAL_GPIO_WritePin(Layer_Reset_GPIO_Port, Layer_Reset_Pin, GPIO_PIN_SET);
    // Enable Panel Board
    HAL_GPIO_WritePin(Panel_Reset_GPIO_Port, Panel_Reset_Pin, GPIO_PIN_SET);
    // Enable Control Panel Board
    HAL_GPIO_WritePin(Control_Reset_GPIO_Port, Control_Reset_Pin, GPIO_PIN_SET);

    // let the layer start
    HAL_Delay(500);

    // Device Configuration
    deviceConfig();
    HIDConfig();

    for (Layer *l : allLayers) {
        l->resetLayer();
    }

    // Preset
    updatePresetList();                  // read preset List from EEPROM
    globalSettings.loadGlobalSettings(); // load global Settings

    // CheckLayerStatus
    if (FlagHandler::renderChip_State[0][0] == READY && FlagHandler::renderChip_State[0][1] == READY) { // Layer A alive
        layerA.layerState.value = 1;
        FlagHandler::layerActive[0] = true;
    }
    else {
        globalSettings.midiLayerAChannel.disable = 1;
        FlagHandler::renderChip_State[0][0] = DISABLED;
        FlagHandler::renderChip_State[0][1] = DISABLED;
    }

    if (FlagHandler::renderChip_State[1][0] == READY && FlagHandler::renderChip_State[1][1] == READY) { // Layer B alive
        layerB.layerState.value = 1;
        FlagHandler::layerActive[1] = true;
    }
    else {
        globalSettings.midiLayerBChannel.disable = 1;

        FlagHandler::renderChip_State[1][0] = DISABLED;
        FlagHandler::renderChip_State[1][1] = DISABLED;
    }

    if (layerA.layerState.value && layerB.layerState.value) {
        globalSettings.multiLayer.value = 1;
    }
    else {                                                    // disable multilayerSettings
        liveData.livemodeKeysplit.disable = 1;                // disable Key splitting
        liveData.voiceHandler.livemodeMergeLayer.disable = 1; // disable merge Layers
    }

    if (layerA.layerState.value == 0 && layerB.layerState.value == 0) {
        PolyError_Handler("ERROR | FATAL | NO Layer Connected!");
    }

    FlagHandler::readTemperature_ISR = temperature; // registerFunction pointer to ISR

    // Init Encoder, Touchbuttons,..
    HAL_Delay(500);

    // User Interface
    if (layerA.layerState.value == 1) {
        newFocus = {0, 0, 0, FOCUSMODULE};
    }
    else if (layerB.layerState.value == 1) {
        newFocus = {1, 0, 0, FOCUSMODULE};
    }

    ui.Init();

    // Interchip communication
    layerCom.initOutTransmission(
        std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi4, std::placeholders::_1, std::placeholders::_2),
        (uint8_t *)interChipDMABufferLayerA);

    layerCom.initInTransmission(
        std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi4, std::placeholders::_1, std::placeholders::_2),
        std::bind<uint8_t>(HAL_SPI_Abort, &hspi4), (uint8_t *)interChipDMABufferLayerA);

    // Midi configuration
    midiConfig();

    // Say hello
    println("Hi, Frank here!");
    if (layerA.layerState.value) {
        println("Layer A active");
    }
    if (layerB.layerState.value) {
        println("Layer B active");
    }

    // And turn the Display on
    HAL_GPIO_WritePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin, GPIO_PIN_SET);
}

elapsedMillis askMessage = 0;

void PolyControlRun() { // Here the party starts
    while (1) {

        if (askMessage > 1000) {
            askMessage = 0;
            sendRequestUIData();
        }

        mididevice.read();
        liveData.serviceRoutine();
        FlagHandler::handleFlags();

        for (uint8_t i = 0; i < 2; i++) {
            layerCom.beginSendTransmission();
        }

        if (getRenderState() == RENDER_DONE) {
            ui.Draw();
            renderLED();
        }

        // Com Receive Buffer
        //        if (comAvailable()) {
        //            print("received : ", (char)comRead());
        //        }
    }
}

// Hardware configuration
void deviceConfig() {
    // connect interfaces
    spiBusLayer.connectToInterface(&hspi4);
    spiBusPanel.connectToInterface(&hspi1);
    spiBusEEPROM.connectToInterface(&hspi6);

    i2cBusTouch.connectToInterface(&hi2c1);
    i2cBusIOExp.connectToInterface(&hi2c2);
    i2cBusPanel1.connectToInterface(&hi2c4);
    i2cBusPanel2.connectToInterface(&hi2c3);

    i2cBusPanel1A.connectToBus(&i2cBusPanel1);
    i2cBusPanel1A.connectToMultiplexer(&busMultiplexerPanelA, 0);

    i2cBusPanel1B.connectToBus(&i2cBusPanel1);
    i2cBusPanel1B.connectToMultiplexer(&busMultiplexerPanelA, 1);

    i2cBusTouchA.connectToBus(&i2cBusTouch);
    i2cBusTouchA.connectToMultiplexer(&busMultiplexerControl, 0);

    i2cBusTouchB.connectToBus(&i2cBusTouch);
    i2cBusTouchB.connectToMultiplexer(&busMultiplexerControl, 1);

    i2cBusPanel1H.connectToBus(&i2cBusPanel1);
    i2cBusPanel1H.connectToMultiplexer(&busMultiplexerPanelA, 7);

    // connect multiplexer
    busMultiplexerControl.configurate(&i2cBusTouch, 0);
    busMultiplexerPanelA.configurate(&i2cBusPanel1, 0);

    // connect devices
    // setup devices
    touchControl.resize(2);
    touchPanelA.resize(2);
    ledDriverA.resize(1);
    ledDriverB.resize(0);

    // touchPanelB.resize(0);

    touchPanelA[0].configurate(&i2cBusPanel1A);
    touchPanelA[1].configurate(&i2cBusPanel1B);

    touchControl[0].configurate(&i2cBusTouchA);
    touchControl[1].configurate(&i2cBusTouchB);

    ioExpander.configurate(&i2cBusIOExp, 0);

    ledDriverA[0].configurate(&i2cBusPanel1H, 0);
    // ledDriverB[0].configurate(&i2cBusPanel1H, 0);

    adcA.configurate(&spiBusPanel, 12, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin);
    adcB.configurate(&spiBusPanel, 12, Panel_2_CS_GPIO_Port, Panel_2_CS_Pin);

    eeprom.configurate(&spiBusEEPROM, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin);

    // device Mananger

    deviceManager.addBus(&spiBusLayer);
    deviceManager.addBus(&spiBusPanel);
    deviceManager.addBus(&spiBusEEPROM);

    deviceManager.addBus(&i2cBusTouch);
    deviceManager.addBus(&i2cBusIOExp);
    deviceManager.addBus(&i2cBusPanel1);
    deviceManager.addBus(&i2cBusPanel2);

    deviceManager.addBus(&i2cBusPanel1A);
    deviceManager.addBus(&i2cBusPanel1B);
    deviceManager.addBus(&i2cBusPanel1H);

    deviceManager.addBus(&i2cBusTouchA);
    deviceManager.addBus(&i2cBusTouchB);

    deviceManager.addDevice(&touchPanelA[0]);
    deviceManager.addDevice(&touchPanelA[1]);
    deviceManager.addDevice(&touchControl[0]);
    deviceManager.addDevice(&touchControl[1]);

    deviceManager.addDevice(&busMultiplexerControl);
    deviceManager.addDevice(&ioExpander);

    deviceManager.addDevice(&adcA);
    deviceManager.addDevice(&adcB);

    deviceManager.addDevice(&ledDriverA[0]);

    deviceManager.addDevice(&eeprom);

    // println(*(deviceManager.report()));
}

//////////////LAYER SPECIFIC HANDLING////////////

uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount) {
    // println("send i setting ", amount);

    return layerCom.sendSetting(layerId, moduleId, settingsId, amount);
}
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount) {
    // println("send f setting ", amount);

    return layerCom.sendSetting(layerId, moduleId, settingsId, amount);
}
uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    return layerCom.sendCreatePatchInOut(layerId, outputId, inputId, amount);
}
uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    return layerCom.sendUpdatePatchInOut(layerId, outputId, inputId, amount);
}
uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId) {
    return layerCom.sendDeletePatchInOut(layerId, outputId, inputId);
}

uint8_t sendDeleteAllPatches(uint8_t layerId) {
    return layerCom.sendDeleteAllPatches(layerId);
}

uint8_t sendRequestUIData() {
    return layerCom.sendRequestUIData();
}

//////////////TEMPERATURE////////////

void temperature() {

    static unsigned int adc_v;
    static double adcx;

    adc_v = HAL_ADC_GetValue(&hadc3);
    adcx = (110.0 - 30.0) / (*(unsigned short *)(0x1FF1E840) - *(unsigned short *)(0x1FF1E820));
    globalSettings.temperature = (uint32_t)round(adcx * (adc_v - *(unsigned short *)(0x1FF1E820)) + 30);

    if (HAL_ADC_PollForConversion(&hadc3, 100) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com
    if (hspi == &hspi4) {
        layerCom.decodeCurrentInBuffer();
        FlagHandler::receiveDMARunning = false;
    }
}

//////////////MIDI////////////
inline void midiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    liveData.keyPressed(channel, note, velocity);
}
inline void midiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    liveData.keyReleased(channel, note);
}
inline void midiControlChange(uint8_t channel, uint8_t cc, uint8_t value) {

    liveData.controlChange(channel, cc, value);
}
inline void midiPitchBend(uint8_t channel, int value) {
    liveData.controlChange(channel, midi::PitchBend, value);
}

inline void midiAfterTouch(uint8_t channel, byte value) {

    liveData.controlChange(channel, midi::AfterTouchChannel, value);
}
inline void midiClock() {
    liveData.midiClockTick();
}
inline void midiStart() {
    liveData.receivedStart();
}
inline void midiStop() {
    liveData.receivedStop();
}
inline void midiContinue() {
    liveData.receivedContinue();
}
inline void midiReset() {
    liveData.receivedReset();
}
inline void receivedSPP(unsigned int spp) {
    liveData.receivedMidiSongPosition(spp);
}
void midiConfig() {
    mididevice.setHandleNoteOn(midiNoteOn);
    mididevice.setHandleNoteOff(midiNoteOff);
    mididevice.setHandleControlChange(midiControlChange);
    mididevice.setHandleClock(midiClock);
    mididevice.setHandlePitchBend(midiPitchBend);
    mididevice.setHandleAfterTouchChannel(midiAfterTouch);
    mididevice.setHandleStart(midiStart);
    mididevice.setHandleStop(midiStop);
    mididevice.setHandleContinue(midiContinue);
    mididevice.setHandleSystemReset(midiReset);
    mididevice.setHandleSongPosition(receivedSPP);
}

void receiveFromRenderChip(uint8_t layer, uint8_t chip) {
    while (layerCom.beginReceiveTransmission(layer, chip)) {
        mididevice.read();
        liveData.serviceRoutine();
        FlagHandler::handleFlags();
    }

    FlagHandler::renderChipAwaitingData[layer][chip] = false;
}

//////////////Callback////////////
// SPI Callback
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // InterChip Com
    if (hspi == &hspi4) {
        if (FlagHandler::interChipSend_DMA_Started == 1) {
            FlagHandler::interChipSend_DMA_Started = 0;
            // FlagHandler::interChipSend_DMA_Finished = 1;
            layerCom.sendTransmissionSuccessfull();
            // close ChipSelectLine

            // TODO check CS  selection

            HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);
        }
    }
}

// EXTI Callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    if (pin & GPIO_PIN_12) { // ioExpander -> encoder
        FlagHandler::Control_Encoder_Interrupt = true;
        FlagHandler::Control_Encoder_Interrupt_Timer = 0;
    }
    if (pin & GPIO_PIN_11) { // EOC
        FlagHandler::Panel_0_EOC_Interrupt = true;
        FlagHandler::Panel_1_EOC_Interrupt = true;
    }
    if (pin & GPIO_PIN_14) { // Control Touch
        FlagHandler::Control_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_2) { //  Touch
        FlagHandler::Panel_0_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_5) { //  Touch
        FlagHandler::Panel_1_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_3) { // Layer B Chip 0
        if (FlagHandler::renderChip_State[1][0] == NOTCONNECT) {
            FlagHandler::renderChip_State[1][0] = READY;
        }
        else if (FlagHandler::renderChip_State[1][0] == WAITFORRESPONSE) {
            if (FlagHandler::renderChipAwaitingData[1][0])
                receiveFromRenderChip(1, 0);
            FlagHandler::renderChip_StateTimeout[1][0] = 0;
            FlagHandler::renderChip_State[1][0] = READY;
        }
    }

    if (pin & GPIO_PIN_4) { // Layer B Chip 1
        if (FlagHandler::renderChip_State[1][1] == NOTCONNECT) {
            FlagHandler::renderChip_State[1][1] = READY;
        }
        else if (FlagHandler::renderChip_State[1][1] == WAITFORRESPONSE) {
            if (FlagHandler::renderChipAwaitingData[1][1])
                receiveFromRenderChip(1, 1);
            FlagHandler::renderChip_StateTimeout[1][1] = 0;
            FlagHandler::renderChip_State[1][1] = READY;
        }
    }

    if (pin & GPIO_PIN_6) { // Layer A Chip 0

        if (FlagHandler::renderChip_State[0][0] == NOTCONNECT) {
            FlagHandler::renderChip_State[0][0] = READY;
        }
        else if (FlagHandler::renderChip_State[0][0] == WAITFORRESPONSE) {
            if (FlagHandler::renderChipAwaitingData[0][0])
                receiveFromRenderChip(0, 0);
            FlagHandler::renderChip_StateTimeout[0][0] = 0;
            FlagHandler::renderChip_State[0][0] = READY;
        }
    }

    if (pin & GPIO_PIN_7) { // Layer A Chip 1
        if (FlagHandler::renderChip_State[0][1] == NOTCONNECT) {
            FlagHandler::renderChip_State[0][1] = READY;
        }
        else if (FlagHandler::renderChip_State[0][1] == WAITFORRESPONSE) {
            if (FlagHandler::renderChipAwaitingData[0][1])
                receiveFromRenderChip(0, 1);
            FlagHandler::renderChip_StateTimeout[0][1] = 0;
            FlagHandler::renderChip_State[0][1] = READY;
        }
    }
}

// USB Connect detection
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
    /* Prevent unused argument(s) compilation warning */
    if (hpcd->Instance == hpcd_USB_OTG_HS.Instance) { // HS Connected
        FlagHandler::USB_FS_CONNECTED = true;
    }

    if (hpcd->Instance == hpcd_USB_OTG_HS.Instance) { // FS Connected
        FlagHandler::USB_HS_CONNECTED = true;
    }
}

// TIM Callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) { // timer interrupts

    if (htim->Instance == htim5.Instance) {
        liveData.internalClockTick();
    }
    if (htim->Instance == htim4.Instance) {
        FlagHandler::readTemperature = true;
    }
}
