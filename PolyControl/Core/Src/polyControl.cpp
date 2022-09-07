
#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"

#include "midiInterface/MIDIInterface.h"
#include "usbd_midi_if.hpp"

#include <cassert>

// GUI
// extern midi::MidiInterface<midiUSB::COMusb> midiDeviceUSB;
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

i2cVirtualBus i2cBusPanelA;
i2cVirtualBus i2cBusPanelB;
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
TS3A5017D multiplexer(4, ADC_Mult_A_GPIO_Port, ADC_Mult_A_Pin, ADC_Mult_B_GPIO_Port, ADC_Mult_B_Pin);

// LED Driver
std::vector<IS31FL3216> ledDriverA;
std::vector<IS31FL3216> ledDriverB;

M95M01 eeprom;

// Encoder    // 16,17,18 sind EXTI rest IO Expander
rotary encoders[NUMBERENCODERS] = {rotary(4, 3), rotary(12, 11), rotary(14, 13),
                                   rotary(6, 7), rotary(18, 17), rotary(0, 1)};
tactileSwitch switches[NUMBERENCODERS] = {tactileSwitch(5), tactileSwitch(10), tactileSwitch(9),
                                          tactileSwitch(8), tactileSwitch(16), tactileSwitch(2)};

// ADC

// Buffer for InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);

// USB
COMdin MIDIDinRead;
midi::MidiInterface<midiUSB::COMusb> midiDeviceUSB(MIDIComRead);
midi::MidiInterface<COMdin> midiDeviceDIN(MIDIDinRead);

// Layer
ID layerId;
RAM1 Layer layerA(layerId.getNewId());
RAM1 Layer layerB(layerId.getNewId());

extern void polyControlLoop();

extern void midiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
extern void midiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
extern void midiControlChange(uint8_t channel, uint8_t cc, uint8_t value);
extern void midiPitchBend(uint8_t channel, int value);
extern void midiAfterTouch(uint8_t channel, byte value);
extern void midiClock();
extern void midiStart();
extern void midiStop();
extern void midiContinue();
extern void midiReset();
extern void receivedSPP(unsigned int spp);

extern void temperature();

// InterChip Com
COMinterChip layerCom(&spiBusLayer, (uint8_t *)interChipDMAInBuffer, (uint8_t *)interChipDMAOutBuffer);

void midiConfig();
void deviceConfig();

void PolyControlInit() {

    HAL_Delay(200); // wait 200ms for system stabilisation

    // Enable Layer Board
    HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_SET);
    // Enable Panel Board
    HAL_GPIO_WritePin(Panel_RST_GPIO_Port, Panel_RST_Pin, GPIO_PIN_SET);
    // Enable Control Panel Board
    HAL_GPIO_WritePin(Control_RST_GPIO_Port, Control_RST_Pin, GPIO_PIN_SET);

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
    HAL_Delay(200);

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
    HAL_Delay(100);

    // User Interface
    if (layerA.layerState.value == 1) {
        newFocus = {0, 11, 0, FOCUSMODULE};
    }
    else if (layerB.layerState.value == 1) {
        newFocus = {1, 0, 0, FOCUSMODULE};
    }

    HIDConfig();

    for (Layer *l : allLayers) {
        l->resetLayer();
    }

    ui.Init();

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

    HAL_Delay(100);
    // And turn the Display on
    HAL_GPIO_WritePin(Display_EN_GPIO_Port, Display_EN_Pin, GPIO_PIN_SET);
    // turn sound on
    HAL_GPIO_WritePin(Enable_OUTPUT_GPIO_Port, Enable_OUTPUT_Pin, GPIO_PIN_SET);

    // setPanelActive(5);
}

void PolyControlRun() { // Here the party starts

    __HAL_RCC_USB_OTG_HS_ULPI_CLK_SLEEP_DISABLE();
    __HAL_RCC_USB_OTG_FS_ULPI_CLK_SLEEP_DISABLE();
    __HAL_RCC_USB_OTG_HS_CLK_SLEEP_ENABLE();
    __HAL_RCC_USB_OTG_FS_CLK_SLEEP_ENABLE();
    HAL_TIM_Base_Start_IT(&htim16);

    polyControlLoop();
}

