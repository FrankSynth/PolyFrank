
#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"

#include "midiInterface/MIDIInterface.h"
#include "usbd_midi_if.hpp"

#include <cassert>

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
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);

// USB
midi::MidiInterface<midiUSB::COMusb> mididevice(MIDIComRead);

// Layer
ID layerId;
RAM1 Layer layerA(layerId.getNewId());
RAM1 Layer layerB(layerId.getNewId());

void sendRequestAllUIData();
void receiveFromRenderChip(uint8_t layer, uint8_t chip);
void checkLayerRequests();

// InterChip Com
COMinterChip layerCom(&spiBusLayer, (uint8_t *)interChipDMAInBuffer, (uint8_t *)interChipDMAOutBuffer);

void midiConfig();
void temperature();
void deviceConfig();

void PolyControlInit() {

    // Enable Layer Board
    HAL_GPIO_WritePin(Layer_Reset_GPIO_Port, Layer_Reset_Pin, GPIO_PIN_SET);
    // Enable Panel Board
    HAL_GPIO_WritePin(Panel_Reset_GPIO_Port, Panel_Reset_Pin, GPIO_PIN_SET);
    // Enable Control Panel Board
    HAL_GPIO_WritePin(Control_Reset_GPIO_Port, Control_Reset_Pin, GPIO_PIN_SET);

    initWavetables();

    // Layer
    allLayers.push_back(&layerA);
    allLayers.push_back(&layerB);

    // empty buffers
    uint32_t emptyData = 0;
    fastMemset(&emptyData, (uint32_t *)interChipDMAInBuffer, 2 * (INTERCHIPBUFFERSIZE + 4) / 4);
    fastMemset(&emptyData, (uint32_t *)interChipDMAOutBuffer, 2 * (INTERCHIPBUFFERSIZE + 4) / 4);

    initPoly();

    // let the layer start
    HAL_Delay(500);

    // Device Configuration
    deviceConfig();

    // Preset
    updatePresetList();                  // read preset List from EEPROM
    globalSettings.loadGlobalSettings(); // load global Settings

    // CheckLayerStatus
    if (layerCom.chipState[0][0] == CHIP_READY && layerCom.chipState[0][1] == CHIP_READY) { // Layer A alive
        layerA.layerState.value = 1;
    }
    else {
        globalSettings.midiLayerAChannel.disable = 1;
        layerCom.chipState[0][0] = CHIP_DISABLED;
        layerCom.chipState[0][1] = CHIP_DISABLED;
    }

    if (layerCom.chipState[1][0] == CHIP_READY && layerCom.chipState[1][1] == CHIP_READY) { // Layer B alive
        layerB.layerState.value = 1;
    }
    else {
        globalSettings.midiLayerBChannel.disable = 1;
        layerCom.chipState[1][0] = CHIP_DISABLED;
        layerCom.chipState[1][1] = CHIP_DISABLED;
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

    HIDConfig();
    ui.Init();

    for (Layer *l : allLayers) {
        l->resetLayer();
    }

    HAL_Delay(100);

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

void PolyControlRun() { // Here the party starts

    elapsedMicros timer;

    while (1) {

        mididevice.read();
        liveData.serviceRoutine();
        FlagHandler::handleFlags();

        layerCom.beginSendTransmission();

        checkLayerRequests();

        if (getRenderState() == RENDER_DONE) {
            // timer = 0;
            ui.Draw();
            // println("ui draw:", timer);
            renderLED();
            sendRequestAllUIData();
        }
    }
}

void checkLayerRequests() {

    if (!layerCom.sentRequestUICommand)
        return;

    if (layerCom.singleChipRequested) {
        if (layerCom.chipState[layerCom.receiveLayer][layerCom.receiveChip] == CHIP_DATAREADY) {
            receiveFromRenderChip(layerCom.receiveLayer, layerCom.receiveChip);
        }
    }
    else {
        uint32_t layer = 0xFF;
        uint32_t chip = 0xFF;
        for (int i = 0; i < 2; i++)
            for (int v = 0; v < 2; v++)
                if (allLayers[i]->layerState.value) {
                    if (layerCom.requestState[i][v] == RQ_REQUESTDATA) {
                        layer = i;
                        chip = v;
                        i = 2;
                        v = 2;
                    }
                }

        if (layer == 0xFF || chip == 0xFF) {
            return;
        }
        layerCom.sendRequestUIData(layer, chip);
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
    return layerCom.sendSetting(layerId, moduleId, settingsId, amount);
}
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount) {
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

/**
 * @brief retreive data from other chips
 *
 * @param layer layer to retreive
 * @param chip chip to retreive
 */
void receiveFromRenderChip(uint8_t layer, uint8_t chip) {
    while (layerCom.beginReceiveTransmission(layer, chip) != BUS_OK) {
    }
}

void sendRequestAllUIData() {
    if (layerCom.sentRequestUICommand == true) {
        println("not all data received yet");
        return;
    }

    for (int i = 0; i < 2; i++)
        for (int v = 0; v < 2; v++)
            if (allLayers[i]->layerState.value) {
                layerCom.requestState[i][v] = RQ_REQUESTDATA;
            }
    layerCom.sentRequestUICommand = true;
}

void setCSLine(uint8_t layer, uint8_t chip, GPIO_PinState state) {
    if (layer) {
        if (chip)
            HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, state);
        else
            HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, state);
    }
    else {
        if (chip)
            HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, state);
        else
            HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, state);
    }
}

