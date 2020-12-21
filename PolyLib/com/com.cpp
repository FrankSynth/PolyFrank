#include "com.hpp"

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// COMusb //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

const uint8_t COMusb::read() {
    uint8_t data = rBuffer.front();
    rBuffer.pop_front();
    return data;
}

uint8_t COMusb::write(uint8_t data) {
    wBuffer[writeBufferSelect].push_back(data);

    return 0; // might be used for error codes
}

uint8_t COMusb::write(uint8_t *data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        wBuffer[writeBufferSelect].push_back(data[i]);
    }
    return 0; // might be used for error codes
}

uint8_t COMusb::beginUSBTransmission() {
    uint8_t ret = sendViaUSB(wBuffer[writeBufferSelect].data(), wBuffer[writeBufferSelect].size());

    if (ret == 0) {
        writeBufferSelect = !writeBufferSelect; // switch out buffers
        wBuffer[writeBufferSelect].clear();
        return 0; // return ok
    }
    else { // return error codes
        return ret;
    }
}

uint8_t COMusb::push(uint8_t *data, uint32_t length) {

    for (uint32_t i = 0; i < length; i++) {
        // println(data[i]);
        if (rBuffer.push_back(data[i])) {
            return 1;
        }
    }
    return 0;
}

uint8_t COMusb::push(uint8_t data) {
    return rBuffer.push_back(data);
}

/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// COMinterChip //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void comMDMACallback(MDMA_HandleTypeDef *_hmdma);
void comMDMACallbackError(MDMA_HandleTypeDef *_hmdma);

#ifdef POLYCONTROL

uint8_t COMinterChip::beginSendTransmission() {
    // buffers empty
    if ((outBufferChipA[currentBufferSelect].size() + outBufferChipB[currentBufferSelect].size()) < 5) {
        // println("buffer empty, skip send");
        return 0;
    }

    // block new transmissions as long as a transmission is already running
    if (blockNewSendBeginCommand) {
        println("ERRORCODE_SENDBLOCK");
        return ERRORCODE_SENDBLOCK;
    }

    if (!(HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_1_Pin)
          // TODO enable other layers when ready, or Pull Ups on not used pins
          //   && HAL_GPIO_ReadPin(Layer_1_READY_2_GPIO_Port, Layer_1_READY_2_Pin) &&
          //   HAL_GPIO_ReadPin(Layer_2_READY_1_GPIO_Port, Layer_2_READY_1_Pin) &&
          //   HAL_GPIO_ReadPin(Layer_2_READY_2_GPIO_Port, Layer_2_READY_2_Pin)
          )) {
        // println("ERRORCODE_RECEPTORNOTREADY");

        return ERRORCODE_RECEPTORNOTREADY;
    }

    // TODO enable later
    // second Layer active?
    // if (globalSettings.amountLayers.value) {
    //     if (!(HAL_GPIO_ReadPin(Layer_2_READY_1_GPIO_Port, Layer_2_READY_1_Pin) &&
    //           HAL_GPIO_ReadPin(Layer_2_READY_2_GPIO_Port, Layer_2_READY_2_Pin))) {
    //         return ERRORCODE_RECEPTORNOTREADY;
    //     }
    // }

    // close both buffers
    appendLastByte();

    // size rounded up to word length
    // dmaOutCurrentBufferASize = (outBufferChipA[currentBufferSelect].size() | 0x0004) + 1;
    // dmaOutCurrentBufferBSize = (outBufferChipB[currentBufferSelect].size() | 0x0004) + 1;

    dmaOutCurrentBufferASize = outBufferChipA[currentBufferSelect].size();

    // write size into first two bytes of outBuffers
    *(uint16_t *)outBufferChipA[currentBufferSelect].data() = dmaOutCurrentBufferASize;
    *(uint16_t *)outBufferChipB[currentBufferSelect].data() = dmaOutCurrentBufferBSize;
    // println("register flaghandlers interChipA_MDMA_Started");

    FlagHandler::interChipA_MDMA_Started[layer] = 1;
    FlagHandler::interChipA_MDMA_FinishedFunc[layer] = std::bind(&COMinterChip::startFirstDMA, this);

    switchBuffer();

    //println("send buffer size", dmaOutCurrentBufferASize);

    HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID, comMDMACallback);
    HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_ERROR_CB_ID, comMDMACallbackError);
    prepareMDMAHandle();

    uint8_t ret = HAL_MDMA_Start_IT(&hmdma_mdma_channel40_sw_0, (uint32_t)outBufferChipA[!currentBufferSelect].data(),
                                    (uint32_t)dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferASize, 1);

    if (ret) {
        FlagHandler::interChipA_MDMA_Started[layer] = 0;
        FlagHandler::interChipA_MDMA_FinishedFunc[layer] = nullptr;
        HAL_MDMA_UnRegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID);
    }
    else {
        blockNewSendBeginCommand = 1;
    }
    return ret;
}