// Hardware configuration
void deviceConfig() {
    // connect interfaces
    spiBusLayer.connectToInterface(&hspi1);
    spiBusPanel.connectToInterface(&hspi2);
    spiBusEEPROM.connectToInterface(&hspi6);

    i2cBusTouch.connectToInterface(&hi2c1);
    i2cBusIOExp.connectToInterface(&hi2c2);
    i2cBusPanel1.connectToInterface(&hi2c4);
    i2cBusPanel2.connectToInterface(&hi2c3);

    i2cBusPanelA.connectToBus(&i2cBusPanel1);
    i2cBusPanelA.connectToMultiplexer(&busMultiplexerPanelA, 0);

    // i2cBusPanelB.connectToBus(&i2cBusPanel1);
    // i2cBusPanelB.connectToMultiplexer(&busMultiplexerPanelA, 1);

    i2cBusTouchA.connectToBus(&i2cBusTouch);
    i2cBusTouchA.connectToMultiplexer(&busMultiplexerControl, 0);

    i2cBusTouchB.connectToBus(&i2cBusTouch);
    i2cBusTouchB.connectToMultiplexer(&busMultiplexerControl, 1);

    // i2cBusPanel1H.connectToBus(&i2cBusPanel1);
    // i2cBusPanel1H.connectToMultiplexer(&busMultiplexerPanelA, 7);

    // connect multiplexer
    busMultiplexerControl.configurate(&i2cBusTouch, 0);
    busMultiplexerPanelA.configurate(&i2cBusPanel1, 0);

    // connect devices
    // setup devices
    touchControl.resize(2);
    // touchPanelA.resize(2);
    // ledDriverA.resize(1);
    // ledDriverB.resize(0);

    // touchPanelB.resize(0);

    // touchPanelA[0].configurate(&i2cBusPanelA);
    // touchPanelA[1].configurate(&i2cBusPanelB);

    touchControl[0].configurate(&i2cBusTouchA);
    touchControl[1].configurate(&i2cBusTouchB);

    ioExpander.configurate(&i2cBusIOExp, 0);

    // ledDriverA[0].configurate(&i2cBusPanel1H, 0);
    // ledDriverB[0].configurate(&i2cBusPanel1H, 0);

    adcA.configurate(&spiBusPanel, 12, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin);
    adcB.configurate(&spiBusPanel, 12, Panel_2_CS_GPIO_Port, Panel_2_CS_Pin);

    eeprom.configurate(&spiBusEEPROM, Control_EEPROM_CS_GPIO_Port, Control_EEPROM_CS_Pin);

    // device Mananger

    deviceManager.addBus(&spiBusLayer);
    deviceManager.addBus(&spiBusPanel);
    deviceManager.addBus(&spiBusEEPROM);

    deviceManager.addBus(&i2cBusTouch);
    deviceManager.addBus(&i2cBusIOExp);
    deviceManager.addBus(&i2cBusPanel1);
    // deviceManager.addBus(&i2cBusPanel2);

    // deviceManager.addBus(&i2cBusPanel1A);
    // deviceManager.addBus(&i2cBusPanel1B);
    // deviceManager.addBus(&i2cBusPanel1H);

    deviceManager.addBus(&i2cBusTouchA);
    deviceManager.addBus(&i2cBusTouchB);

    // deviceManager.addDevice(&touchPanelA[0]);
    // deviceManager.addDevice(&touchPanelA[1]);
    deviceManager.addDevice(&touchControl[0]);
    deviceManager.addDevice(&touchControl[1]);

    deviceManager.addDevice(&busMultiplexerControl);
    deviceManager.addDevice(&ioExpander);

    deviceManager.addDevice(&adcA);
    deviceManager.addDevice(&adcB);

    // deviceManager.addDevice(&ledDriverA[0]);

    deviceManager.addDevice(&eeprom);

    // println(*(deviceManager.report()));
}

void midiConfig() {
    midiDeviceUSB.setHandleNoteOn(midiNoteOn);
    midiDeviceUSB.setHandleNoteOff(midiNoteOff);
    midiDeviceUSB.setHandleControlChange(midiControlChange);
    midiDeviceUSB.setHandleClock(midiClock);
    midiDeviceUSB.setHandlePitchBend(midiPitchBend);
    midiDeviceUSB.setHandleAfterTouchChannel(midiAfterTouch);
    midiDeviceUSB.setHandleStart(midiStart);
    midiDeviceUSB.setHandleStop(midiStop);
    midiDeviceUSB.setHandleContinue(midiContinue);
    midiDeviceUSB.setHandleSystemReset(midiReset);
    midiDeviceUSB.setHandleSongPosition(receivedSPP);

    midiDeviceDIN.setHandleNoteOn(midiNoteOn);
    midiDeviceDIN.setHandleNoteOff(midiNoteOff);
    midiDeviceDIN.setHandleControlChange(midiControlChange);
    midiDeviceDIN.setHandleClock(midiClock);
    midiDeviceDIN.setHandlePitchBend(midiPitchBend);
    midiDeviceDIN.setHandleAfterTouchChannel(midiAfterTouch);
    midiDeviceDIN.setHandleStart(midiStart);
    midiDeviceDIN.setHandleStop(midiStop);
    midiDeviceDIN.setHandleContinue(midiContinue);
    midiDeviceDIN.setHandleSystemReset(midiReset);
    midiDeviceDIN.setHandleSongPosition(receivedSPP);
}
