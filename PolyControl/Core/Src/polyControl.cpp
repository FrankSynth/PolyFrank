#include "polyControl.hpp"

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABufferLayerA[2 * INTERCHIPBUFFERSIZE];
RAM2_DMA volatile uint8_t interChipDMABufferLayerB[2 * INTERCHIPBUFFERSIZE];

// USB
midi::MidiInterface<COMusb> mididevice(MIDIComRead);
// CDC USB DEVICE missing

// Layer
ID layerId;
Layer layerA(layerId.getNewId());
Layer layerB(layerId.getNewId());

// global settings
GlobalSettings globalSettings;

// InterChip Com
COMinterChip layerCom[2];

// live Data
LiveData liveData;

// hardware
MAX11128 max(&hspi1, 16, Panel_1_CS_GPIO_Port, Panel_1_CS_Pin);
//
TS3A5017D multiplexer = TS3A5017D(4, Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, Panel_ADC_Mult_A_GPIO_Port,
                                  Panel_ADC_Mult_A_Pin, Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin);

// function pointers
void initMidi();
// uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, uint8_t *amount);

// uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
// uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
// uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
// uint8_t sendDeleteAllPatches(uint8_t layerId);

uint8_t test = 0;

// poly control init
void PolyControlInit() {
    ////////Layer init////////
    initPoly();

    allLayers.push_back(&layerA);
    allLayers.push_back(&layerB);

    max.init();
    multiplexer.enableChannels();

    initHID();
    ////////Hardware init////////

    // enable Layer
    HAL_GPIO_WritePin(Layer_Reset_GPIO_Port, Layer_Reset_Pin, GPIO_PIN_SET); // Enable Layer Board

    // mult tmp
    HAL_GPIO_WritePin(Panel_ADC_Mult_A_GPIO_Port, Panel_ADC_Mult_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Panel_ADC_Mult_B_GPIO_Port, Panel_ADC_Mult_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Panel_ADC_Mult_C_GPIO_Port, Panel_ADC_Mult_C_Pin, GPIO_PIN_RESET);

    ////////Sofware init////////
    // enable Panel
    HAL_GPIO_WritePin(Panel_Reset_GPIO_Port, Panel_Reset_Pin, GPIO_PIN_SET); // Enable Panel Board

    // Init Encoder, Touchbuttons,..
    initHID();

    // init EEPROM
    initPreset();

    // init UI, Display
    ui.Init();

    // enable display
    HAL_GPIO_WritePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin, GPIO_PIN_SET);

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

    // init midi
    initMidi();

    println("Hi, Frank here!");
}

uint16_t *testbuffer = (uint16_t *)pFrameBuffer;

void PolyControlRun() { // Here the party starts

    while (1) {
        // if (!) {
        // for (int y = 0; y < 4; y++) {
        //   HAL_Delay(100);

        for (int x = 0; x < 1; x++) {

            multiplexer.setChannel(x);
            max.fetchNewData();
            print("ch: ", max.adcData[0] >> 13, "  value: ", (max.adcData[0] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[1] >> 13, "  value: ", (max.adcData[1] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[2] >> 13, "  value: ", (max.adcData[2] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[3] >> 13, "  value: ", (max.adcData[3] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[4] >> 13, "  value: ", (max.adcData[4] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[5] >> 13, "  value: ", (max.adcData[5] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[6] >> 13, "  value: ", (max.adcData[6] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[7] >> 13, "  value: ", (max.adcData[7] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[8] >> 13, "  value: ", (max.adcData[8] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[9] >> 13, "  value: ", (max.adcData[9] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[10] >> 13, "  value: ", (max.adcData[10] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[11] >> 13, "  value: ", (max.adcData[11] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[12] >> 13, "  value: ", (max.adcData[12] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[13] >> 13, "  value: ", (max.adcData[13] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[14] >> 13, "  value: ", (max.adcData[14] >> 1) & 0x0FFF, " | ",
                  "ch: ", max.adcData[15] >> 13, "  value: ", (max.adcData[15] >> 1) & 0x0FFF);
        }
        println(" ");
        // }
        // }

        // elapsedMillis millitimer = 0;
        // elapsedMillis millitimer2 = 0;
        // elapsedMicros microtimer = 0;

        FlagHandler::handleFlags();

        if (getRenderState() == RENDER_DONE) {
            ui.Draw();
        }

        layerCom[0].beginSendTransmission();
    };
}

///////////////////////////////////////////////// LAYER SPECIFIC HANDLING
////////////////////////////////////////////////

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
            println("HAL_SPI_TxCpltCallback: close SPI Line spi4");

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
    liveData.keyPressed(channel, cc, value);
}
void initMidi() {
    mididevice.setHandleNoteOn(midiNoteOn);
    mididevice.setHandleNoteOff(midiNoteOff);
    mididevice.setHandleControlChange(midiControlChange);
}

// EXTI Callback

void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    // println(pin);

    switch (pin) {
        case GPIO_PIN_12: FlagHandler::Control_Encoder_Interrupt = true; break; // ioExpander -> encoder
        case GPIO_PIN_2:
            FlagHandler::Control_Touch_Interrupt = true;
            break; // touch
            // case GPIO_PIN_2: FlagHandler::Panel_2_Touch_Interrupt = true; break;    // touch     //TODO pin
            // raussuchen und interrupts eintragen case GPIO_PIN_2: FlagHandler::Panel_1_Touch_Interrupt = true;
            // break;
            // // touch

        default: break;
    }
}