void COMinterChip::initOutTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaTransmitFunc, uint8_t *dmaBuffer,
                                       uint8_t layer) {

    sendViaDMA = dmaTransmitFunc;

    this->layer = layer;

    dmaOutBufferPointer[0] = dmaBuffer;
    dmaOutBufferPointer[1] = dmaBuffer + INTERCHIPBUFFERSIZE;

    outBufferChipA[0].reserve(INTERCHIPBUFFERSIZE);
    outBufferChipA[1].reserve(INTERCHIPBUFFERSIZE);
    outBufferChipB[0].reserve(INTERCHIPBUFFERSIZE);
    outBufferChipB[1].reserve(INTERCHIPBUFFERSIZE);

    pushDummySizePlaceHolder();
}

uint8_t COMinterChip::startFirstDMA() {
    // println("startFirstDMA");

    // enable NSS to Render Chip A
    if (layer == 0)
        HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_RESET);

    FlagHandler::interChipA_DMA_Started[layer] = 1;

    // TODO remove single chip temp setting, switch following lines
    FlagHandler::interChipA_DMA_FinishedFunc[layer] = std::bind(&COMinterChip::sendTransmissionSuccessfull, this);
    // FlagHandler::interChipA_DMA_FinishedFunc[layer] = std::bind(&COMinterChip::startSecondMDMA, this);

    uint8_t ret = sendViaDMA(dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferASize);
    if (ret) {
        FlagHandler::interChipA_DMA_Started[layer] = 0;
        FlagHandler::interChipA_DMA_FinishedFunc[layer] = nullptr;
    }
    return ret;
}

uint8_t COMinterChip::startSecondMDMA() {

    prepareMDMAHandle();
    HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID, comMDMACallback);
    FlagHandler::interChipB_MDMA_Started[layer] = 1;
    FlagHandler::interChipB_MDMA_FinishedFunc[layer] = std::bind(&COMinterChip::startSecondDMA, this);
    uint8_t ret = HAL_MDMA_Start_IT(&hmdma_mdma_channel40_sw_0, (uint32_t)outBufferChipB[!currentBufferSelect].data(),
                                    (uint32_t)dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferBSize, 1);
    if (ret) {
        FlagHandler::interChipB_MDMA_Started[layer] = 0;
        FlagHandler::interChipB_MDMA_FinishedFunc[layer] = nullptr;
        HAL_MDMA_UnRegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID);
    }
    return ret;
}

uint8_t COMinterChip::startSecondDMA() {

    // enable NSS to Render Chip B
    if (layer == 0)
        HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_RESET);

    FlagHandler::interChipB_DMA_Started[layer] = 1;
    FlagHandler::interChipB_DMA_FinishedFunc[layer] = std::bind(&COMinterChip::sendTransmissionSuccessfull, this);

    uint8_t ret = sendViaDMA(dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferASize);
    if (ret) {
        FlagHandler::interChipB_DMA_Started[layer] = 0;
        FlagHandler::interChipB_DMA_FinishedFunc[layer] = nullptr;
    }
    return ret;
}

uint8_t COMinterChip::sendTransmissionSuccessfull() {
    if (layer == 0) {
        HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);
    }
    //println("Transmission success!!!");
    blockNewSendBeginCommand = 0;
    return 0;
}

// copy the received SPI via DMA data using the MDMA to the DTCM Ram

uint8_t COMinterChip::pushOutBufferChipA(uint8_t data) {
    if (outBufferChipA[currentBufferSelect].size() >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }
    outBufferChipA[currentBufferSelect].push_back(data);
    return 0;
}

