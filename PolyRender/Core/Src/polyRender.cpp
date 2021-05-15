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
MCP4728 cvDacA(&hi2c1, 0x01, LDAC_1_GPIO_Port, LDAC_1_Pin, (uint16_t *)&cvDacDMABuffer[0]);
MCP4728 cvDacB(&hi2c1, 0x02, LDAC_2_GPIO_Port, LDAC_2_Pin, (uint16_t *)&cvDacDMABuffer[1]);
MCP4728 cvDacC(&hi2c1, 0x03, LDAC_3_GPIO_Port, LDAC_3_Pin, (uint16_t *)&cvDacDMABuffer[2]);

RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABufferx[10][4]);
MCP4728 cvDacAx(&hi2c2, 0x01, LDAC_1_GPIO_Port, LDAC_1_Pin, (uint16_t *)&cvDacDMABufferx[0]);
MCP4728 cvDacBx(&hi2c2, 0x02, LDAC_2_GPIO_Port, LDAC_2_Pin, (uint16_t *)&cvDacDMABufferx[1]);
MCP4728 cvDacCx(&hi2c2, 0x03, LDAC_3_GPIO_Port, LDAC_3_Pin, (uint16_t *)&cvDacDMABufferx[2]);

// Switch Ladder  //andere chip aber selbe logik
TS3A5017D switchLadder = TS3A5017D(4, switch_1_open_GPIO_Port, switch_1_open_Pin, switch_1_A_GPIO_Port, switch_1_A_Pin,
                                   switch_1_B_GPIO_Port, switch_1_B_Pin);

// AUDIO DAC
RAM2_DMA ALIGN_32BYTES(volatile int32_t saiBuffer[SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS]);
PCM1690 audioDacA(&hsai_BlockA1, &hspi4, (int32_t *)saiBuffer);

void testMCPI2CAddress();
void resetMCPI2CAddress();

void PolyRenderInit() {

    // CV DACs
    // cvDacAx.init();
    // cvDacBx.init();
    // cvDacCx.init();

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);

    testMCPI2CAddress(); // check all MCP4728 addressing

    cvDacA.init();
    cvDacB.init();
    cvDacC.init();

    initCVRendering();

    // general inits
    initPoly();

    // init allLayers
    allLayers.push_back(&layerA);
    layerA.resetLayer();

    initAudioRendering();

    // empty buffers
    uint32_t emptyData = 0;
    fastMemset(&emptyData, (uint32_t *)interChipDMABuffer, 2 * INTERCHIPBUFFERSIZE / 4);
    fastMemset(&emptyData, (uint32_t *)saiBuffer, SAIDMABUFFERSIZE * 2);

    // interChipCom
    layerCom.initInTransmission(
        std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi1, std::placeholders::_1, std::placeholders::_2),
        std::bind<uint8_t>(HAL_SPI_Abort, &hspi1), (uint8_t *)interChipDMABuffer);
    // HAL_Delay(20);
    layerCom.beginReceiveTransmission();

    // Audio Render Chips
    __HAL_SAI_ENABLE(&hsai_BlockA1);
    // HAL_Delay(20);
    HAL_GPIO_WritePin(Audio_Reset_GPIO_Port, Audio_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    audioDacA.init();

    // probably obsolete

    FlagHandler::renderNewCVFunc = renderCVs;

    // Ladder stuff
    switchLadder.disableChannels();
}

void PolyRenderRun() {
    // enable reception line
    HAL_GPIO_WritePin(SPI_Ready_toControl_GPIO_Port, SPI_Ready_toControl_Pin, GPIO_PIN_SET);

    // start cv rendering
    renderCVs();
    HAL_TIM_Base_Start_IT(&htim15);

    // start audio rendering
    renderAudio((int32_t *)saiBuffer);
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));
    audioDacA.startSAI();

    // run loop
    while (true) {
        FlagHandler::handleFlags();
    }
}

// CALLBACKS

