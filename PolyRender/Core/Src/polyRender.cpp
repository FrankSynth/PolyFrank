#include "polyRender.hpp"
#include "hardware/device.hpp"
#include "render/renderAudioDef.h"

// #include <cstdlib>

/// LAYER
ID layerId;
Layer layerA(layerId.getNewId());

extern devManager deviceManager;
spiBus spiBusLayer;

uint8_t sendString(const char *message);
uint8_t sendString(std::string &message);
uint8_t sendString(std::string &&message);

// InterChip Com
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);
RAM2_DMA ALIGN_32BYTES(volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)]);

// CV DACS
RAM2_DMA ALIGN_32BYTES(volatile uint16_t cvDacDMABuffer[ALLDACS][4]);

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

void sendDACs();

void PolyRenderInit() {

    loadInitialWavetables();

    HAL_Delay(100);

    spiBusLayer.connectToInterface(&hspi1);
    deviceManager.addBus(&spiBusLayer);

    // TODO chip ID maybe also could be sent instead of read.
    // set chip id
    layerA.chipID = !HAL_GPIO_ReadPin(CHIP_ID_A_GPIO_Port, CHIP_ID_A_Pin);

    // TODO layer id should maybe rather be sent
    // set layer ID
    layerA.id = !HAL_GPIO_ReadPin(CHIP_ID_B_GPIO_Port, CHIP_ID_B_Pin);

    // init pseudo rand so all chips follow different patterns.
    std::srand(layerA.chipID + layerA.id + 1);

    // CV DACs init
    initCVRendering();

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
    FlagHandler::renderNewCVFunc = renderCVs;
}

void PolyRenderRun() {

    println("////////// Hi, it's Render. PolyFrank Render. //////////");
    println("Layer ID is: ", layerA.id);
    println("Chip ID is: ", layerA.chipID);

    // enable reception line
    // HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);

    // start audio rendering
    renderAudio((int32_t *)saiBuffer);
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));
    audioDacA.startSAI();

    // start cv rendering
    renderCVs();
    sendDACs();

    HAL_TIM_Base_Start_IT(&htim15);

    elapsedMillis askMessage = 0;

    // run loop
    while (true) {
        // for timing test purpose
        // if (askMessage > 5000) {
        //     println("alive");
        //     askMessage = 0;
        // }
        FlagHandler::handleFlags();
    }
}

// CALLBACKS

// cv DACs Send Callbacks
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {

    if (hi2c->Instance == hi2c1.Instance) {

        if (FlagHandler::cvDacStarted[0]) {
            // microsecondsDelay(rand);

            cvDac[1].setLatchPin();
            FlagHandler::cvDacStarted[0] = false;
            FlagHandler::cvDacFinished[0] = true;

            FlagHandler::cvDacStarted[1] = true;
            cvDac[1].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[1]) {
            // microsecondsDelay(rand);

            cvDac[2].setLatchPin();
            FlagHandler::cvDacStarted[1] = false;
            FlagHandler::cvDacFinished[1] = true;

            FlagHandler::cvDacStarted[2] = true;
            cvDac[2].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[2]) {
            cvDac[3].setLatchPin();
            FlagHandler::cvDacStarted[2] = false;
            FlagHandler::cvDacFinished[2] = true;

            FlagHandler::cvDacStarted[3] = true;
            cvDac[3].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[3]) {
            FlagHandler::cvDacFinished[3] = true;

            FlagHandler::cvDacLastFinished[0] = true;
        }
    }

    else if (hi2c->Instance == hi2c2.Instance) {
        if (FlagHandler::cvDacStarted[4]) {
            cvDac[5].setLatchPin();
            FlagHandler::cvDacStarted[4] = false;
            FlagHandler::cvDacFinished[4] = true;

            FlagHandler::cvDacStarted[5] = true;
            cvDac[5].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[5]) {
            cvDac[6].setLatchPin();
            FlagHandler::cvDacStarted[5] = false;
            FlagHandler::cvDacFinished[5] = true;

            FlagHandler::cvDacStarted[6] = true;
            cvDac[6].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[6]) {
            FlagHandler::cvDacLastFinished[1] = true;
        }
    }
    else if (hi2c->Instance == hi2c3.Instance) {

        if (FlagHandler::cvDacStarted[7]) {

            cvDac[8].setLatchPin();
            FlagHandler::cvDacStarted[7] = false;
            FlagHandler::cvDacFinished[7] = true;

            FlagHandler::cvDacStarted[8] = true;
            cvDac[8].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[8]) {
            // microsecondsDelay(rand);

            cvDac[9].setLatchPin();
            FlagHandler::cvDacStarted[8] = false;
            FlagHandler::cvDacFinished[8] = true;

            FlagHandler::cvDacStarted[9] = true;
            cvDac[9].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[9]) {
            FlagHandler::cvDacFinished[9] = true;

            FlagHandler::cvDacLastFinished[2] = true;
        }
    }

    if (FlagHandler::cvDacFinished[0] && FlagHandler::cvDacFinished[4] && FlagHandler::cvDacFinished[7])
        cvDac[0].resetLatchPin();

    if (FlagHandler::cvDacFinished[1] && FlagHandler::cvDacFinished[5] && FlagHandler::cvDacFinished[8])
        cvDac[1].resetLatchPin();

    if (FlagHandler::cvDacFinished[2] && FlagHandler::cvDacFinished[6] && FlagHandler::cvDacFinished[9])
        cvDac[2].resetLatchPin();

    if (FlagHandler::cvDacFinished[3])
        cvDac[3].resetLatchPin();
}

