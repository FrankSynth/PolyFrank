#include "com.hpp"
#include "layer/layer.hpp"

extern Layer layerA;

#ifdef POLYRENDER
#endif

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// COMusb //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
namespace midiUSB {

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
} // namespace midiUSB
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// COMDin //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

const uint8_t COMdin::read() {
    uint8_t data = rBuffer.front();
    rBuffer.pop_front();
    return data;
}

bool COMdin::available() {
    return !rBuffer.empty();
}

uint8_t COMdin::write(uint8_t data) {
    wBuffer[writeBufferSelect].push_back(data);

    return 0; // might be used for error codes
}

uint8_t COMdin::write(uint8_t *data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        wBuffer[writeBufferSelect].push_back(data[i]);
    }
    return 0; // might be used for error codes
}

uint8_t COMdin::push(uint8_t *data, uint32_t length) {

    for (uint32_t i = 0; i < length; i++) {
        // println(data[i]);
        if (rBuffer.push_back(data[i])) {
            return 1;
        }
    }
    return 0;
}

uint8_t COMdin::push(uint8_t data) {
    return rBuffer.push_back(data);
}
/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// COMinterChip //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void comMDMACallback(MDMA_HandleTypeDef *_hmdma);
void comMDMACallbackError(MDMA_HandleTypeDef *_hmdma);

#ifdef POLYCONTROL

uint8_t COMinterChip::beginSendTransmission() {

    if (sendViaDMA == nullptr) {
        return 0;
    }
    // buffers empty
    if (outBuffer[currentBufferSelect].size() < 2) {
        // println("buffer empty, skip send");
        return 0;
    }

    // block new transmissions as long as a transmission is already running
    if (blockNewSendBeginCommand) {
        return ERRORCODE_SENDBLOCK;
    }

    if (FlagHandler::interChipA_State[layer] != READY ||
        FlagHandler::interChipB_State[layer] != READY) { // layer not Ready
        return ERRORCODE_RECEPTORNOTREADY;
    }

    // close both buffers
    appendLastByte();
    dmaOutCurrentBufferSize = outBuffer[currentBufferSelect].size();
    // dmaOutCurrentBufferBSize = outBufferChipB[currentBufferSelect].size();

    // write size into first two bytes of outBuffers
    *outBuffer[currentBufferSelect].data() = dmaOutCurrentBufferSize;
    // outBufferChipB[currentBufferSelect].data() = dmaOutCurrentBufferBSize;
    // println("register flaghandlers interChipA_MDMA_Started");

    switchBuffer();

    HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID, comMDMACallback);
    HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_ERROR_CB_ID, comMDMACallbackError);
    prepareMDMAHandle();

    if (dmaOutCurrentBufferSize) {
        startFirstMDMA();
    }
    // else if (dmaOutCurrentBufferBSize) {
    //     startSecondMDMA();
    // }

    return 0;
}

uint8_t COMinterChip::startFirstMDMA() {

    FlagHandler::interChipA_MDMA_Started[layer] = 1;
    FlagHandler::interChipA_MDMA_FinishedFunc[layer] = std::bind(&COMinterChip::startFirstDMA, this);

    // set Interchip state and wait for response
    FlagHandler::interChipA_State[layer] = WAITFORRESPONSE;
    FlagHandler::interChipB_State[layer] = WAITFORRESPONSE;
    FlagHandler::interChipA_StateTimeout[layer] = 0;
    FlagHandler::interChipB_StateTimeout[layer] = 0;

    uint8_t ret = HAL_MDMA_Start_IT(&hmdma_mdma_channel40_sw_0, (uint32_t)outBuffer[!currentBufferSelect].data(),
                                    (uint32_t)dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferSize, 1);

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

    outBuffer[0].reserve(INTERCHIPBUFFERSIZE);
    outBuffer[1].reserve(INTERCHIPBUFFERSIZE);
    // outBufferChipB[0].reserve(INTERCHIPBUFFERSIZE);
    // outBufferChipB[1].reserve(INTERCHIPBUFFERSIZE);

    pushDummySizePlaceHolder();
}