//////////////Callback////////////
// SPI Callback

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // InterChip Com
    if (hspi == layerCom.spi->hspi) {
        layerCom.spi->callTxComplete();
        // close ChipSelectLine

        if (layerA.layerState.value) {
            HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
        }
        if (layerB.layerState.value) {
            HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);
        }
    }
    else if (hspi == spiBusEEPROM.hspi) {
        spiBusEEPROM.callTxComplete();
    }
    else if (hspi == spiBusPanel.hspi) {
        spiBusPanel.callTxComplete();
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com
    if (hspi == layerCom.spi->hspi) {
        if (layerCom.requestSize) {
            layerCom.requestSize = false;
            layerCom.spi->callRxComplete();
        }
        else {
            layerCom.chipState[layerCom.receiveLayer][layerCom.receiveChip] = CHIP_DATASENT;
            layerCom.requestState[layerCom.receiveLayer][layerCom.receiveChip] = RQ_READY;
            layerCom.singleChipRequested = false;

            if (layerCom.sentRequestUICommand && layerCom.requestState[0][0] == RQ_READY &&
                layerCom.requestState[0][1] == RQ_READY && layerCom.requestState[1][0] == RQ_READY &&
                layerCom.requestState[1][1] == RQ_READY) {
                layerCom.sentRequestUICommand = false;
            }

            setCSLine(layerCom.receiveLayer, layerCom.receiveChip, GPIO_PIN_SET);

            layerCom.decodeCurrentInBuffer();
            layerCom.spi->callRxComplete();
        }
    }
    else if (hspi == spiBusEEPROM.hspi) {
        spiBusEEPROM.callRxComplete();
    }
    else if (hspi == spiBusPanel.hspi) {
        spiBusPanel.callRxComplete();
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == spiBusPanel.hspi) {
        spiBusPanel.callRxComplete();
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == spiBusPanel.hspi) {
        PolyError_Handler("spi error");
        spiBusPanel.callRxComplete();
    }
}

// EXTI Callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    if (pin & GPIO_PIN_12) { // ioExpander -> encoder
        FlagHandler::Control_Encoder_Interrupt = true;
        FlagHandler::Control_Encoder_Interrupt_Timer = 0;
    }
    if (pin & GPIO_PIN_11) { // EOC
        if (!HAL_GPIO_ReadPin(Panel_1_EOC_GPIO_Port, Panel_1_EOC_Pin)) {
            FlagHandler::Panel_0_EOC_Interrupt = true;
            FlagHandler::Panel_1_EOC_Interrupt = true;
        }
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
        if (layerCom.chipState[1][0] == CHIP_DATASENT) {
            layerCom.chipState[1][0] = CHIP_READY;
            layerCom.chipStateTimeout[1][0] = 0;
        }
        else if (layerCom.chipState[1][0] == CHIP_WAITFORDATA) {
            layerCom.chipState[1][0] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[1][0] = 0;
        }
        else if (layerCom.chipState[1][0] == CHIP_NOTINIT) {
            layerCom.chipState[1][0] = CHIP_READY;
        }
    }

    if (pin & GPIO_PIN_4) { // Layer B Chip 1
        if (layerCom.chipState[1][1] == CHIP_DATASENT) {
            layerCom.chipState[1][1] = CHIP_READY;
            layerCom.chipStateTimeout[1][1] = 0;
        }
        else if (layerCom.chipState[1][1] == CHIP_WAITFORDATA) {
            layerCom.chipState[1][1] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[1][1] = 0;
        }
        else if (layerCom.chipState[1][1] == CHIP_NOTINIT) {
            layerCom.chipState[1][1] = CHIP_READY;
        }
    }

    if (pin & GPIO_PIN_7) { // Layer A Chip 1
        if (layerCom.chipState[0][1] == CHIP_DATASENT) {
            layerCom.chipState[0][1] = CHIP_READY;
            layerCom.chipStateTimeout[0][1] = 0;
        }
        else if (layerCom.chipState[0][1] == CHIP_WAITFORDATA) {
            layerCom.chipState[0][1] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[0][1] = 0;
        }
        else if (layerCom.chipState[0][1] == CHIP_NOTINIT) {
            layerCom.chipState[0][1] = CHIP_READY;
        }
    }

    if (pin & GPIO_PIN_6) { // Layer A Chip 0
        if (layerCom.chipState[0][0] == CHIP_DATASENT) {
            layerCom.chipState[0][0] = CHIP_READY;
            layerCom.chipStateTimeout[0][0] = 0;
        }
        else if (layerCom.chipState[0][0] == CHIP_WAITFORDATA) {
            layerCom.chipState[0][0] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[0][0] = 0;
        }
        else if (layerCom.chipState[0][0] == CHIP_NOTINIT) {
            layerCom.chipState[0][0] = CHIP_READY;
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
