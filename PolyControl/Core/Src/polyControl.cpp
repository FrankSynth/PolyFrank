#include "polyControl.hpp"

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABufferLayerA[2 * INTERCHIPBUFFERSIZE];
RAM2_DMA volatile uint8_t interChipDMABufferLayerB[2 * INTERCHIPBUFFERSIZE];

// GUI
GUI ui;

// USB
midi::MidiInterface<COMusb> mididevice(MIDIComRead);
// CDC USB DEVICE missing

// Layer
ID layerId;
Layer layerA(layerId.getNewId());
Layer layerB(layerId.getNewId());

// InterChip Com
COMinterChip layerCom[2];

// live Data
LiveData liveData;

// function pointers
void initMidi();
// uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, uint8_t *amount);

// uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
// uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
// uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
// uint8_t sendDeleteAllPatches(uint8_t layerId);

// poly control init
void PolyControlInit() {

    allLayers.push_back(&layerA);
    allLayers.push_back(&layerB);

    // DataElement::sendSetting  = sendSetting;
    // Layer::sendCreatePatchInOut = sendCreatePatchInOut;
    // PatchElementInOut::sendUpdatePatchInOut = sendUpdatePatchInOut;
    // Layer::sendDeletePatchInOut = sendDeletePatchInOut;

    initPoly();

    // enable display
    HAL_GPIO_WritePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin, GPIO_PIN_SET);
    // init UI, Display
    ui.Init(allLayers);

    // init communication to render chips
    layerCom[0].initOutTransmission(
        std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi4, std::placeholders::_1, std::placeholders::_2),
        (uint8_t *)interChipDMABufferLayerA, 0);

    layerCom[1].initOutTransmission(
        std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi5, std::placeholders::_1, std::placeholders::_2),
        (uint8_t *)interChipDMABufferLayerB, 1);

    // init EEPROM
    initPreset();
    // HAL_LTDC_ProgramLineEvent(&hltdc, 0);
    // halltdcline
    // init midi
    initMidi();

    HAL_Delay(512);

    println("Hi, Frank here!");
}

// FRAMEBUFFER volatile uint16_t testbuffer[10000];

uint16_t *testbuffer = (uint16_t *)pFrameBuffer;

void PolyControlRun() { // Here the party starts

    // EEPROM_SPI_WriteBuffer(dataW, 0x00, 1);
    // EEPROM_SPI_ReadBuffer(dataR, 0x00, 1);

    // HAL_Delay(20);
    // uint32_t time;

    while (1) {

        if (getRenderState() == RENDER_DONE) {
            ui.Draw();
        }

        // HAL_Delay(100000);
        // println("test ", testbuffer++);
        // println("Hello Jakob ", testbuffer++);
        // println("SDRam Status ", HAL_SDRAM_GetState(&hsdram1));
        // HAL_GPIO_TogglePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin);
        // HAL_Delay(500);
        // HAL_SDRAM_SendCommand
        // HAL_GPIO_TogglePin(Control_Display_Enable_GPIO_Port, Control_Display_Enable_Pin);
        // testbuffer++;

        // actionHandler.callActionEncoder_1_CW();
        // actionHandler.callActionEncoder_2_CW();

        // uint32_t time = __HAL_TIM_GetCounter(&htim2);
        // // println(__HAL_TIM_GetCounter(&htim2));
        // drawRectangleFill(0xFFFFFF00, 0, 0, LCDWIDTH, LCDHEIGHT);
        // HAL_Delay(500);
        // drawRectangleFill(0xFFFF0000, 0, 0, LCDWIDTH, LCDHEIGHT / 2);
        //  HAL_Delay(500);

        //  if (!FlagHandler::renderingDoneSwitchBuffer) {
        //     FlagHandler::renderingDoneSwitchBuffer = true;

        // HAL_Delay(25);

        // }

        // println("Framebuffer Adress ", (uint32_t)pFrameBuffer);
        // SwitchFrameBuffer();
        // }

        //  drawRectangleFill(0xFFFFFFFF, 0, 0, LCDWIDTH, LCDHEIGHT / 2);

        // println(__HAL_TIM_GetCounter(&htim2) - time);

        // HAL_Delay(250);
        // actionHandler.callActionEncoder_1_CCW();
        //  actionHandler.callActionEncoder_2_CW();
        // actionHandler.callActionLeft3();
        // drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);

        // ui.Draw();
    };
}

///////////////////////////////////////////////// LAYER SPECIFIC HANDLING
////////////////////////////////////////////////

// void sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount) {
//     layerCom[layerId].sendSetting(moduleId, settingsId, amount);
// }

uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, uint8_t *amount) {
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

    // InterChip Com SPI 1
    if (hspi == &hspi1) {

        if (FlagHandler::interChipA_DMA_Started[0] == 1) {
            FlagHandler::interChipA_DMA_Started[0] = 0;
            FlagHandler::interChipA_DMA_Finished[0] = 1;

            // close ChipSelectLine
        }

        if (FlagHandler::interChipB_DMA_Started[0] == 1) {
            FlagHandler::interChipB_DMA_Started[0] = 0;
            FlagHandler::interChipB_DMA_Finished[0] = 1;

            // close ChipSelectLine

            // close both send lines here to start recipient evaluation?
        }

        if (FlagHandler::interChipReceive_DMA_Started == 1) {
            FlagHandler::interChipReceive_DMA_Started = 0;
            FlagHandler::interChipReceive_DMA_Finished = 1;
        }
    }
    // layer2 spi
    // if (hspi == &hspi2) {

    //     if (FlagHandler::interChipA_DMA_Started[1] == 1) {
    //         FlagHandler::interChipA_DMA_Started[1] = 0;
    //         FlagHandler::interChipA_DMA_Finished[1] = 1;

    //         // close ChipSelectLine
    //     }

    //     if (FlagHandler::interChipB_DMA_Started[1] == 1) {
    //         FlagHandler::interChipB_DMA_Started[1] = 0;
    //         FlagHandler::interChipB_DMA_Finished[1] = 1;

    //         // close ChipSelectLine

    //         // close both send lines here to start recipient evaluation?
    //     }

    //     if (FlagHandler::interChipReceive_DMA_Started[1] == 1) {
    //         FlagHandler::interChipReceive_DMA_Started[1] = 0;
    //         FlagHandler::interChipReceive_DMA_Finished[1] = 1;
    //     }
    // }
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