uint8_t COMinterChip::startFirstDMA() {
    // println("startFirstDMA");

    // enable NSS to Render Chip A
    if (layer == 0) {

        HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_RESET);
    }
    else {
        HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_RESET);
    }

    FlagHandler::interChipA_DMA_Started[layer] = 1;

    FlagHandler::interChipA_DMA_FinishedFunc[layer] = std::bind(&COMinterChip::sendTransmissionSuccessfull, this);

    uint8_t ret = sendViaDMA(dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferSize);
    if (ret) {
        FlagHandler::interChipA_DMA_Started[layer] = 0;
        FlagHandler::interChipA_DMA_FinishedFunc[layer] = nullptr;
    }
    return ret;
}

// uint8_t COMinterChip::startSecondMDMA() {

//     FlagHandler::interChipB_MDMA_Started[layer] = 1;
//     FlagHandler::interChipB_MDMA_FinishedFunc[layer] = std::bind(&COMinterChip::startSecondDMA, this);

//     FlagHandler::interChipB_State[layer] = WAITFORRESPONSE;
//     FlagHandler::interChipB_StateTimeout[layer] = 0;

//     HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID, comMDMACallback);

//     uint8_t ret = HAL_MDMA_Start_IT(&hmdma_mdma_channel40_sw_0,
//     (uint32_t)outBufferChipB[!currentBufferSelect].data(),
//                                     (uint32_t)dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferBSize,
//                                     1);
//     if (ret) {
//         FlagHandler::interChipB_MDMA_Started[layer] = 0;
//         FlagHandler::interChipB_MDMA_FinishedFunc[layer] = nullptr;
//         HAL_MDMA_UnRegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID);
//     }
//     else {
//         blockNewSendBeginCommand = 1;
//     }
//     return ret;
// }

// uint8_t COMinterChip::startSecondDMA() {
//     // enable NSS to Render Chip B
//     if (layer == 0)
//         HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_RESET);
//     else
//         HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_RESET);

//     FlagHandler::interChipB_DMA_Started[layer] = 1;
//     FlagHandler::interChipB_DMA_FinishedFunc[layer] = std::bind(&COMinterChip::sendTransmissionSuccessfull, this);

//     uint8_t ret = sendViaDMA(dmaOutBufferPointer[!currentBufferSelect], dmaOutCurrentBufferBSize);
//     if (ret) {
//         FlagHandler::interChipB_DMA_Started[layer] = 0;
//         FlagHandler::interChipB_DMA_FinishedFunc[layer] = nullptr;
//     }
//     return ret;
// }

uint8_t COMinterChip::sendTransmissionSuccessfull() {
    if (layer == 0) {
        HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);
    }
    // println("Transmission success!!!");
    blockNewSendBeginCommand = 0;
    return 0;
}

// copy the received SPI via DMA data using the MDMA to the DTCM Ram

uint8_t COMinterChip::pushOutBuffer(uint8_t data) {
    if (outBuffer[currentBufferSelect].size() >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }
    outBuffer[currentBufferSelect].push_back(data);
    return 0;
}

