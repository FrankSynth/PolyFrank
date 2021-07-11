#include "polyControl.hpp"

// GUI
extern GUI ui;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern ADC_HandleTypeDef hadc3;

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABufferLayerA[2 * INTERCHIPBUFFERSIZE];
RAM2_DMA volatile uint8_t interChipDMABufferLayerB[2 * INTERCHIPBUFFERSIZE];

// USB
midi::MidiInterface<midiUSB::COMusb> mididevice(MIDIComRead);

// Layer
ID layerId;
Layer layerA(layerId.getNewId());
Layer layerB(layerId.getNewId());

// InterChip Com
COMinterChip layerCom[2];

// function pointers
void initMidi();

uint8_t test = 0;

// function to read MCU temperature and store to globalSettings
void readTemperature();

// poly control init
void PolyControlInit() {
    // calibrate adc for temperature reading
    HAL_ADC_Start(&hadc3);

    // Init for Control and Layer Chips
    initPoly();

    // Prepare Layer
    allLayers.push_back(&layerA);
    allLayers.push_back(&layerB);

    ////////Hardware init////////

    // Enable Layer Board
    HAL_GPIO_WritePin(Layer_Reset_GPIO_Port, Layer_Reset_Pin, GPIO_PIN_SET);

    // Enable Panel Board
    HAL_GPIO_WritePin(Panel_Reset_GPIO_Port, Panel_Reset_Pin, GPIO_PIN_SET);

    // Enable Control Panel Board
    HAL_GPIO_WritePin(Control_Reset_GPIO_Port, Control_Reset_Pin, GPIO_PIN_SET);

    // let the layer start

    HAL_Delay(1000);

    // CheckLayerStatus

    if (FlagHandler::interChipA_State[0] == READY && FlagHandler::interChipB_State[0] == READY) { // Layer A alive
        layerA.LayerState.value = 1;
    }
    else {
        globalSettings.midiLayerAChannel.disable = 1;
        FlagHandler::interChipA_State[0] = DISABLED;
        FlagHandler::interChipB_State[0] = DISABLED;
    }
    if (FlagHandler::interChipA_State[1] == READY && FlagHandler::interChipB_State[1] == READY) { // Layer B alive
        layerB.LayerState.value = 1;
    }
    else {
        globalSettings.midiLayerBChannel.disable = 1;

        FlagHandler::interChipA_State[1] = DISABLED;
        FlagHandler::interChipB_State[1] = DISABLED;
    }

    if (layerA.LayerState.value && layerB.LayerState.value) {
        globalSettings.multiLayer.value = 1;
    }
    else {                                                    // disable multilayerSettings
        liveData.livemodeKeysplit.disable = 1;                // disable Key splitting
        liveData.voiceHandler.livemodeMergeLayer.disable = 1; // disable merge Layers
    }

    if (layerA.LayerState.value == 0 && layerB.LayerState.value == 0) {
        PolyError_Handler("ERROR | FATAL | NO Layer Connected!");
    }

    // init EEPROM
    initPreset();
    updatePresetList(); // read preset List from EEPROM

    // Init Encoder, Touchbuttons,..
    initHID();

    // init UI, Display
    // set default FOCUS
    if (layerA.LayerState.value == 1) {
        newFocus = {0, 0, 0, FOCUSMODULE};
    }
    else if (layerB.LayerState.value == 1) {
        newFocus = {1, 0, 0, FOCUSMODULE};
    }
    ui.Init();

    ////////Sofware init////////
    // init communication to render chip
    if (layerA.LayerState.value == 1) {
        layerCom[0].initOutTransmission(
            std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi4, std::placeholders::_1, std::placeholders::_2),
            (uint8_t *)interChipDMABufferLayerA, 0);
    }
    if (layerB.LayerState.value == 1) {
        layerCom[1].initOutTransmission(
            std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi5, std::placeholders::_1, std::placeholders::_2),
            (uint8_t *)interChipDMABufferLayerB, 1);
    }

    // init midi
    initMidi();

    // load global Settings
    globalSettings.loadGlobalSettings();

    // Sit back and relax for a moment
    HAL_Delay(1000);

    // Reset all Layer to default configuration,
    for (Layer *l : allLayers) {
        l->resetLayer();
    };

    // Say hello
    println("Hi, Frank here!");
    if (layerA.LayerState.value) {
        println("Layer A active");
    }
    if (layerB.LayerState.value) {
        println("Layer B active");
    }

    // And turn the Display on
    HAL_GPIO_WritePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin, GPIO_PIN_SET);

    /* // Temp set Note
    Key startKey = {80, 100, 0, 0, 0, 0};
    liveData.voiceHandler.playNote(startKey); */
}

void PolyControlRun() { // Here the party starts

    elapsedMillis temperatureReadTimout = 0;

    while (1) {

        mididevice.read();

        // Com Receive Buffer
        //        if (comAvailable()) {
        //            print("received : ", (char)comRead());
        //        }

        liveData.serviceRoutine();

        FlagHandler::handleFlags();

        if (getRenderState() == RENDER_DONE) {
            ui.Draw();
            updatePatchLED();
            if (HAL_ADC_PollForConversion(&hadc3, 100) != HAL_OK) {
                Error_Handler();
            }
        }

        if (temperatureReadTimout > 1000) { // read every second
            readTemperature();
            temperatureReadTimout = 0;
        }

        for (uint8_t i = 0; i < 2; i++) {
            layerCom[i].beginSendTransmission();
        }
    }
}

