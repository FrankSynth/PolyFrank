#include "polyControl.hpp"

// GUI
extern GUI ui;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern ADC_HandleTypeDef hadc3;

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABufferLayerA[2 * INTERCHIPBUFFERSIZE];

// USB
midi::MidiInterface<midiUSB::COMusb> mididevice(MIDIComRead);

// Layer
ID layerId;
Layer layerA(layerId.getNewId());
Layer layerB(layerId.getNewId());

// InterChip Com
COMinterChip layerCom;

// function pointers
void initMidi();

// function to read MCU temperature and store to globalSettings
void readTemperature();

// poly control init
void PolyControlInit() { ////////Hardware init////////

    // Enable Layer Board
    HAL_GPIO_WritePin(Layer_Reset_GPIO_Port, Layer_Reset_Pin, GPIO_PIN_SET);

    // Enable Panel Board
    HAL_GPIO_WritePin(Panel_Reset_GPIO_Port, Panel_Reset_Pin, GPIO_PIN_SET);

    // Enable Control Panel Board
    HAL_GPIO_WritePin(Control_Reset_GPIO_Port, Control_Reset_Pin, GPIO_PIN_SET);

    initPoly();

    // calibrate adc for temperature reading
    HAL_ADC_Start(&hadc3);
    FlagHandler::readTemperature_ISR = readTemperature; // registerFunction pointer to ISR

    // Init for Control and Layer Chips

    // Prepare Layer
    allLayers.push_back(&layerA);
    allLayers.push_back(&layerB);

    // let the layer start

    HAL_Delay(50);

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

    // init EEPROM
    initPreset();
    updatePresetList(); // read preset List from EEPROM

    // Init Encoder, Touchbuttons,..
    initHID();

    // init UI, Display
    // set default FOCUS
    if (layerA.layerState.value == 1) {
        newFocus = {0, 0, 0, FOCUSMODULE};
    }
    else if (layerB.layerState.value == 1) {
        newFocus = {1, 0, 0, FOCUSMODULE};
    }
    ui.Init();

    ////////Sofware init////////
    // init communication to render chip
    layerCom.initOutTransmission(
        std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi4, std::placeholders::_1, std::placeholders::_2),
        (uint8_t *)interChipDMABufferLayerA);

    // init midi
    initMidi();

    // load global Settings
    globalSettings.loadGlobalSettings();

    // Reset all Layer to default configuration,
    for (Layer *l : allLayers) {
        l->resetLayer();
    }

    // Reset Poti States
    resetPanelPotis();

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

    /* // Temp set Note
    Key startKey = {80, 100, 0, 0, 0, 0};
    liveData.voiceHandler.playNote(startKey); */
}

void PolyControlRun() { // Here the party starts

    while (1) {

        mididevice.read();

        // Com Receive Buffer
        //        if (comAvailable()) {
        //            print("received : ", (char)comRead());
        //        }

        liveData.serviceRoutine();

        FlagHandler::handleFlags();

        if (getRenderState() == RENDER_DONE) {
            // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
            ui.Draw();
            // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
            renderLED();
        }

        // for (uint8_t i = 0; i < 2; i++) {
        layerCom.beginSendTransmission();
        // }
    }
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

void readTemperature() {

    static unsigned int adc_v;
    static double adcx;

    adc_v = HAL_ADC_GetValue(&hadc3);
    adcx = (110.0 - 30.0) / (*(unsigned short *)(0x1FF1E840) - *(unsigned short *)(0x1FF1E820));
    globalSettings.temperature = (uint32_t)round(adcx * (adc_v - *(unsigned short *)(0x1FF1E820)) + 30);

    if (HAL_ADC_PollForConversion(&hadc3, 100) != HAL_OK) {
        Error_Handler();
    }
}

// SPI Callbacks
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

// Midi Handling

inline void midiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    // println(micros(), " - kp: ", note);
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
void initMidi() {
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

// EXTI Callback

void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    println("exti callback ", pin);

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
        else {
            if (FlagHandler::renderChip_State[1][0] == WAITFORRESPONSE) {
                FlagHandler::renderChip_StateTimeout[1][0] = 0;
                FlagHandler::renderChip_State[1][0] = READY;
            }
        }
    }
    if (pin & GPIO_PIN_4) { // Layer B Chip 1
        if (FlagHandler::renderChip_State[1][1] == NOTCONNECT) {
            FlagHandler::renderChip_State[1][1] = READY;
        }
        else {
            if (FlagHandler::renderChip_State[1][1] == WAITFORRESPONSE) {
                FlagHandler::renderChip_StateTimeout[1][1] = 0;
                FlagHandler::renderChip_State[1][1] = READY;
            }
        }
    }
    if (pin & GPIO_PIN_6) { // Layer A Chip 0

        if (FlagHandler::renderChip_State[0][0] == NOTCONNECT) {
            FlagHandler::renderChip_State[0][0] = READY;
        }
        else {
            if (FlagHandler::renderChip_State[0][0] == WAITFORRESPONSE) {
                FlagHandler::renderChip_StateTimeout[0][0] = 0;
                FlagHandler::renderChip_State[0][0] = READY;
            }
        }
    }
    if (pin & GPIO_PIN_7) { // Layer A Chip 1
        if (FlagHandler::renderChip_State[0][1] == NOTCONNECT) {
            FlagHandler::renderChip_State[0][1] = READY;
        }
        else {
            if (FlagHandler::renderChip_State[0][1] == WAITFORRESPONSE) {
                FlagHandler::renderChip_StateTimeout[0][1] = 0;
                FlagHandler::renderChip_State[0][1] = READY;
            }
        }
    }
}

// USB Connect detection

/**
 * @brief  Connection event callback.
 * @param  hpcd PCD handle
 * @retval None
 */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
    /* Prevent unused argument(s) compilation warning */
    if (hpcd->Instance == hpcd_USB_OTG_HS.Instance) { // HS Connected
        FlagHandler::USB_FS_CONNECTED = true;
    }

    if (hpcd->Instance == hpcd_USB_OTG_HS.Instance) { // FS Connected
        FlagHandler::USB_HS_CONNECTED = true;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) { // timer interrupts

    if (htim->Instance == htim5.Instance) {
        liveData.internalClockTick();
    }
    if (htim->Instance == htim4.Instance) {
        FlagHandler::readTemperature = true;
    }
}
