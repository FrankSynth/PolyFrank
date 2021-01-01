#include "polyRender.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "hardware/TS3A5017D.hpp"

#include "render/renderAudio.hpp"

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

void hardwareInit() {
    // Audio Render Chips
    __HAL_SAI_ENABLE(&hsai_BlockA1);

    HAL_Delay(200);
    HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    // HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_RESET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_SET);
    // updateI2CAddress(); // update the I2C addresses of the MCP4728 DACs
    cvDacA.init();
    cvDacB.init();
    cvDacC.init();
}

void PolyRenderInit() {

    hardwareInit();

    audioDacA.init();

    switchLadder.disableChannels();

    // empty buffers
    uint32_t emptyData = 0;
    fastMemset(&emptyData, (uint32_t *)interChipDMABuffer, 2 * INTERCHIPBUFFERSIZE / 4);
    fastMemset(&emptyData, (uint32_t *)saiBuffer, SAIDMABUFFERSIZE * 2);

    // cvDacA.setDataPointer((uint8_t *)&cvDacDMABuffer[0]);
    // cvDacB.setDataPointer((uint8_t *)&cvDacDMABuffer[1]);
    // cvDacC.setDataPointer((uint8_t *)&cvDacDMABuffer[2]);

    // init allLayers
    allLayers.push_back(&layerA);

    // general inits
    initPoly();

    // TODO copy wavetables
    initAudioRendering();

    // interChipCom
    layerCom.initInTransmission(
        std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi1, std::placeholders::_1, std::placeholders::_2),
        std::bind<uint8_t>(HAL_SPI_Abort, &hspi1), (uint8_t *)interChipDMABuffer);

    layerCom.beginReceiveTransmission();
}

void PolyRenderRun() {

    // init Sai, first fill buffer
    // renderAudio((int32_t *)saiBuffer, SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS, AUDIOCHANNELS);
    // audioDacA.startSAI();

    elapsedMillis millitimer = 0;

    elapsedMillis dacSendTimer = 0;
    elapsedMicros dacSendTimer2 = 0;
    uint32_t microTimer = micros();
    while (1) {

        FlagHandler::handleFlags();

        // cvDacBbuffer[3] = (fast_sin_f32((float)micros() / 1000.0f) + 1) * 4095 / 2; // 1khz sinus
        // cvDacBbuffer[3] = (fast_sin_f32((float)micros() / 100000.0f) + 1) * 4095 / 2; // 10hz sinus

        // switch ladder Filter
        switchLadder.setChannel(layerA.test.dSelectFilter.valueMapped);

        if (micros() - microTimer > layerA.test.aResonance.valueMapped * 1000) {
            microTimer = micros();

            static float dacStep = 0;

            dacStep += layerA.test.aCutoff.valueMapped / (10 / layerA.test.aResonance.valueMapped); // 100hz sinus

            cvDacB.data[3] = (fast_sin_f32(dacStep) + 1) * 2047; // 100hz sinus

            if (dacStep > 1) {
                dacStep -= 1;
            }

            cvDacA.data[0] = layerA.test.aCutoff.valueMapped * 4095;
            cvDacA.data[1] = (1 - layerA.test.aResonance.valueMapped) * 4095;
            cvDacA.data[2] = layerA.test.aFreq.valueMapped * 4095;
            cvDacA.data[3] = (1 - layerA.test.aDistort.valueMapped) * 4095;

            cvDacA.fastUpdate();
            cvDacB.fastUpdate();
            cvDacC.fastUpdate();
        }

        // cvDacAbuffer[0] = fast_sin_f32((float)micros() / 1000000.0f) * 2048 + 2047;
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1,
        //                      fastMapLEDBrightness((fast_sin_f32((float)micros() / 1000000.0f) + 1) / 2) *
        //                          LEDMAXBRIGHTNESSCOUNT);
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2,
        //                      fastMapLEDBrightness((fast_sin_f32((float)micros() / 5000000.0f) + 1) / 2) *
        //                          LEDMAXBRIGHTNESSCOUNT);
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, cvDacAbuffer[0] / 4);
        // cvDacAbuffer[1] = fast_sin_f32((float)micros() / 300000.0f) * 2048 + 2047;
        // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, 1);
        // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, 1);

        // if (millitimer > 10000) {
        //     millitimer = 0;
        //     //     uint8_t answer;
        //     uint8_t commandZeros[2];
        //     commandZeros[0] = 0x44;
        //     commandZeros[1] = 0xFF;
        //     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);

        //     if (HAL_SPI_Transmit(&hspi4, commandZeros, 2, 50) != HAL_OK) {
        //         Error_Handler();
        //         println("transmit error");
        //     }
        //     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
        //     println("shotdown sent");
        //     microsecondsDelay(1);
        //     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
        //     if (HAL_SPI_Receive(&hspi4, &answer, 1, 50) != HAL_OK) {
        //         Error_Handler();
        //         println("receive error");
        //     }
        //     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);

        //     println("answer zeros: ", answer);
        // }
        // HAL_SAI_Transmit(&hsai_BlockA1, (uint8_t *)saiBuffer, 32, 50);
    }
}

// EXTI Callbacks
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    // reception Line from Control
    if (pin == GPIO_PIN_8)
        FlagHandler::interChipReceive_DMA_Finished = true;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]), SAIDMABUFFERSIZE, AUDIOCHANNELS);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((int32_t *)saiBuffer, SAIDMABUFFERSIZE, AUDIOCHANNELS);
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