elapsedMicros audiotimer = 0;
uint32_t counter = 0;
uint32_t cache = 0;

// Audio Render Callbacks
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    // HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
    audiotimer = 0;
    renderAudio((int32_t *)&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));

    cache += audiotimer;
    counter++;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    // HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
    audiotimer = 0;
    renderAudio((int32_t *)saiBuffer);

    cache += audiotimer;
    counter++;

    if (counter > 5000) {
        sendString(std::to_string((float)cache / (float)counter));

        counter = 0;
        cache = 0;
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai) {
    PolyError_Handler("SAI error callback");
}

// reception line callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    if (pin == SPI_CS_fromControl_Pin) {

        if (layerCom.spi->state == BUS_SEND) {
            HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_RESET);
            layerCom.spi->callTxComplete();
        }

        if (layerCom.spi->state == BUS_RECEIVE) {
            // disable reception line
            HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_RESET);
            layerCom.spi->stopDMA();
            layerCom.spi->callRxComplete();
            layerCom.decodeCurrentInBuffer();
        }
        else if (layerCom.spi->state == BUS_READY && layerCom.state == COM_READY) {
            if (layerCom.beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | receive bus occuppied");
        }
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // InterChip Com
    // if (hspi == layerCom.spi->hspi) {
    //     layerCom.spi->callTxComplete();
    // }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    // InterChip Com
    // if (hspi == layerCom.spi->hspi) {
    //     layerCom.spi->callTxComplete();
    // }
    PolyError_Handler("ERROR | FATAL | SPI Error");
}

void sendDACs() {
    for (uint16_t i = 0; i < ALLDACS; i++) {
        cvDac[0].switchIC2renderBuffer();
    }
    FlagHandler::renderNewCV = true;
    FlagHandler::cvDacLastFinished[0] = false;
    FlagHandler::cvDacLastFinished[1] = false;
    FlagHandler::cvDacLastFinished[2] = false;

    for (int i = 0; i < 10; i++)
        FlagHandler::cvDacFinished[0] = false;

    // just those 4 will set all as they are shared
    cvDac[0].setLatchPin();

    // out DacB and DacC gets automatially triggered by flags when transmission is done
    FlagHandler::cvDacStarted[0] = true;
    cvDac[0].fastUpdate();
    FlagHandler::cvDacStarted[4] = true;
    cvDac[4].fastUpdate();
    FlagHandler::cvDacStarted[7] = true;
    cvDac[7].fastUpdate();
}

// cv rendering timer IRQ
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim15) {
        if (FlagHandler::cvDacLastFinished[2] == false) {
            PolyError_Handler("polyRender | timerCallback | cvDacLastFinished[3] false");
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

uint8_t sendString(std::string &message) {
    return layerCom.sendString(message);
}
uint8_t sendString(std::string &&message) {
    return layerCom.sendString(message);
}
uint8_t sendString(const char *message) {
    return layerCom.sendString(message);
}
uint8_t sendOutput(uint8_t modulID, uint8_t settingID, int32_t amount) {
    return layerCom.sendOutput(modulID, settingID, amount);
}
uint8_t sendOutput(uint8_t modulID, uint8_t settingID, float amount) {
    return layerCom.sendOutput(modulID, settingID, amount);
}
uint8_t sendInput(uint8_t modulID, uint8_t settingID, float amount) {
    return layerCom.sendInput(modulID, settingID, amount);
}