// cv DACs Send Callbacks
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == hi2c1.Instance) {
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
        }
    }
    // else if (hi2c->Instance == hi2c2.Instance) {
    //     if (FlagHandler::cvDacAxStarted) {
    //         cvDacBx.fastUpdate();

    //         FlagHandler::cvDacAxStarted = false;
    //         FlagHandler::cvDacBxStarted = true;
    //     }
    //     else if (FlagHandler::cvDacBxStarted) {
    //         cvDacCx.fastUpdate();

    //         FlagHandler::cvDacBxStarted = false;
    //         FlagHandler::cvDacCxStarted = true;
    //     }
    //     else if (FlagHandler::cvDacCxStarted) {
    //         FlagHandler::cvDacCxStarted = false;
    //         FlagHandler::cvDacCxFinished = true;
    //     }
    // }
}
// void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {}

// Audio Render Callbacks
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    renderAudio((int32_t *)saiBuffer);
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
    if (pin == GPIO_PIN_8) {
        // disable reception line
        // println("EXTI callback, transmission done");
        HAL_GPIO_WritePin(SPI_Ready_toControl_GPIO_Port, SPI_Ready_toControl_Pin, GPIO_PIN_RESET);
        // rising flank
        FlagHandler::interChipReceive_DMA_Finished = true;
    }
}

inline void sendDACs() {
    cvDacA.switchIC2renderBuffer();
    cvDacB.switchIC2renderBuffer();
    cvDacC.switchIC2renderBuffer();
    // cvDacAx.switchIC2renderBuffer();
    // cvDacBx.switchIC2renderBuffer();
    // cvDacCx.switchIC2renderBuffer();

    cvDacA.setLatchPin();
    cvDacB.setLatchPin();
    cvDacC.setLatchPin();

    // out DacB and DacC gets automatially triggered by flags when transmission is done
    cvDacA.fastUpdate();
    FlagHandler::cvDacAStarted = true;
    // cvDacAx.fastUpdate();
    // FlagHandler::cvDacAxStarted = true;
}

// cv rendering timer IRQ
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim15) {

        cvDacA.resetLatchPin();
        cvDacB.resetLatchPin();
        cvDacC.resetLatchPin();
        // if (FlagHandler::cvDacCFinished == false) {
        //     PolyError_Handler("polyRender | timerCallback | cvDacCFinished = false");
        // }

        FlagHandler::renderNewCV = true;
        FlagHandler::cvDacCFinished = false;
        FlagHandler::cvDacCxFinished = false;

        // waste just a little bit of time for ldacs to update properly
        volatile uint32_t count = 0;
        for (uint32_t i = 0; i < 8; i++) {
            count++;
        }
        sendDACs();
    }
}

void testMCPI2CAddress() {

    // set latch pins high

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);

    // check i2c ready
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        println("I2C AddressChange - I2C Busy");
        return;
    }

    // if (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // if (HAL_I2C_GetState(&hi2c3) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // test Address reachable -> set target address

    uint8_t addressFailed = 0;

    addressFailed |= cvDacA.testAddress();
    addressFailed |= cvDacB.testAddress();
    addressFailed |= cvDacC.testAddress();

    if (addressFailed) { // reset all MCP4728 address to start clean and reassign every one;
        resetMCPI2CAddress();

        sendI2CAddressUpdate(i2c1Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, 0x00, 0x01);
        // sendI2CAddressUpdate(i2c2Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, 0x00, 0x01);
        // sendI2CAddressUpdate(i2c3Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, 0x00, 0x01);

        sendI2CAddressUpdate(i2c1Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, 0x00, 0x02);
        // sendI2CAddressUpdate(i2c2Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, 0x00, 0x02);
        // sendI2CAddressUpdate(i2c3Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, 0x00, 0x02);

        sendI2CAddressUpdate(i2c1Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, 0x00, 0x03);
        // sendI2CAddressUpdate(i2c2Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, 0x00, 0x03);
        // sendI2CAddressUpdate(i2c3Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, 0x00, 0x03);
    }
}

void resetMCPI2CAddress() {

    // set latch pins high

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);

    // check i2c ready
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        println("I2C AddressChange - I2C Busy");
        return;
    }

    // if (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // if (HAL_I2C_GetState(&hi2c3) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // test Address reachable -> set target address
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_1_GPIO_Port, LDAC_1_Pin, address, 0x00);
    }
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_2_GPIO_Port, LDAC_2_Pin, address, 0x00);
    }
    for (uint8_t address = 0x00; address < 0x08; address++) {
        sendI2CAddressUpdate(i2c1Pins, LDAC_3_GPIO_Port, LDAC_3_Pin, address, 0x00);
    }
}
