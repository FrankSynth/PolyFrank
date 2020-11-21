#include "polyRender.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudio.hpp"

/// LAYER
ID layerId;
Layer layerA(layerId.getNewId());

// InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMABuffer[2 * INTERCHIPBUFFERSIZE]);
COMinterChip layerCom;

// CV DACS
uint16_t cvDacAbuffer[4];
uint16_t cvDacBbuffer[4];
uint16_t cvDacCbuffer[4];
MCP4728 cvDacA(&hi2c1, 0x00, LDAC_1_GPIO_Port, LDAC_1_Pin);
MCP4728 cvDacB(&hi2c1, 0x01, LDAC_2_GPIO_Port, LDAC_2_Pin);
MCP4728 cvDacC(&hi2c1, 0x02, LDAC_3_GPIO_Port, LDAC_3_Pin);

// AUDIO DAC
RAM2_DMA ALIGN_32BYTES(volatile uint32_t saiBuffer[SAIDMABUFFERSIZE]);
PCM1690 audioDacA(&hsai_BlockA1, &hspi4, (uint32_t *)saiBuffer);

void hardwareInit() {

    // updateI2CAddress(); // update the I2C addresses of the MCP4728 DACs

    cvDacA.init();
    cvDacB.init();
    cvDacC.init();

    // Audio Render Chips
    HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_SET);
}

void PolyRenderInit() {

    hardwareInit();

    // empty buffers
    uint32_t emptyData = 0;
    fastMemset(&emptyData, (uint32_t *)interChipDMABuffer, 2 * INTERCHIPBUFFERSIZE / 4);
    fastMemset(&emptyData, (uint32_t *)saiBuffer, SAIDMABUFFERSIZE);

    cvDacA.setDataPointer((uint8_t *)cvDacAbuffer);
    cvDacB.setDataPointer((uint8_t *)cvDacBbuffer);
    cvDacC.setDataPointer((uint8_t *)cvDacCbuffer);

    // init allLayers
    allLayers.push_back(&layerA);

    // general inits
    initPoly();

    // interChipCom
    layerCom.initInTransmission(
        std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi1, std::placeholders::_1, std::placeholders::_2),
        std::bind<uint8_t>(HAL_SPI_Abort, &hspi1), (uint8_t *)interChipDMABuffer);

    layerCom.beginReceiveTransmission();

    // init Sai, first fill buffer
    renderAudio((uint32_t *)saiBuffer, SAIDMABUFFERSIZE);
    audioDacA.startSAI();
}

void PolyRenderRun() {

    while (1) {

        FlagHandler::handleFlags();
        // cvDacAbuffer[0] = layerA.adsrA.aDecay.valueMapped * 400;
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1,
        //                      fastMapLEDBrightness(layerA.adsrA.aDecay.valueMapped * 0.1) * LEDMAXBRIGHTNESSCOUNT);

        // cvDacA.fastUpdate();

        // cvDacAbuffer[0] = fast_sin_f32((float)micros() / 1000000.0f) * 2048 + 2047;
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1,
                             fastMapLEDBrightness(fast_sin_f32((float)micros() / 1000000.0f)) * LEDMAXBRIGHTNESSCOUNT);
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2,
                             fastMapLEDBrightness(fast_sin_f32((float)micros() / 300000.0f)) * LEDMAXBRIGHTNESSCOUNT);
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, cvDacAbuffer[0] / 4);
        // cvDacAbuffer[1] = fast_sin_f32((float)micros() / 300000.0f) * 2048 + 2047;
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, powf(2, (float)cvDacAbuffer[1] / (float)409));
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, powf(2, (float)cvDacAbuffer[1] / (float)409));
    }
}

// EXTI Callbacks
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    // reception Line from Control
    if (pin == GPIO_PIN_8)
        FlagHandler::interChipReceive_DMA_Finished = true;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((uint32_t *)saiBuffer, SAIDMABUFFERSIZE >> 1);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((uint32_t *)&(saiBuffer[SAIDMABUFFERSIZE >> 1]), SAIDMABUFFERSIZE >> 1);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com SPI 1
    if (hspi == &hspi1) {
        if (FlagHandler::interChipReceive_DMA_Started == 1) {
            FlagHandler::interChipReceive_DMA_Started = 0;
            FlagHandler::interChipReceive_DMA_Finished = 1;
        }
    }
}
