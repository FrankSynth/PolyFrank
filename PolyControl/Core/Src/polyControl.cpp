#include "polyControl.hpp"

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

// poly control init
void PolyControlInit() {
    ////////Layer init////////

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

    // init EEPROM
    initPreset();
    updatePresetList(); // read preset List from EEPROM

    // Init Encoder, Touchbuttons,..
    initHID();

    // init UI, Display
    ui.Init();

    // Set UI Default Focus
    newFocus = {0, allLayers[0]->steiner.id, 0, FOCUSMODULE};

    ////////Sofware init////////
    // init communication to render chip
    layerCom[0].initOutTransmission(
        std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi4, std::placeholders::_1, std::placeholders::_2),
        (uint8_t *)interChipDMABufferLayerA, 0);

    if (globalSettings.amountLayers.value) {
        layerCom[1].initOutTransmission(
            std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi5, std::placeholders::_1, std::placeholders::_2),
            (uint8_t *)interChipDMABufferLayerB, 1);
    }

    // Reset all Layer to default configuration,
    allLayers[0]->resetLayer();
    // for (Layer *l : allLayers) {
    //     l->resetLayer();
    // };

    // init midi
    initMidi();

    // load global Settings
    globalSettings.loadGlobalSettings();

    // Sit back and relax for a moment
    HAL_Delay(1000);

    // Say hello
    println("Hi, Frank here!");

    // And turn the Display on
    HAL_GPIO_WritePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin, GPIO_PIN_SET);
}

void PolyControlRun() { // Here the party starts

    while (1) {

        mididevice.read();

        liveData.serviceRoutine();

        FlagHandler::handleFlags();
        if (getRenderState() == RENDER_DONE) {
            ui.Draw();
            // TODO updatePatchLED an wenn chip dran updatePatchLED();
        }

        layerCom[0].beginSendTransmission();
        // layerCom[1].beginSendTransmission();
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
uint8_t sendCreatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
    return layerCom[layerId].sendCreatePatchOutOut(outputOutId, outputInId, amount, offset);
}
uint8_t sendUpdatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
    return layerCom[layerId].sendUpdatePatchOutOut(outputOutId, outputInId, amount, offset);
}
uint8_t sendDeletePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId) {
    return layerCom[layerId].sendDeletePatchOutOut(outputOutId, outputInId);
}
uint8_t sendDeleteAllPatches(uint8_t layerId) {
    return layerCom[layerId].sendDeleteAllPatches();
}

// SPI Callbacks
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com Layer 1
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

    // second Layer active?
    if (globalSettings.amountLayers.value) {

        // layer2 spi
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
inline void midiClock() {
    liveData.clockTick();
}
void initMidi() {
    mididevice.setHandleNoteOn(midiNoteOn);
    mididevice.setHandleNoteOff(midiNoteOff);
    mididevice.setHandleControlChange(midiControlChange);
    mididevice.setHandleClock(midiClock);
}

// EXTI Callback

void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    // println(pin);

    switch (pin) {
        case GPIO_PIN_12: FlagHandler::Control_Encoder_Interrupt = true; break; // ioExpander -> encoder
        case GPIO_PIN_11: FlagHandler::Panel_0_EOC_Interrupt = true; break;     // Panel 1 -> EOC
        case GPIO_PIN_2: FlagHandler::Control_Touch_Interrupt = true; break;    // touch

        case GPIO_PIN_3: // Layer 2 Ready 1
            if (FlagHandler::interChipA_State[1] == 0) {
                FlagHandler::interChipA_State[1] = 2;
                FlagHandler::interChipA_StateTimeout[1] = 0;
            }
            if (FlagHandler::interChipA_State[1] == 2) {
                FlagHandler::interChipA_State[1] = 4;
                FlagHandler::interChipA_StateTimeout[1] = 0;
            }
            break;

        case GPIO_PIN_4: // Layer 2 Ready 2
            if (FlagHandler::interChipB_State[1] == 0) {
                FlagHandler::interChipB_State[1] = 2;
                FlagHandler::interChipB_StateTimeout[1] = 0;
            }
            if (FlagHandler::interChipB_State[1] == 2) {
                FlagHandler::interChipB_State[1] = 4;
                FlagHandler::interChipB_StateTimeout[1] = 0;
            }
            break;
        case GPIO_PIN_6: // Layer 1 Ready 1
            if (FlagHandler::interChipA_State[0] == 0) {
                FlagHandler::interChipA_State[0] = 2;
                FlagHandler::interChipA_StateTimeout[0] = 0;
            }
            if (FlagHandler::interChipA_State[0] == 2) {
                FlagHandler::interChipA_State[0] = 4;
                FlagHandler::interChipA_StateTimeout[0] = 0;
            }
            break;

        case GPIO_PIN_7: // Layer 1 Ready 2
            if (FlagHandler::interChipA_State[0] == 0) {
                FlagHandler::interChipA_State[0] = 2;
                FlagHandler::interChipA_StateTimeout[0] = 0;
            }
            if (FlagHandler::interChipA_State[0] == 2) {
                FlagHandler::interChipA_State[0] = 4;
                FlagHandler::interChipA_StateTimeout[0] = 0;
            }
            break;
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