uint8_t COMinterChip::pushOutBufferChipA(uint8_t *data, uint32_t length) {
    if (outBufferChipA[currentBufferSelect].size() + length >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    for (uint32_t i = 0; i < length; i++) {
        // println(data[i]);
        outBufferChipA[currentBufferSelect].push_back(data[i]);
    }
    return 0;
}

uint8_t COMinterChip::pushOutBufferChipB(uint8_t data) {
    if (outBufferChipB[currentBufferSelect].size() >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }
    outBufferChipB[currentBufferSelect].push_back(data);
    return 0;
}

uint8_t COMinterChip::pushOutBufferChipB(uint8_t *data, uint32_t length) {
    if (outBufferChipB[currentBufferSelect].size() + length >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    for (uint32_t i = 0; i < length; i++) {
        // println(data[i]);
        outBufferChipB[currentBufferSelect].push_back(data[i]);
    }
    return 0;
}

uint8_t COMinterChip::invokeBufferFullSend() {
    uint8_t ret = beginSendTransmission();
    while (ret == ERRORCODE_SENDBLOCK || ret == ERRORCODE_RECEPTORNOTREADY) {
        ret = beginSendTransmission();
    }
    return ret;
}

// Patch functions
uint8_t COMinterChip::sendCreatePatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[CREATEINOUTPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | CREATEINOUTPATCH;
    comCommand[1] = outputId;
    comCommand[2] = inputId;
    *(float *)(&comCommand[3]) = amount;

    pushOutBufferChipA(comCommand, CREATEINOUTPATCHCMDSIZE);
    pushOutBufferChipB(comCommand, CREATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendUpdatePatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[UPDATEINOUTPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | UPDATEINOUTPATCH;
    comCommand[1] = outputId;
    comCommand[2] = inputId;
    *(float *)(&comCommand[3]) = amount;

    pushOutBufferChipA(comCommand, UPDATEINOUTPATCHCMDSIZE);
    pushOutBufferChipB(comCommand, UPDATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeletePatchInOut(uint8_t outputId, uint8_t inputId) {
    uint8_t comCommand[DELETEPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | DELETEINOUTPATCH;
    comCommand[1] = outputId;
    comCommand[2] = inputId;

    pushOutBufferChipA(comCommand, DELETEPATCHCMDSIZE);
    pushOutBufferChipB(comCommand, DELETEPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendCreatePatchOutOut(uint8_t outputOutId, uint8_t OutputInId, float amount, float offset) {
    uint8_t comCommand[CREATEOUTOUTPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | CREATEOUTOUTPATCH;
    comCommand[1] = outputOutId;
    comCommand[2] = OutputInId;
    *(float *)(&comCommand[3]) = amount;
    *(float *)(&comCommand[7]) = offset;

    pushOutBufferChipA(comCommand, CREATEOUTOUTPATCHCMDSIZE);
    pushOutBufferChipB(comCommand, CREATEOUTOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendUpdatePatchOutOut(uint8_t outputOutId, uint8_t OutputInId, float amount, float offset) {
    uint8_t comCommand[UPDATEOUTOUTPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | UPDATEOUTOUTPATCH;
    comCommand[1] = outputOutId;
    comCommand[2] = OutputInId;
    *(float *)(&comCommand[3]) = amount;
    *(float *)(&comCommand[7]) = offset;

    pushOutBufferChipA(comCommand, UPDATEOUTOUTPATCHCMDSIZE);
    pushOutBufferChipB(comCommand, UPDATEOUTOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeletePatchOutOut(uint8_t outputOutId, uint8_t OutputInId) {
    uint8_t comCommand[DELETEPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | DELETEOUTOUTPATCH;
    comCommand[1] = outputOutId;
    comCommand[2] = OutputInId;

    pushOutBufferChipA(comCommand, DELETEPATCHCMDSIZE);
    pushOutBufferChipB(comCommand, DELETEPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeleteAllPatches() {
    uint8_t comCommand[DELETEALLPATCHESCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | DELETEALLPATCHES;

    pushOutBufferChipA(comCommand[0]);
    pushOutBufferChipB(comCommand[0]);
    return 0;
}

// this function now dows all settings, int & float settings
uint8_t COMinterChip::sendSetting(uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = SETTINGTYPE | (modulID << 3);
    comCommand[1] = UPDATESETTINGINT | settingID;
    *(int32_t *)(&comCommand[2]) = amount;

    pushOutBufferChipA(comCommand, SETTINGCMDSIZE);
    pushOutBufferChipB(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = SETTINGTYPE | (modulID << 3);
    comCommand[1] = UPDATESETTINGFLOAT | settingID;
    *(float *)(&comCommand[2]) = amount;

    pushOutBufferChipA(comCommand, SETTINGCMDSIZE);
    pushOutBufferChipB(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendNewNote(uint8_t modulID, uint8_t voiceID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[NEWNOTECMDSIZE];
    uint8_t voiceIDsend = voiceID;

    if (voiceIDsend != VOICEALL)
        voiceIDsend %= 4;

    comCommand[0] = SETTINGTYPE | (modulID << 3) | voiceIDsend;
    comCommand[1] = UPDATESETTINGINT | settingID;
    *(int32_t *)(&comCommand[2]) = amount;
    comCommand[6] = OPENGATE | voiceIDsend;

    if (voiceID < 4)
        pushOutBufferChipA(comCommand, NEWNOTECMDSIZE);
    else if (voiceID < 8)
        pushOutBufferChipB(comCommand, NEWNOTECMDSIZE);
    else {
        pushOutBufferChipA(comCommand, NEWNOTECMDSIZE);
        pushOutBufferChipB(comCommand, NEWNOTECMDSIZE);
    }
    return 0;
}

uint8_t COMinterChip::sendOpenGate(uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];
    uint8_t voiceIDsend = voiceID;

    if (voiceIDsend != VOICEALL)
        voiceIDsend %= 4;

    comCommand[0] = PATCHCMDTYPE | OPENGATE | voiceIDsend;

    if (voiceID < 4)
        pushOutBufferChipA(comCommand[0]);
    else if (voiceID < 8)
        pushOutBufferChipB(comCommand[0]);
    else {
        pushOutBufferChipA(comCommand[0]);
        pushOutBufferChipB(comCommand[0]);
    }
    return 0;
}

uint8_t COMinterChip::sendCloseGate(uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];
    uint8_t voiceIDsend = voiceID;

    if (voiceIDsend != VOICEALL)
        voiceIDsend %= 4;

    comCommand[0] = PATCHCMDTYPE | CLOSEGATE | voiceIDsend;

    if (voiceID < 4)
        pushOutBufferChipA(comCommand[0]);
    else if (voiceID < 8)
        pushOutBufferChipB(comCommand[0]);
    else {
        pushOutBufferChipA(comCommand[0]);
        pushOutBufferChipB(comCommand[0]);
    }
    return 0;
}

uint8_t COMinterChip::sendRetrigger(uint8_t modulID, uint8_t voiceID) {
    uint8_t comCommand[RETRIGGERCMDSIZE];
    uint8_t voiceIDsend = voiceID;

    if (voiceIDsend != VOICEALL)
        voiceIDsend %= 4;

    comCommand[0] = SETTINGTYPE | (modulID << 3) | voiceIDsend;
    comCommand[1] = RETRIGGER;

    if (voiceID < 4)
        pushOutBufferChipA(comCommand, RETRIGGERCMDSIZE);
    else if (voiceID < 8)
        pushOutBufferChipB(comCommand, RETRIGGERCMDSIZE);
    else {
        pushOutBufferChipA(comCommand, RETRIGGERCMDSIZE);
        pushOutBufferChipB(comCommand, RETRIGGERCMDSIZE);
    }
    return 0;
}

uint8_t COMinterChip::sendResetAll() {
    uint8_t comCommand[RESETALLCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | RESETALL;

    pushOutBufferChipA(comCommand[0]);
    pushOutBufferChipB(comCommand[0]);
    return 0;
}

// wrap up out buffers
uint8_t COMinterChip::appendLastByte() {
    uint8_t comCommand[LASTBYTECMDSIZE];
    comCommand[0] = PATCHCMDTYPE | LASTBYTE;
    outBufferChipA[currentBufferSelect].push_back(comCommand[0]);
    outBufferChipB[currentBufferSelect].push_back(comCommand[0]);

    return 0;
}

// reserve place for size byte
void COMinterChip::pushDummySizePlaceHolder() {
    uint16_t dummysize = 0;
    pushOutBufferChipA((uint8_t *)&dummysize, 2);
    pushOutBufferChipB((uint8_t *)&dummysize, 2);
}

#elif POLYRENDER

void COMinterChip::initInTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaReceiveFunc,
                                      std::function<uint8_t()> dmaStopReceiveFunc, uint8_t *dmaBuffer) {
    receiveViaDMA = dmaReceiveFunc;
    stopReceiveViaDMA = dmaStopReceiveFunc;
    dmaInBufferPointer[0] = dmaBuffer;
    dmaInBufferPointer[1] = dmaBuffer + INTERCHIPBUFFERSIZE;
    inBufferPointer[0] = inBuffer;
    inBufferPointer[1] = inBuffer + INTERCHIPBUFFERSIZE;
}

uint8_t COMinterChip::beginReceiveTransmission() {

    // enable reception line
    HAL_GPIO_WritePin(SPI_Ready_toControl_GPIO_Port, SPI_Ready_toControl_Pin, GPIO_PIN_SET);

    uint8_t ret = receiveViaDMA(dmaInBufferPointer[!currentBufferSelect], INTERCHIPBUFFERSIZE);
    FlagHandler::interChipReceive_DMA_Started = 1;
    FlagHandler::interChipReceive_DMA_FinishedFunc = std::bind(&COMinterChip::copyReceivedInBuffer, this);
    return ret;
}

uint8_t COMinterChip::copyReceivedInBuffer() {

    // disable reception line
    HAL_GPIO_WritePin(SPI_Ready_toControl_GPIO_Port, SPI_Ready_toControl_Pin, GPIO_PIN_RESET);

    // as receive DMA is probably still running, disable it here
    stopReceiveViaDMA();

    // flush fifo buffer necessary?
    HAL_SPIEx_FlushRxFifo(&hspi1);

    // println("start in MDMA");
    // copy via MDMA into ITC Memory
    prepareMDMAHandle();
    HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID, comMDMACallback);

    FlagHandler::interChipReceive_MDMA_Started = 1;
    FlagHandler::interChipReceive_MDMA_FinishedFunc = std::bind(&COMinterChip::decodeCurrentInBuffer, this);

    uint8_t ret = HAL_MDMA_Start_IT(&hmdma_mdma_channel40_sw_0, (uint32_t)(dmaInBufferPointer[!currentBufferSelect]),
                                    (uint32_t)(inBufferPointer[!currentBufferSelect]), INTERCHIPBUFFERSIZE, 1);

    if (ret) {
        FlagHandler::interChipReceive_MDMA_Started = 0;
        FlagHandler::interChipReceive_MDMA_FinishedFunc = nullptr;
        HAL_MDMA_UnRegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID);
    }

    return ret;
}

uint8_t COMinterChip::decodeCurrentInBuffer() {
    switchBuffer();
    println("decode Buffer");
    // necessary decoding vars
    uint8_t outputID;
    uint8_t inputID;
    uint8_t currentByte;
    uint8_t setting;
    uint8_t modul;
    uint8_t voice;
    int32_t amountInt;
    float amountFloat;
    float offsetFloat;

    beginReceiveTransmission();

    // assemble size
    uint16_t sizeOfReadBuffer = *(uint16_t *)(inBufferPointer[currentBufferSelect]);
    println("Buffer size: ", sizeOfReadBuffer);

    if (sizeOfReadBuffer > INTERCHIPBUFFERSIZE) {
        // buffer too big, sth went wrong
        Error_Handler();
        return 1;
    }

    // start with offset, as two bytes were size
    for (uint16_t i = 2; i < sizeOfReadBuffer; i++) {
        currentByte = (inBufferPointer[currentBufferSelect])[i];
        println("currentByte: ", currentByte);

        switch (currentByte & CMD_TYPEMASK) {

            // first byte is is Patch/Command
            case PATCHCMDTYPE:

                switch (currentByte & CMD_PATCHMASK) {

                    case UPDATEINOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        allLayers[0]->updatePatchInOut(outputID, inputID, amountFloat);

                        break;

                    case CREATEINOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        allLayers[0]->addPatchInOut(outputID, inputID, amountFloat);

                        break;

                    case DELETEINOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];

                        allLayers[0]->removePatchInOut(outputID, inputID);

                        break;
                    case UPDATEOUTOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;
                        offsetFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        allLayers[0]->updatePatchOutOut(outputID, inputID, amountFloat);

                        break;

                    case CREATEOUTOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;
                        offsetFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        allLayers[0]->addPatchOutOut(outputID, inputID, amountFloat);

                        break;

                    case DELETEOUTOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];

                        allLayers[0]->removePatchOutOut(outputID, inputID);

                        break;
                    case DELETEALLPATCHES:
                        // delete all patchesInOut at once
                        allLayers[0]->clearPatches();
                        break;

                    case OPENGATE:
                        voice = currentByte & CMD_VOICEMASK;
                        // openGate(voice)
                        break;

                    case CLOSEGATE:
                        voice = currentByte & CMD_VOICEMASK;
                        // closeGate(voice)
                        break;

                    case CLOCK:
                        // clock()
                        break;

                    case RESETALL:
                        // resetAll()
                        break;

                    case LASTBYTE:
                        // transmission complete
                        println("decode success!");
                        return 0;

                    default:
                        // seomething went wrong here
                        Error_Handler();
                        return 1;
                }
                break;

            // first byte is setting
            case SETTINGTYPE:

                // save infos
                modul = (currentByte & CMD_MODULEMASK) >> 3;
                voice = currentByte & CMD_VOICEMASK;

                // get next byte
                currentByte = (inBufferPointer[currentBufferSelect])[++i];
                // second byte setting
                switch (currentByte & CMD_PATCHMASK) {
                    case UPDATESETTINGINT:
                        setting = currentByte & CMD_SETTINGSMASK;
                        amountInt = *(int32_t *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(int32_t) - 1;

                        allLayers[0]->getModules()[modul]->getSwitches()[setting]->setValueWithoutMapping(amountInt);

                        println("Modul: ", modul, ", Setting: ", setting, ", Amount: ", amountInt);

                        break;
                    case UPDATESETTINGFLOAT:
                        setting = currentByte & CMD_SETTINGSMASK;
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        allLayers[0]->getModules()[modul]->getPotis()[setting]->setValueWithoutMapping(amountFloat);

                        println("Modul: ", modul, ", Setting: ", setting, ", Amount: ", amountFloat);

                        break;
                    case RETRIGGER:

                        // retrigger(modul, voice)
                        break;

                    default:
                        // something went wrong here
                        Error_Handler();
                        return 1;
                }
                break;
            default:
                // seomething went wrong here
                Error_Handler();
                return 1;
        }
    }

    // surpress compiler warnings while not used
    UNUSED(setting);
    UNUSED(modul);
    UNUSED(voice);
    UNUSED(amountFloat);
    UNUSED(amountInt);
    UNUSED(offsetFloat);
    UNUSED(outputID);
    UNUSED(inputID);

    // we should always exit with LASTBYTE
    Error_Handler();
    return 1;
}

#endif

void COMinterChip::prepareMDMAHandle() {
    hmdma_mdma_channel40_sw_0.Instance = MDMA_Channel0;
    hmdma_mdma_channel40_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel40_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel40_sw_0.Init.Priority = MDMA_PRIORITY_HIGH;
    hmdma_mdma_channel40_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    // FIXME polycontrol compatibility
#ifdef POLYRENDER
    hmdma_mdma_channel40_sw_0.Init.SourceInc = MDMA_SRC_INC_WORD;
    hmdma_mdma_channel40_sw_0.Init.DestinationInc = MDMA_DEST_INC_WORD;
    hmdma_mdma_channel40_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
    hmdma_mdma_channel40_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
#elif POLYCONTROL
    hmdma_mdma_channel40_sw_0.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_mdma_channel40_sw_0.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    hmdma_mdma_channel40_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_mdma_channel40_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
#endif
    hmdma_mdma_channel40_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel40_sw_0.Init.BufferTransferLength = 64;
    hmdma_mdma_channel40_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_mdma_channel40_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_mdma_channel40_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel40_sw_0.Init.DestBlockAddressOffset = 0;

    HAL_MDMA_Init(&hmdma_mdma_channel40_sw_0);
}

void COMinterChip::switchBuffer() {
    // println("com buffer switched");
    currentBufferSelect = !currentBufferSelect;

#ifdef POLYCONTROL
    outBufferChipA[currentBufferSelect].clear();
    outBufferChipB[currentBufferSelect].clear();
    pushDummySizePlaceHolder();
#endif
}

void comMDMACallback(MDMA_HandleTypeDef *_hmdma) {
    // println("HAL_MDMA_XFER_CPLT_CB_ID");

    HAL_MDMA_UnRegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID);

#ifdef POLYCONTROL
    for (uint8_t i = 0; i < 2; i++) {

        if (FlagHandler::interChipA_MDMA_Started[i] == 1) {
            FlagHandler::interChipA_MDMA_Started[i] = 0;
            FlagHandler::interChipA_MDMA_Finished[i] = 1;
        }
        if (FlagHandler::interChipB_MDMA_Started[i] == 1) {
            FlagHandler::interChipB_MDMA_Started[i] = 0;
            FlagHandler::interChipB_MDMA_Finished[i] = 1;
        }
    }

#elif POLYRENDER

    if (FlagHandler::interChipReceive_MDMA_Started == 1) {
        FlagHandler::interChipReceive_MDMA_Started = 0;
        FlagHandler::interChipReceive_MDMA_Finished = 1;
    }
#endif
}

void comMDMACallbackError(MDMA_HandleTypeDef *_hmdma) {
    println("MDMA ERROR");
}
