#include "polyRender.hpp"
#include "render/renderAudioDef.h"

/// LAYER
ID layerId;
Layer layerA(layerId.getNewId());

// InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMABuffer[2 * INTERCHIPBUFFERSIZE]);
COMinterChip layerCom;

// CV DACS
RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABuffer[10][4]);
MCP4728 cvDacA(&hi2c1, 0x00, LDAC_1_GPIO_Port, LDAC_1_Pin, (uint16_t *)&cvDacDMABuffer[0]);
MCP4728 cvDacB(&hi2c1, 0x01, LDAC_2_GPIO_Port, LDAC_2_Pin, (uint16_t *)&cvDacDMABuffer[1]);
MCP4728 cvDacC(&hi2c1, 0x02, LDAC_3_GPIO_Port, LDAC_3_Pin, (uint16_t *)&cvDacDMABuffer[2]);

// Switch Ladder  //andere chip aber selbe logik
TS3A5017D switchLadder = TS3A5017D(4, switch_1_open_GPIO_Port, switch_1_open_Pin, switch_1_A_GPIO_Port, switch_1_A_Pin,
                                   switch_1_B_GPIO_Port, switch_1_B_Pin);

// AUDIO DAC
RAM2_DMA ALIGN_32BYTES(volatile int32_t saiBuffer[SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS]);
PCM1690 audioDacA(&hsai_BlockA1, &hspi4, (int32_t *)saiBuffer);

void PolyRenderInit() {

    layerA.resetLayer();
    // general inits
    initPoly();

    // Audio Render Chips
    __HAL_SAI_ENABLE(&hsai_BlockA1);
    HAL_Delay(200);
    HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    audioDacA.init();

    // TODO copy wavetables to RAM D1
    initAudioRendering();

    // probably obsolete
    // initCVRendering();

    // CV DACs
    cvDacA.init();
    cvDacB.init();
    cvDacC.init();
    FlagHandler::renderNewCVFunc = renderCVs;

    // Ladder stuff
    switchLadder.disableChannels();

    // empty buffers
    uint32_t emptyData = 0;
    fastMemset(&emptyData, (uint32_t *)interChipDMABuffer, 2 * INTERCHIPBUFFERSIZE / 4);
    fastMemset(&emptyData, (uint32_t *)saiBuffer, SAIDMABUFFERSIZE * 2);

    // init allLayers
    allLayers.push_back(&layerA);

    // interChipCom
    layerCom.initInTransmission(
        std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi1, std::placeholders::_1, std::placeholders::_2),
        std::bind<uint8_t>(HAL_SPI_Abort, &hspi1), (uint8_t *)interChipDMABuffer);
    layerCom.beginReceiveTransmission();
}

void PolyRenderRun() {

    // init Sai, first fill buffer once
    renderAudio((int32_t *)saiBuffer, SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS);
    audioDacA.startSAI();

    // run loop
    while (true) {
        FlagHandler::handleFlags();
    }
}

// CALLBACKS

// cv DACs Send Callbacks
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (FlagHandler::cvDacAStarted) {
        cvDacB.fastUpdate();
        FlagHandler::cvDacAStarted = false;
        FlagHandler::cvDacBStarted = true;
    }
    else if (FlagHandler::cvDacBStarted) {
        cvDacC.fastUpdate();
        FlagHandler::cvDacBStarted = false;
        FlagHandler::cvDacCStarted = true;
    }
    else if (FlagHandler::cvDacCStarted) {
        FlagHandler::cvDacCStarted = false;
        FlagHandler::cvDacCFinished = true;
        cvDacA.resetLatchPin();
        cvDacB.resetLatchPin();
        cvDacC.resetLatchPin();

        // TODO check if lines go down
    }
}
// void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {}

// Audio Render Callbacks
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]), SAIDMABUFFERSIZE);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((int32_t *)saiBuffer, SAIDMABUFFERSIZE);
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai) {
    PolyError_Handler("SAI error callback");
}

// reception from Control SPI
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com SPI 1
    if (hspi == &hspi1) {
        if (FlagHandler::interChipReceive_DMA_Started == 1) {
            FlagHandler::interChipReceive_DMA_Started = 0;
            FlagHandler::interChipReceive_DMA_Finished = 1;
        }
    }
}

// reception line callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    // reception Line from Control
    if (pin == GPIO_PIN_8)
        FlagHandler::interChipReceive_DMA_Finished = true;
}

// cv rendering timer IRQ
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim15) {
        if (FlagHandler::cvDacCFinished) {
            FlagHandler::cvDacCFinished = false;
            cvDacA.setLatchPin();
            cvDacB.setLatchPin();
            cvDacC.setLatchPin();

            // out DacB and DacC gets automatially triggered by flags when transmission is done
            cvDacA.fastUpdate();
            FlagHandler::cvDacAStarted = true;
        }
        else {
            PolyError_Handler("polyRender | timerCallback | cvDacCFinished = false");
        }
        FlagHandler::renderNewCV = true;
    }
}