//////////////LAYER SPECIFIC HANDLING////////////

uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount) {
    return layerCom[layerId].sendSetting(moduleId, settingsId, amount);
}
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount) {
    return layerCom[layerId].sendSetting(moduleId, settingsId, amount);
}
uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    return layerCom[layerId].sendCreatePatchInOut(outputId, inputId, amount);
}
uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    return layerCom[layerId].sendUpdatePatchInOut(outputId, inputId, amount);
}
uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId) {
    return layerCom[layerId].sendDeletePatchInOut(outputId, inputId);
}
// uint8_t sendCreatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount, float
// offset) {
//     return layerCom[layerId].sendCreatePatchOutOut(outputOutId, outputInId, amount, offset);
// }
// uint8_t sendUpdatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount, float
// offset) {
//     return layerCom[layerId].sendUpdatePatchOutOut(outputOutId, outputInId, amount, offset);
// }
// uint8_t sendDeletePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId) {
//     return layerCom[layerId].sendDeletePatchOutOut(outputOutId, outputInId);
// }
uint8_t sendDeleteAllPatches(uint8_t layerId) {
    return layerCom[layerId].sendDeleteAllPatches();
}

void readTemperature() {
    static unsigned int adc_v;
    static double adcx;

    adc_v = HAL_ADC_GetValue(&hadc3);
    adcx = (110.0 - 30.0) / (*(unsigned short *)(0x1FF1E840) - *(unsigned short *)(0x1FF1E820));
    globalSettings.temperature = (uint32_t)round(adcx * (adc_v - *(unsigned short *)(0x1FF1E820)) + 30);
}

// SPI Callbacks
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com Layer 1
    if (layerA.LayerState.value) {
        if (hspi == &hspi4) {
            if (FlagHandler::interChipA_DMA_Started[0] == 1) {
                FlagHandler::interChipA_DMA_Started[0] = 0;
                FlagHandler::interChipA_DMA_Finished[0] = 1;
                // close ChipSelectLine
                HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
            }

            if (FlagHandler::interChipB_DMA_Started[0] == 1) {
                FlagHandler::interChipB_DMA_Started[0] = 0;
                FlagHandler::interChipB_DMA_Finished[0] = 1;

                // close ChipSelectLine
                HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
            }
        }
    }

    // second Layer active?
    if (layerB.LayerState.value) {
        if (hspi == &hspi5) {

            if (FlagHandler::interChipA_DMA_Started[1] == 1) {
                FlagHandler::interChipA_DMA_Started[1] = 0;
                FlagHandler::interChipA_DMA_Finished[1] = 1;

                // close ChipSelectLine
                HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
            }

            if (FlagHandler::interChipB_DMA_Started[1] == 1) {
                FlagHandler::interChipB_DMA_Started[1] = 0;
                FlagHandler::interChipB_DMA_Finished[1] = 1;

                // close ChipSelectLine
                HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);
            }
        }
    }
}

// Midi Handling

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
}

// EXTI Callback

void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    if (pin & GPIO_PIN_12) { // ioExpander -> encoder
        FlagHandler::Control_Encoder_Interrupt = true;
    }
    if (pin & GPIO_PIN_11) { // EOC
        FlagHandler::Panel_0_EOC_Interrupt = true;
        FlagHandler::Panel_1_EOC_Interrupt = true;
    }
    if (pin & GPIO_PIN_14) { // Control Touch
        FlagHandler::Control_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_2) { // Control Touch
        FlagHandler::Panel_0_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_5) { // Control Touch
        FlagHandler::Panel_1_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_3) { // Layer 2 Ready 1
        if (FlagHandler::interChipA_State[1] == NOTCONNECT) {
            FlagHandler::interChipA_State[1] = READY;
        }
        else {
            if (FlagHandler::interChipA_State[1] == WAITFORRESPONSE) {
                FlagHandler::interChipA_StateTimeout[1] = 0;
                FlagHandler::interChipA_State[1] = READY;
            }
        }
    }
    if (pin & GPIO_PIN_4) { // Layer 2 Ready 2
        if (FlagHandler::interChipB_State[1] == NOTCONNECT) {
            FlagHandler::interChipB_State[1] = READY;
        }
        else {
            if (FlagHandler::interChipB_State[1] == WAITFORRESPONSE) {
                FlagHandler::interChipB_StateTimeout[1] = 0;
                FlagHandler::interChipB_State[1] = READY;
            }
        }
    }
    if (pin & GPIO_PIN_6) { // Layer 1 Ready 1
        if (FlagHandler::interChipA_State[0] == NOTCONNECT) {
            FlagHandler::interChipA_State[0] = READY;
        }
        else {
            if (FlagHandler::interChipA_State[0] == WAITFORRESPONSE) {
                FlagHandler::interChipA_StateTimeout[0] = 0;
                FlagHandler::interChipA_State[0] = READY;
            }
        }
    }
    if (pin & GPIO_PIN_7) { // Layer 1 Ready 2
        if (FlagHandler::interChipB_State[0] == NOTCONNECT) {
            FlagHandler::interChipB_State[0] = READY;
        }
        else {
            if (FlagHandler::interChipB_State[0] == WAITFORRESPONSE) {
                FlagHandler::interChipB_StateTimeout[0] = 0;
                FlagHandler::interChipB_State[0] = READY;
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) { // internal Clock interrupt
                                                              //  println("timer Interrupt");

    if (htim->Instance == htim5.Instance) {
        liveData.internalClockTick();
    }
}