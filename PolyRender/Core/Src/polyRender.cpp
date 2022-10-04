#include "polyRender.hpp"
#include "hardware/device.hpp"
#include "render/renderAudioDef.h"

// #include <cstdlib>

/// LAYER
ID layerId;
Layer layerA(layerId.getNewId());

extern devManager deviceManager;
spiBus spiBusLayer;
spiBus spiBusCVDac[2];

// InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);

// CV DACS
RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABufferA[20]);
RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABufferB[20]);

RAM2_DMA ALIGN_32BYTES(volatile float interchipLFOBuffer[2]);

int8_t audioSendBuffer[UPDATEAUDIOBUFFERSIZE - 1];

COMinterChip layerCom(&spiBusLayer, (uint8_t *)interChipDMAInBuffer, (uint8_t *)interChipDMAOutBuffer);

DUALBU22210 cvDac[2];

// Switch Ladder  //andere chip aber selbe logik
TS3A5017D switchLadder = TS3A5017D(4, SWITCH_1_A_GPIO_Port, SWITCH_1_A_Pin, SWITCH_1_B_GPIO_Port, SWITCH_1_B_Pin);

// AUDIO DAC
RAM2_DMA ALIGN_32BYTES(volatile int32_t saiBuffer[SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS]);
PCM1690 audioDacA(&hsai_BlockA1, &hspi6, (int32_t *)saiBuffer);

void outputCollect();

void sendDACs();

extern void polyRenderLoop();

void PolyRenderInit() {

    // initCVRendering();

    loadInitialWavetables();

    HAL_Delay(10);

    spiBusLayer.connectToInterface(&hspi4);
    deviceManager.addBus(&spiBusLayer);

    layerA.chipID = !HAL_GPIO_ReadPin(CHIP_ID_A_GPIO_Port, CHIP_ID_A_Pin);

    layerA.id = !HAL_GPIO_ReadPin(CHIP_ID_B_GPIO_Port, CHIP_ID_B_Pin);
    // layerA.id = 0;+++++

    // init pseudo rand so all chips follow different patterns.
    std::srand(layerA.chipID + layerA.id + 1);

    // CV DACs init
    spiBusCVDac[0].connectToInterface(&hspi1);
    spiBusCVDac[1].connectToInterface(&hspi2);

    deviceManager.addBus(&spiBusCVDac[0]);
    deviceManager.addBus(&spiBusCVDac[1]);

    cvDac[0].configurate(&spiBusCVDac[0], cvDacDMABufferA);
    cvDac[1].configurate(&spiBusCVDac[1], cvDacDMABufferB);

    initPoly();

    // init all Layers
    allLayers.push_back(&layerA);
    layerA.initLayer();
    layerA.resetLayer();

    // empty buffers
    uint32_t emptyData = 0;
    fastMemset(&emptyData, (uint32_t *)interChipDMAInBuffer, 2 * (INTERCHIPBUFFERSIZE + 4) / 4);
    fastMemset(&emptyData, (uint32_t *)interChipDMAOutBuffer, 2 * (INTERCHIPBUFFERSIZE + 4) / 4);
    fastMemset(&emptyData, (uint32_t *)saiBuffer, SAIDMABUFFERSIZE * 2);

    layerCom.beginReceiveTransmission();

    // Audio Render Chips
    __HAL_SAI_ENABLE(&hsai_BlockA1);
    HAL_GPIO_WritePin(AUDIO_RST_GPIO_Port, AUDIO_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    audioDacA.init();

    // FlagHandler::sendRenderedCVsFunc = sendDACs;
    FlagHandler::renderNewCVFunc = renderCVs;
    FlagHandler::outputCollectFunc = outputCollect;
}

void PolyRenderRun() {

    std::string message =
        "Hi, it's Render. PolyFrank Render...  ID: " + std::to_string(layerA.id) + std::to_string(layerA.chipID);
    println(message);

    // start Receive
    if (layerA.chipID == 1) {
        HAL_UART_Receive_DMA(&huart1, (uint8_t *)interchipLFOBuffer, 8);
        huart1.Instance->ICR = 0b1100;
        huart1.Instance->RQR = UART_RXDATA_FLUSH_REQUEST; // clear rx Register

        // // detect timeout for resyncing DMA
        // HAL_UART_ReceiverTimeout_Config(&huart1, 16);
        // HAL_UART_EnableReceiverTimeout(&huart1);
        // huart1.Instance->CR1 |= USART_CR1_RTOIE_Msk;
    }

    // start cv rendering
    renderCVs();
    sendDACs();

    // start audio rendering
    renderAudio((int32_t *)saiBuffer);
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));
    audioDacA.startSAI();

    HAL_Delay(20); // wait for receiver started uart ->

    // start Transmit
    if (layerA.chipID == 0) {
        huart1.Instance->RQR = UART_TXDATA_FLUSH_REQUEST; // clear tx Register
        HAL_UART_Transmit_DMA(&huart1, (uint8_t *)interchipLFOBuffer, 8);
    }

    HAL_TIM_Base_Start_IT(&htim15);

    // run loop
    polyRenderLoop();
}
