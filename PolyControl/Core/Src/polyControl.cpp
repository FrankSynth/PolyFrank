
#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"
#include "humanInterface/hidStartup.hpp"

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

extern USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_HandleTypeDef hUsbDeviceHS;

// List of all busObjects
spiBus spiBusEEPROM;
spiBus spiBusLayer;
spiBus spiBusPanel;

i2cBus i2cBusControl;
i2cBus i2cBusIOExp;
i2cBus i2cBusPanelA;
i2cBus i2cBusPanelB;

i2cVirtualBus i2cVBusPanelA0;
i2cVirtualBus i2cVBusPanelA1;
i2cVirtualBus i2cVBusPanelA2;
i2cVirtualBus i2cVBusPanelA3;
i2cVirtualBus i2cVBusPanelA4;

i2cVirtualBus i2cVBusPanelB0;
i2cVirtualBus i2cVBusPanelB1;
i2cVirtualBus i2cVBusPanelB2;
i2cVirtualBus i2cVBusPanelB3;
i2cVirtualBus i2cVBusPanelB4;

i2cVirtualBus i2cVBusControl0;
i2cVirtualBus i2cVBusControl1;

// List of all devices
// Control Touch devices
std::vector<AT42QT2120> touchControl;

//  Panel Touch devices
std::vector<AT42QT2120> touchPanelA;
std::vector<AT42QT2120> touchPanelB;

// LedDriver devices

RAM2_DMA uint8_t ledDataControl[25]; // data + 1 command byte
RAM2_DMA uint8_t ledDataA[2][74];    // 36*2 + 2 command byte
RAM3_DMA uint8_t ledDataB[2][74];    // 36*2 + 2 command byte

IS32FL3237 ledDriver[2][2];
IS31FL3205 ledDriverControl;

// I2C BusMultiplexer
PCA9548 busMultiplexerControl;
PCA9548 busMultiplexerPanelA;
PCA9548 busMultiplexerPanelB;

// IO Expander
PCA9555 ioExpander;

//  ADC
RAM2_DMA ALIGN_32BYTES(uint32_t commandA[12]);
RAM2 ALIGN_32BYTES(uint32_t adcDataA[12]);
RAM2_DMA ALIGN_32BYTES(uint32_t commandB[12]);
RAM2 ALIGN_32BYTES(uint32_t adcDataB[12]);