uint8_t COMinterChip::pushOutBuffer(uint8_t *data, uint32_t length) {
    if (outBuffer[currentBufferSelect].size() + length >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    for (uint32_t i = 0; i < length; i++) {
        // println(data[i]);
        outBuffer[currentBufferSelect].push_back(data[i]);
    }
    return 0;
}

// uint8_t COMinterChip::pushOutBufferChipB(uint8_t data) {
//     if (outBufferChipB[currentBufferSelect].size() >= INTERCHIPBUFFERSIZE - 1) {
//         uint8_t ret = invokeBufferFullSend();
//         if (ret) {
//             return ret;
//         }
//     }
//     outBufferChipB[currentBufferSelect].push_back(data);
//     return 0;
// }

// uint8_t COMinterChip::pushOutBufferChipB(uint8_t *data, uint32_t length) {
//     if (outBufferChipB[currentBufferSelect].size() + length >= INTERCHIPBUFFERSIZE - 1) {
//         uint8_t ret = invokeBufferFullSend();
//         if (ret) {
//             return ret;
//         }
//     }

//     for (uint32_t i = 0; i < length; i++) {
//         // println(data[i]);
//         outBufferChipB[currentBufferSelect].push_back(data[i]);
//     }
//     return 0;
// }

uint8_t COMinterChip::invokeBufferFullSend() {
    uint8_t ret = beginSendTransmission();

    if (ret == ERRORCODE_SENDBLOCK || ret == ERRORCODE_RECEPTORNOTREADY) {
        uint32_t timer = millis();
        while (ret == ERRORCODE_SENDBLOCK || ret == ERRORCODE_RECEPTORNOTREADY) { // wait...
            if ((millis() - timer) > 100) { // transmission takes longer than 500ms
                                            //   PolyError_Handler("ERROR | COMMUNICATION | COM -> TIMEOUT > 100ms ");
            }
            FlagHandler::handleFlags(); // Flaghandler muss ausgefuehrt werden damit wir nicht im Loop haengen
                                        // bleiben
            ret = beginSendTransmission();
        }
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

    pushOutBuffer(comCommand, CREATEINOUTPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, CREATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendUpdatePatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[UPDATEINOUTPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | UPDATEINOUTPATCH;
    comCommand[1] = outputId;
    comCommand[2] = inputId;
    *(float *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, UPDATEINOUTPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, UPDATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeletePatchInOut(uint8_t outputId, uint8_t inputId) {
    uint8_t comCommand[DELETEPATCHCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | DELETEINOUTPATCH;
    comCommand[1] = outputId;
    comCommand[2] = inputId;

    pushOutBuffer(comCommand, DELETEPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, DELETEPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeleteAllPatches() {
    uint8_t comCommand[DELETEALLPATCHESCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | DELETEALLPATCHES;

    pushOutBuffer(comCommand[0]);
    // pushOutBufferChipB(comCommand[0]);
    return 0;
}

// this function now dows all settings, int & float settings
uint8_t COMinterChip::sendSetting(uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = SETTINGTYPE | (modulID << 3);
    comCommand[1] = UPDATESETTINGINT | settingID;
    *(int32_t *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    // pushOutBufferChipB(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = SETTINGTYPE | (modulID << 3);
    comCommand[1] = UPDATESETTINGFLOAT | settingID;
    *(float *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendNewNote(uint8_t voiceID, uint8_t note, uint8_t velocity) {
    uint8_t comCommand[NEWNOTECMDSIZE];

    comCommand[1] = note;
    comCommand[2] = velocity;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] = PATCHCMDTYPE | NEWNOTE | voice;
            pushOutBuffer(comCommand, NEWNOTECMDSIZE);
        }
    }
    else {
        comCommand[0] = PATCHCMDTYPE | NEWNOTE | voiceID;
        pushOutBuffer(comCommand, NEWNOTECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendOpenGate(uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] = PATCHCMDTYPE | OPENGATE | voice;
            pushOutBuffer(comCommand[0]);
        }
    }
    else {
        comCommand[0] = PATCHCMDTYPE | OPENGATE | voiceID;
        pushOutBuffer(comCommand[0]);
    }

    return 0;
}

uint8_t COMinterChip::sendCloseGate(uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] = PATCHCMDTYPE | CLOSEGATE | voice;
            pushOutBuffer(comCommand[0]);
        }
    }
    else {
        comCommand[0] = PATCHCMDTYPE | CLOSEGATE | voiceID;
        pushOutBuffer(comCommand[0]);
    }

    return 0;
}

uint8_t COMinterChip::sendRetrigger(uint8_t modulID, uint8_t voiceID) {
    uint8_t comCommand[RETRIGGERCMDSIZE];

    comCommand[1] = RETRIGGER;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] = SETTINGTYPE | (modulID << 3) | voice;

            pushOutBuffer(comCommand, RETRIGGERCMDSIZE);
        }
    }
    else {
        comCommand[0] = SETTINGTYPE | (modulID << 3) | voiceID;

        pushOutBuffer(comCommand, RETRIGGERCMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendResetAll() {
    uint8_t comCommand[RESETALLCMDSIZE];
    comCommand[0] = PATCHCMDTYPE | RESETALL;

    pushOutBuffer(comCommand[0]);

    return 0;
}

// wrap up out buffers
uint8_t COMinterChip::appendLastByte() {
    uint8_t comCommand[LASTBYTECMDSIZE];
    comCommand[0] = PATCHCMDTYPE | LASTBYTE;
    outBuffer[currentBufferSelect].push_back(comCommand[0]);

    return 0;
}

// reserve place for size byte
void COMinterChip::pushDummySizePlaceHolder() {
    uint8_t dummysize = 0;
    pushOutBuffer((uint8_t *)&dummysize, 1);
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

    // println("start reception");

    uint8_t ret = receiveViaDMA(dmaInBufferPointer[!currentBufferSelect], INTERCHIPBUFFERSIZE);
    FlagHandler::interChipReceive_DMA_Started = 1;
    FlagHandler::interChipReceive_DMA_FinishedFunc = std::bind(&COMinterChip::copyReceivedInBuffer, this);
    return ret;
}

uint8_t COMinterChip::copyReceivedInBuffer() {

    // as receive DMA is probably still running, disable it here
    stopReceiveViaDMA();

    // flush fifo buffer necessary?
    // HAL_SPIEx_FlushRxFifo(&hspi1);

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

uint8_t COMinterChip::checkVoice(uint8_t voice) {
    if (layerA.chipID == 0) {
        if (voice < VOICESPERCHIP)
            return voice;
        else
            return NOVOICE;
    }
    else {
        if (voice < VOICESPERCHIP)
            return NOVOICE;
        else
            return voice - VOICESPERCHIP;
    }
}

uint8_t COMinterChip::decodeCurrentInBuffer() {

    switchBuffer();

    // show that we are decoding the buffer

    // println("decode Buffer");
    // necessary decoding vars
    volatile uint8_t outputID = 0;
    volatile uint8_t inputID = 0;
    volatile uint8_t currentByte = 0;
    volatile uint8_t setting = 0;
    volatile uint8_t modul = 0;
    volatile uint8_t voice = 0;
    volatile uint8_t note = 0;
    volatile uint8_t velocity = 0;
    volatile int32_t amountInt = 0;
    volatile float amountFloat = 0;
    // volatile float offsetFloat = 0;

    beginReceiveTransmission();

    // assemble size
    uint8_t sizeOfReadBuffer = *inBufferPointer[currentBufferSelect];
    // println("Buffer size: ", sizeOfReadBuffer);

    if (sizeOfReadBuffer > INTERCHIPBUFFERSIZE) {
        // buffer too big, sth went wrong
        PolyError_Handler("ERROR | FATAL | com buffer too big");
        return 1;
    }

    if ((inBufferPointer[currentBufferSelect])[sizeOfReadBuffer - 1] != (LASTBYTE | PATCHCMDTYPE)) {
        PolyError_Handler("ERROR | FATAL | com buffer last byte wrong");
        return 1;
    }

    HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);

    // start with offset, as two bytes were size
    for (uint8_t i = 1; i < sizeOfReadBuffer; i++) {
        currentByte = (inBufferPointer[currentBufferSelect])[i];

        switch (currentByte & CMD_TYPEMASK) {

            // first byte is is Patch/Command
            case PATCHCMDTYPE:

                switch (currentByte & CMD_PATCHMASK) {

                    case UPDATEINOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        layerA.updatePatchInOutByIdWithoutMapping(outputID, inputID, amountFloat);

                        break;

                    case CREATEINOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];

                        i += sizeof(float) - 1;

                        layerA.addPatchInOutById(outputID, inputID, amountFloat);

                        break;

                    case DELETEINOUTPATCH:
                        outputID = (inBufferPointer[currentBufferSelect])[++i];
                        inputID = (inBufferPointer[currentBufferSelect])[++i];

                        layerA.removePatchInOutById(outputID, inputID);

                        break;

                    case DELETEALLPATCHES:
                        // delete all patchesInOut at once
                        layerA.clearPatches();
                        break;

                    case NEWNOTE:
                        voice = currentByte & CMD_VOICEMASK;
                        note = (inBufferPointer[currentBufferSelect])[++i];
                        velocity = (inBufferPointer[currentBufferSelect])[++i];

                        voice = checkVoice(voice);
                        if (voice != NOVOICE) {
                            layerA.setNote(voice, note, velocity);
                            layerA.gateOn(voice);
                        }

                        break;

                    case OPENGATE:
                        voice = currentByte & CMD_VOICEMASK;
                        voice = checkVoice(voice);
                        if (voice != NOVOICE)
                            layerA.gateOn(voice);
                        break;

                    case CLOSEGATE:
                        voice = currentByte & CMD_VOICEMASK;
                        voice = checkVoice(voice);
                        if (voice != NOVOICE)
                            layerA.gateOff(voice);
                        break;

                    case CLOCK:
                        // clock()
                        // probably obsolete
                        break;

                    case RESETALL:
                        // clear everything
                        layerA.resetLayer();
                        break;

                    case LASTBYTE:
                        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);

                        // transmission complete
                        // println("decode success!");
                        return 0;

                    default:
                        // seomething went wrong here
                        PolyError_Handler("ERROR | FATAL | COM wrong PATCHCMDTYPE");

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
                switch (currentByte & CMD_SETTINGSCMDMASK) {
                    case UPDATESETTINGINT:
                        setting = currentByte & CMD_SETTINGSMASK;
                        amountInt = *(int32_t *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(int32_t) - 1;

                        layerA.getModules()[modul]->getSwitches()[setting]->setValueWithoutMapping(amountInt);

                        // println("setting Int: ", amountInt);

                        break;
                    case UPDATESETTINGFLOAT:
                        setting = currentByte & CMD_SETTINGSMASK;
                        amountFloat = *(float *)&(inBufferPointer[currentBufferSelect])[++i];
                        i += sizeof(float) - 1;

                        layerA.getModules()[modul]->getPotis()[setting]->setValueWithoutMapping(amountFloat);

                        // println("setting float: ", amountFloat);

                        break;
                    case RETRIGGER:
                        voice = checkVoice(voice);
                        if (voice != NOVOICE)
                            layerA.getModules()[modul]->resetPhase(voice);
                        break;

                    default:
                        // something went wrong here
                        std::string error = "ERROR | FATAL | COM wrong SETTINGTYPE: " + std::to_string(currentByte);
                        PolyError_Handler(error.data());

                        return 1;
                }
                break;
            default:
                // seomething went wrong here
                PolyError_Handler("ERROR | FATAL | Decode error");

                return 1;
        }
    }

    // we should always exit with LASTBYTE
    PolyError_Handler("no LASTBYTE received");

    return 1;
}

#endif

void COMinterChip::prepareMDMAHandle() {
    hmdma_mdma_channel40_sw_0.Instance = MDMA_Channel0;
    hmdma_mdma_channel40_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel40_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel40_sw_0.Init.Priority = MDMA_PRIORITY_HIGH;
    hmdma_mdma_channel40_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
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
    outBuffer[currentBufferSelect].clear();
    // outBufferChipB[currentBufferSelect].clear();
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
        // if (FlagHandler::interChipB_MDMA_Started[i] == 1) {
        //     FlagHandler::interChipB_MDMA_Started[i] = 0;
        //     FlagHandler::interChipB_MDMA_Finished[i] = 1;
        // }
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
