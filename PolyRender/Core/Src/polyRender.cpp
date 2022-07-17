#include "polyRender.hpp"
#include "hardware/device.hpp"
#include "render/renderAudioDef.h"

// #include <cstdlib>

/// LAYER
ID layerId;
Layer layerA(layerId.getNewId());

extern devManager deviceManager;
spiBus spiBusLayer;

// InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);

// CV DACS
RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABuffer[ALLDACS][4]);

int8_t audioSendBuffer[UPDATEAUDIOBUFFERSIZE - 1];

COMinterChip layerCom(&spiBusLayer, (uint8_t *)interChipDMAInBuffer, (uint8_t *)interChipDMAOutBuffer);

MCP4728 cvDac[] = {

    MCP4728(&hi2c1, 0x01, LDAC_1_GPIO_Port, LDAC_1_Pin, (uint16_t *)(&cvDacDMABuffer[0])),
    MCP4728(&hi2c1, 0x02, LDAC_2_GPIO_Port, LDAC_2_Pin, (uint16_t *)(&cvDacDMABuffer[1])),
    MCP4728(&hi2c1, 0x03, LDAC_3_GPIO_Port, LDAC_3_Pin, (uint16_t *)(&cvDacDMABuffer[2])),
    MCP4728(&hi2c1, 0x04, LDAC_4_GPIO_Port, LDAC_4_Pin, (uint16_t *)(&cvDacDMABuffer[3])),

    MCP4728(&hi2c2, 0x01, LDAC_1_GPIO_Port, LDAC_1_Pin, (uint16_t *)(&cvDacDMABuffer[4])),
    MCP4728(&hi2c2, 0x02, LDAC_2_GPIO_Port, LDAC_2_Pin, (uint16_t *)(&cvDacDMABuffer[5])),
    MCP4728(&hi2c2, 0x03, LDAC_3_GPIO_Port, LDAC_3_Pin, (uint16_t *)(&cvDacDMABuffer[6])),

    MCP4728(&hi2c3, 0x01, LDAC_1_GPIO_Port, LDAC_1_Pin, (uint16_t *)(&cvDacDMABuffer[7])),
    MCP4728(&hi2c3, 0x02, LDAC_2_GPIO_Port, LDAC_2_Pin, (uint16_t *)(&cvDacDMABuffer[8])),
    MCP4728(&hi2c3, 0x03, LDAC_3_GPIO_Port, LDAC_3_Pin, (uint16_t *)(&cvDacDMABuffer[9]))};

// Switch Ladder  //andere chip aber selbe logik
TS3A5017D switchLadder = TS3A5017D(4, switch_1_A_GPIO_Port, switch_1_A_Pin, switch_1_B_GPIO_Port, switch_1_B_Pin);

// AUDIO DAC
RAM2_DMA ALIGN_32BYTES(volatile int32_t saiBuffer[SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS]);
PCM1690 audioDacA(&hsai_BlockA1, &hspi2, (int32_t *)saiBuffer);

void testMCPI2CAddress();
void resetMCPI2CAddress();
void outputCollect();

void sendDACs();

extern void polyRenderLoop();

void PolyRenderInit() {

    initCVRendering();

    loadInitialWavetables();

    HAL_Delay(10);

    spiBusLayer.connectToInterface(&hspi1);
    deviceManager.addBus(&spiBusLayer);

    layerA.chipID = !HAL_GPIO_ReadPin(CHIP_ID_A_GPIO_Port, CHIP_ID_A_Pin);

    layerA.id = !HAL_GPIO_ReadPin(CHIP_ID_B_GPIO_Port, CHIP_ID_B_Pin);

    // init pseudo rand so all chips follow different patterns.
    std::srand(layerA.chipID + layerA.id + 1);

    // CV DACs init

    testMCPI2CAddress(); // check all MCP4728 addressing

    for (uint16_t i = 0; i < ALLDACS; i++) {
        cvDac[i].init();
    }
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
    HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    audioDacA.init();

    // FlagHandler::sendRenderedCVsFunc = sendDACs;
    // FlagHandler::renderNewCVFunc = renderCVs;
    FlagHandler::outputCollectFunc = outputCollect;
}

void PolyRenderRun() {

    println("////////// Hi, it's Render. PolyFrank Render. //////////");
    println("Layer ID is: ", layerA.id);
    println("Chip ID is: ", layerA.chipID);

    // enable reception line
    // HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);

    // start cv rendering
    renderCVs();
    sendDACs();

    // start audio rendering
    renderAudio((int32_t *)saiBuffer);
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));
    audioDacA.startSAI();

<<<<<<< HEAD
    // start cv rendering
    renderCVs();
    sendDACs();
=======
>>>>>>> polyrender
    HAL_TIM_Base_Start_IT(&htim15);

    // run loop
    polyRenderLoop();
}

void testMCPI2CAddress() {

    // set latch pins high

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_4_GPIO_Port, LDAC_4_Pin, GPIO_PIN_SET);

    // check i2c ready
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        println("I2C AddressChange - I2C Busy");
        return;
    }

    // test Address reachable -> set target address

    uint8_t addressFailed = 0;

    for (uint16_t i = 0; i < ALLDACS; i++) {
        addressFailed |= cvDac[i].testAddress();
    }

    if (addressFailed) { // reset all MCP4728 address to start clean and reassign every one;
        resetMCPI2CAddress();

        sendI2CAddressUpdate(i2c1Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, 0x00, 0x01);
        sendI2CAddressUpdate(i2c2Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, 0x00, 0x01);
        sendI2CAddressUpdate(i2c3Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, 0x00, 0x01);

        sendI2CAddressUpdate(i2c1Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, 0x00, 0x02);
        sendI2CAddressUpdate(i2c2Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, 0x00, 0x02);
        sendI2CAddressUpdate(i2c3Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, 0x00, 0x02);

        sendI2CAddressUpdate(i2c1Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, 0x00, 0x03);
        sendI2CAddressUpdate(i2c2Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, 0x00, 0x03);
        sendI2CAddressUpdate(i2c3Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, 0x00, 0x03);

        sendI2CAddressUpdate(i2c1Pins, LDAC_4_GPIO_Port, LDAC_4_Pin, 0x00, 0x04);
    }
}

void resetMCPI2CAddress() {

    // set latch pins high

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_4_GPIO_Port, LDAC_4_Pin, GPIO_PIN_SET);

    // check i2c ready
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        println("I2C AddressChange - I2C Busy");
        return;
    }

    // test Address reachable -> set target address
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, address, 0x00);
        sendI2CAddressUpdate(i2c2Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, address, 0x00);
        sendI2CAddressUpdate(i2c3Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, address, 0x00);
    }
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, address, 0x00);
        sendI2CAddressUpdate(i2c2Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, address, 0x00);
        sendI2CAddressUpdate(i2c3Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, address, 0x00);
    }
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, address, 0x00);
        sendI2CAddressUpdate(i2c2Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, address, 0x00);
        sendI2CAddressUpdate(i2c3Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, address, 0x00);
    }
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_4_GPIO_Port, LDAC_4_Pin, address, 0x00);
    }
}