MAX11128 adcA;
MAX11128 adcB;
TS3A5017D multiplexer(4, ADC_Mult_A_GPIO_Port, ADC_Mult_A_Pin, ADC_Mult_B_GPIO_Port, ADC_Mult_B_Pin);

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

    HAL_Delay(800); // wait 200ms for system stabilisation

    // Say hello
    println("\n\nINFO || Hi, Frank here! Lets get everything ready...");

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
    HAL_Delay(350);

    // Device Configuration
    deviceConfig();

    // clearEEPROM(); // ENABLE ONLY IF NEEDED!!!!

    adcA.setup();
    adcB.setup();

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

    // Init Hardware User Interface
    HIDConfig();

    for (Layer *l : allLayers) {
        l->resetLayer();
    }

    // Graphical User Interface
    ui.Init();

    // Midi configuration
    midiConfig();

    if (layerA.layerState.value) {
        println("INFO || Layer A active");
    }
    if (layerB.layerState.value) {
        println("INFO || Layer B active");
    }

    println("INFO || ... everything looks fine! Let the party start");
    println("INFO || For help type -h");

    HAL_Delay(50);
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

    i2cBusControl.connectToInterface(&hi2c1);
    i2cBusIOExp.connectToInterface(&hi2c2);
    i2cBusPanelA.connectToInterface(&hi2c3);
    i2cBusPanelB.connectToInterface(&hi2c4);

    // VirtualBus
    i2cVBusPanelA0.connectToBus(&i2cBusPanelA);
    i2cVBusPanelA1.connectToBus(&i2cBusPanelA);
    i2cVBusPanelA2.connectToBus(&i2cBusPanelA);
    i2cVBusPanelA3.connectToBus(&i2cBusPanelA);
    i2cVBusPanelA4.connectToBus(&i2cBusPanelA);

    i2cVBusPanelB0.connectToBus(&i2cBusPanelB);
    i2cVBusPanelB1.connectToBus(&i2cBusPanelB);
    i2cVBusPanelB2.connectToBus(&i2cBusPanelB);
    i2cVBusPanelB3.connectToBus(&i2cBusPanelB);
    i2cVBusPanelB4.connectToBus(&i2cBusPanelB);

    //-> connect Multiplexer
    i2cVBusPanelA0.connectToMultiplexer(&busMultiplexerPanelA, 0);
    i2cVBusPanelA1.connectToMultiplexer(&busMultiplexerPanelA, 1);
    i2cVBusPanelA2.connectToMultiplexer(&busMultiplexerPanelA, 2);
    i2cVBusPanelA3.connectToMultiplexer(&busMultiplexerPanelA, 3);
    i2cVBusPanelA4.connectToMultiplexer(&busMultiplexerPanelA, 4);

    i2cVBusPanelB0.connectToMultiplexer(&busMultiplexerPanelB, 0);
    i2cVBusPanelB1.connectToMultiplexer(&busMultiplexerPanelB, 1);
    i2cVBusPanelB2.connectToMultiplexer(&busMultiplexerPanelB, 2);
    i2cVBusPanelB3.connectToMultiplexer(&busMultiplexerPanelB, 3);
    i2cVBusPanelB4.connectToMultiplexer(&busMultiplexerPanelB, 4);

    i2cVBusControl0.connectToBus(&i2cBusControl);
    i2cVBusControl1.connectToBus(&i2cBusControl);

    i2cVBusControl0.connectToMultiplexer(&busMultiplexerControl, 0);
    i2cVBusControl1.connectToMultiplexer(&busMultiplexerControl, 1);

    // connect multiplexer
    busMultiplexerControl.configurate(&i2cBusControl, 0);
    busMultiplexerPanelA.configurate(&i2cBusPanelA, 0);
    busMultiplexerPanelB.configurate(&i2cBusPanelB, 0);

    // connect devices
    // setup devices

    touchControl.resize(2);
    touchPanelA.resize(5);
    touchPanelB.resize(5);

    touchControl[0].configurate(&i2cVBusControl0);
    touchControl[1].configurate(&i2cVBusControl1);

    ioExpander.configurate(&i2cBusIOExp, 0);

    // TODO NEW ENABLE
    touchPanelA[0].configurate(&i2cVBusPanelA0);
    touchPanelA[1].configurate(&i2cVBusPanelA1);
    touchPanelA[2].configurate(&i2cVBusPanelA2);
    touchPanelA[3].configurate(&i2cVBusPanelA3);
    touchPanelA[4].configurate(&i2cVBusPanelA4);

    ledDriver[1][0].configurate(&i2cBusPanelB, 0, ledDataB[0]);
    ledDriver[1][1].configurate(&i2cBusPanelB, 3, ledDataB[1]);

    touchPanelB[0].configurate(&i2cVBusPanelB0);
    touchPanelB[1].configurate(&i2cVBusPanelB1);
    touchPanelB[2].configurate(&i2cVBusPanelB2);
    touchPanelB[3].configurate(&i2cVBusPanelB3);
    touchPanelB[4].configurate(&i2cVBusPanelB4);

    ledDriver[0][0].configurate(&i2cBusPanelA, 0, ledDataA[0]);
    ledDriver[0][1].configurate(&i2cBusPanelA, 3, ledDataA[1]);

    ledDriverControl.configurate(&i2cBusControl, 0, ledDataControl);

    adcA.configurate(&spiBusPanel, 12, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin, commandA, adcDataA);
    adcB.configurate(&spiBusPanel, 12, Panel_2_CS_GPIO_Port, Panel_2_CS_Pin, commandB, adcDataB);

    eeprom.configurate(&spiBusEEPROM, Control_EEPROM_CS_GPIO_Port, Control_EEPROM_CS_Pin);

    // device Mananger

    deviceManager.addBus(&spiBusLayer);
    deviceManager.addBus(&spiBusPanel);
    deviceManager.addBus(&spiBusEEPROM);

    deviceManager.addBus(&i2cBusControl);
    deviceManager.addBus(&i2cBusIOExp);
    deviceManager.addBus(&i2cBusPanelA);
    deviceManager.addBus(&i2cBusPanelB);

    deviceManager.addBus(&i2cVBusControl0);
    deviceManager.addBus(&i2cVBusControl1);

    deviceManager.addBus(&i2cVBusPanelA0);
    deviceManager.addBus(&i2cVBusPanelA1);
    deviceManager.addBus(&i2cVBusPanelA2);
    deviceManager.addBus(&i2cVBusPanelA3);
    deviceManager.addBus(&i2cVBusPanelA4);

    deviceManager.addBus(&i2cVBusPanelB0);
    deviceManager.addBus(&i2cVBusPanelB1);
    deviceManager.addBus(&i2cVBusPanelB2);
    deviceManager.addBus(&i2cVBusPanelB3);
    deviceManager.addBus(&i2cVBusPanelB4);

    deviceManager.addDevice(&busMultiplexerControl);
    deviceManager.addDevice(&busMultiplexerPanelA);
    deviceManager.addDevice(&busMultiplexerPanelB);

    deviceManager.addDevice(&ioExpander);

    deviceManager.addDevice(&touchControl[0]);
    deviceManager.addDevice(&touchControl[1]);

    deviceManager.addDevice(&ledDriverControl);
    deviceManager.addDevice(&ledDriver[0][0]);
    deviceManager.addDevice(&ledDriver[0][1]);

    deviceManager.addDevice(&ledDriver[1][0]);
    deviceManager.addDevice(&ledDriver[1][1]);

    deviceManager.addDevice(&touchPanelA[0]);
    deviceManager.addDevice(&touchPanelA[1]);
    deviceManager.addDevice(&touchPanelA[2]);
    deviceManager.addDevice(&touchPanelA[3]);
    deviceManager.addDevice(&touchPanelA[4]);

    deviceManager.addDevice(&touchPanelB[0]);
    deviceManager.addDevice(&touchPanelB[1]);
    deviceManager.addDevice(&touchPanelB[2]);
    deviceManager.addDevice(&touchPanelB[3]);
    deviceManager.addDevice(&touchPanelB[4]);

    deviceManager.addDevice(&adcA);
    deviceManager.addDevice(&adcB);

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
