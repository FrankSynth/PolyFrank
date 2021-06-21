#include "polyRender.hpp"
#include "render/renderAudioDef.h"

/// LAYER
ID layerId;
Layer layerA(layerId.getNewId());

// InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMABuffer[2 * INTERCHIPBUFFERSIZE]);
COMinterChip layerCom;

// CV DACS
RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABuffer[ALLDACS][4]);

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

void PolyRenderInit() {

    // CV DACs

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);

    testMCPI2CAddress(); // check all MCP4728 addressing

    for (uint16_t i = 0; i < ALLDACS; i++) {
        cvDac[i].init();
    }

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
    // switchLadder.disableChannels();
}

void PolyRenderRun() {
    // enable reception line
    HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);

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
        if (FlagHandler::cvDacStarted[0]) {
            cvDac[1].fastUpdate();
            FlagHandler::cvDacStarted[0] = false;
            FlagHandler::cvDacStarted[1] = true;
        }
        else if (FlagHandler::cvDacStarted[1]) {
            cvDac[2].fastUpdate();
            FlagHandler::cvDacStarted[1] = false;
            FlagHandler::cvDacStarted[2] = true;
        }
        else if (FlagHandler::cvDacStarted[2]) {
            cvDac[3].fastUpdate();
            FlagHandler::cvDacStarted[2] = false;
            FlagHandler::cvDacStarted[3] = true;
        }
        else if (FlagHandler::cvDacStarted[3]) {
            FlagHandler::cvDacLastFinished[0] = true;
        }
    }

    else if (hi2c->Instance == hi2c2.Instance) {
        if (FlagHandler::cvDacStarted[4]) {
            cvDac[5].fastUpdate();
            FlagHandler::cvDacStarted[4] = false;
            FlagHandler::cvDacStarted[5] = true;
        }
        else if (FlagHandler::cvDacStarted[5]) {
            cvDac[6].fastUpdate();
            FlagHandler::cvDacStarted[5] = false;
            FlagHandler::cvDacStarted[6] = true;
        }
        else if (FlagHandler::cvDacStarted[6]) {
            FlagHandler::cvDacLastFinished[1] = true;
        }
    }
    else if (hi2c->Instance == hi2c3.Instance) {
        if (FlagHandler::cvDacStarted[7]) {
            cvDac[8].fastUpdate();
            FlagHandler::cvDacStarted[7] = false;
            FlagHandler::cvDacStarted[8] = true;
        }
        else if (FlagHandler::cvDacStarted[8]) {
            cvDac[9].fastUpdate();
            FlagHandler::cvDacStarted[8] = false;
            FlagHandler::cvDacStarted[9] = true;
        }
        else if (FlagHandler::cvDacStarted[9]) {
            FlagHandler::cvDacLastFinished[2] = true;
        }
    }
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

    if (pin == GPIO_PIN_3) {
        // disable reception line
        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_RESET);
        // rising flank
        FlagHandler::interChipReceive_DMA_Finished = true;
    }
}

inline void sendDACs() {
    for (uint16_t i = 0; i < ALLDACS; i++) {
        cvDac[0].switchIC2renderBuffer();
    }

    // just those 4 will set all as they are shared
    cvDac[0].setLatchPin();
    cvDac[1].setLatchPin();
    cvDac[2].setLatchPin();
    cvDac[3].setLatchPin();

    // out DacB and DacC gets automatially triggered by flags when transmission is done
    cvDac[0].fastUpdate();
    FlagHandler::cvDacStarted[0] = true;
    cvDac[4].fastUpdate();
    FlagHandler::cvDacStarted[4] = true;
    cvDac[7].fastUpdate();
    FlagHandler::cvDacStarted[7] = true;
}

// cv rendering timer IRQ
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim15) {

        // just those 4 will set all as they are shared
        cvDac[0].resetLatchPin();
        cvDac[1].resetLatchPin();
        cvDac[2].resetLatchPin();
        cvDac[3].resetLatchPin();

        // error callback maybe
        if (FlagHandler::cvDacLastFinished[0] == false) {
            PolyError_Handler("polyRender | timerCallback | cvDacLastFinished[0] false");
        }
        if (FlagHandler::cvDacLastFinished[1] == false) {
            PolyError_Handler("polyRender | timerCallback | cvDacLastFinished[1] false");
        }
        if (FlagHandler::cvDacLastFinished[2] == false) {
            PolyError_Handler("polyRender | timerCallback | cvDacLastFinished[2] false");
        }

        FlagHandler::renderNewCV = true;
        FlagHandler::cvDacLastFinished[0] = false;
        FlagHandler::cvDacLastFinished[1] = false;
        FlagHandler::cvDacLastFinished[2] = false;

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
    HAL_GPIO_WritePin(LDAC_4_GPIO_Port, LDAC_4_Pin, GPIO_PIN_SET);

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

    for (uint16_t i = 0; i < ALLDACS; i++) {
        addressFailed |= cvDac[i].testAddress();
    }

    if (addressFailed) { // reset all MCP4728 address to start clean and reassign every one;
        resetMCPI2CAddress();

        // TODO do it, make it work

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

    // if (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // if (HAL_I2C_GetState(&hi2c3) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // TODO do it, make it work ADD BUSSES
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
