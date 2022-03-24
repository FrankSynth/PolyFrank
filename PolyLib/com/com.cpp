#include "com.hpp"
#include "layer/layer.hpp"

extern Layer layerA;

#ifdef POLYCONTROL
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

#endif

/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// COMinterChip //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// extern void comMDMACallback(MDMA_HandleTypeDef *_hmdma);
// extern void comMDMACallbackError(MDMA_HandleTypeDef *_hmdma);

#ifdef POLYCONTROL

// Patch functions
uint8_t COMinterChip::sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[CREATEINOUTPATCHCMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = CREATEINOUTPATCH;
    comCommand[2] = outputId;
    comCommand[3] = inputId;
    *(float *)(&comCommand[4]) = amount;

    pushOutBuffer(comCommand, CREATEINOUTPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, CREATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[UPDATEINOUTPATCHCMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = UPDATEINOUTPATCH;
    comCommand[2] = outputId;
    comCommand[3] = inputId;
    *(float *)(&comCommand[4]) = amount;

    pushOutBuffer(comCommand, UPDATEINOUTPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, UPDATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId) {
    uint8_t comCommand[DELETEPATCHCMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = DELETEINOUTPATCH;
    comCommand[2] = outputId;
    comCommand[3] = inputId;

    pushOutBuffer(comCommand, DELETEPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, DELETEPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeleteAllPatches(uint8_t layerId) {
    uint8_t comCommand[DELETEALLPATCHESCMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = DELETEALLPATCHES;

    pushOutBuffer(comCommand, DELETEALLPATCHESCMDSIZE);
    // pushOutBufferChipB(comCommand[0]);
    return 0;
}

uint8_t COMinterChip::sendNewNote(uint8_t layerId, uint8_t voiceID, uint8_t note, uint8_t velocity) {
    uint8_t comCommand[NEWNOTECMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = NEWNOTE;
    comCommand[2] = note;
    comCommand[3] = velocity;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] |= voice;
            pushOutBuffer(comCommand, NEWNOTECMDSIZE);
        }
    }
    else {
        comCommand[0] |= voiceID;
        pushOutBuffer(comCommand, NEWNOTECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendOpenGate(uint8_t layerId, uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = OPENGATE;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] |= voice;
            pushOutBuffer(comCommand, GATECMDSIZE);
        }
    }
    else {
        comCommand[0] |= voiceID;
        pushOutBuffer(comCommand, GATECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendCloseGate(uint8_t layerId, uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = CLOSEGATE;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] |= voice;
            pushOutBuffer(comCommand, GATECMDSIZE);
        }
    }
    else {
        comCommand[0] |= voiceID;
        pushOutBuffer(comCommand, GATECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendRetrigger(uint8_t layerId, uint8_t modulID, uint8_t voiceID) {
    uint8_t comCommand[RETRIGGERCMDSIZE];

    comCommand[0] = (layerId << 7) | (modulID << 3);
    comCommand[1] = RETRIGGER;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[0] |= voice;

            pushOutBuffer(comCommand, RETRIGGERCMDSIZE);
        }
    }
    else {
        comCommand[0] |= voiceID;

        pushOutBuffer(comCommand, RETRIGGERCMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendResetAll(uint8_t layerId) {
    uint8_t comCommand[RESETALLCMDSIZE];

    comCommand[0] = layerId << 7;
    comCommand[1] = RESETALL;

    pushOutBuffer(comCommand, RESETALLCMDSIZE);

    return 0;
}

uint8_t COMinterChip::sendRequestUIData() {
    uint8_t comCommand[SENDUPDATETOCONTROLSIZE];

    comCommand[0] = SENDUPDATETOCONTROL;

    // TODO set flags for data reception, separaate flags pls

    pushOutBuffer(comCommand, SENDUPDATETOCONTROLSIZE);

    return 0;
}

#endif

#ifdef POLYRENDER

// this function now dows all settings, int & float settings
uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = (layerId << 7) | (modulID << 3);
    comCommand[1] = UPDATESETTINGINT;
    comCommand[2] = settingID;
    *(int32_t *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    // pushOutBufferChipB(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = (layerId << 7) | (modulID << 3);
    comCommand[1] = UPDATESETTINGFLOAT;
    comCommand[2] = settingID;
    *(float *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendString(std::string &message) {

    uint8_t messagesize = message.size();

    if (messagesize > INTERCHIPBUFFERSIZE - 2 - LASTBYTECMDSIZE - MESSAGECMDSIZE)
        return 1; // not possible to send this

    messagebuffer.clear();

    messagebuffer = (char)SENDMESSAGE;
    messagebuffer += (char)messagesize;
    messagebuffer.append(message);

    pushOutBuffer((uint8_t *)messagebuffer.data(), messagebuffer.size());

    return 0;
}

// check if voices beling to current render chip, and shift if necessary
uint8_t COMinterChip::checkVoiceAgainstChipID(uint8_t voice) {
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

#endif

/////////////////////////// MASTER & SLAVE ////////////////////////

uint8_t COMinterChip::beginReceiveTransmission() {
    return receiveViaDMA(dmaInBufferPointer[!currentInBufferSelect], INTERCHIPBUFFERSIZE);
}

uint8_t COMinterChip::beginSendTransmission() {

    if (sendViaDMA == nullptr) {
        return 0;
    }
    // buffers empty
    if (outBuffer[currentOutBufferSelect].size() < 2) {
        // println("buffer empty, skip send");
        return 0;
    }

    // block new transmissions as long as a transmission is already running
    if (blockNewSendBeginCommand) {
        return ERRORCODE_SENDBLOCK;
    }

#ifdef POLYCONTROL
    if (FlagHandler::layerActive[0]) {
        if (FlagHandler::renderChip_State[0][0] != READY && FlagHandler::renderChip_State[0][1]) {
            return ERRORCODE_RECEPTORNOTREADY;
        }
    }

    if (FlagHandler::layerActive[1]) {
        if (FlagHandler::renderChip_State[1][0] != READY && FlagHandler::renderChip_State[1][1] != READY) {
            return ERRORCODE_RECEPTORNOTREADY;
        }
    }
#endif

    appendLastByte();
    dmaOutCurrentBufferSize = outBuffer[currentOutBufferSelect].size();

    // write size into first two bytes of outBuffers

    // HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID, comMDMACallback);
    // HAL_MDMA_RegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_ERROR_CB_ID, comMDMACallbackError);
    // prepareSendMDMAHandle();

    *outBuffer[currentOutBufferSelect].data() = dmaOutCurrentBufferSize;
    switchOutBuffer();

    startSendDMA();

    return 0;
}

void COMinterChip::initInTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaReceiveFunc,
                                      std::function<uint8_t()> dmaStopReceiveFunc, uint8_t *dmaBuffer) {
    receiveViaDMA = dmaReceiveFunc;
    stopReceiveViaDMA = dmaStopReceiveFunc;
    dmaInBufferPointer[0] = dmaBuffer;
    dmaInBufferPointer[1] = dmaBuffer + INTERCHIPBUFFERSIZE;
}

void COMinterChip::initOutTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaTransmitFunc,
                                       uint8_t *dmaBuffer) {

    sendViaDMA = dmaTransmitFunc;

    dmaOutBufferPointer[0] = dmaBuffer;
    dmaOutBufferPointer[1] = dmaBuffer + INTERCHIPBUFFERSIZE;

    pushDummySizePlaceHolder();
    // FlagHandler::interChipSend_MDMA_FinishedFunc = std::bind(&COMinterChip::startSendDMA, this);
    FlagHandler::interChipSend_DMA_FinishedFunc = std::bind(&COMinterChip::sendTransmissionSuccessfull, this);
}

uint8_t COMinterChip::startSendDMA() {

    // set Interchip state and wait for response

#ifdef POLYCONTROL

    // TODO currently wrong? more defined state pls, response vs datareception (last should be separate i think)
    // !! see comInterchipStates !!
    for (uint8_t i = 0; i < 2; i++) {
        for (uint8_t j = 0; j < 2; j++) {
            FlagHandler::renderChip_State[i][j] = WAITFORRESPONSE;
            FlagHandler::renderChip_StateTimeout[i][j] = 0;
        }
    }
#endif

    fast_copy_f32((uint32_t *)outBuffer[!currentOutBufferSelect].data(),
                  (uint32_t *)dmaOutBufferPointer[!currentOutBufferSelect], (dmaOutCurrentBufferSize + 3) >> 2);

    blockNewSendBeginCommand = 1;

#ifdef POLYCONTROL
    // enable NSS to Render Chip A

    // TODO check CS pins
    HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_RESET);

#endif

    FlagHandler::interChipSend_DMA_Started = 1;

    uint8_t ret = sendViaDMA(dmaOutBufferPointer[!currentOutBufferSelect], dmaOutCurrentBufferSize);
    if (ret) {
        FlagHandler::interChipSend_DMA_Started = 0;
    }
    return ret;
}

/**
 * @brief out DMA was successfull, executed from DMA callback
 *
 * @return uint8_t status
 */
uint8_t COMinterChip::sendTransmissionSuccessfull() {

#ifdef POLYCONTROL

    // TODO check CS  selection
    HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);

#endif

    blockNewSendBeginCommand = 0;
    return 0;
}

uint8_t COMinterChip::decodeCurrentInBuffer() {

    stopReceiveViaDMA();

    switchInBuffer();

    // necessary decoding vars
#ifdef POLYRENDER
    volatile uint8_t outputID = 0;
    volatile uint8_t inputID = 0;
    volatile uint8_t note = 0;
    volatile uint8_t velocity = 0;
#endif
    volatile uint8_t layerID = 0;
    volatile uint8_t setting = 0;
    volatile uint8_t module = 0;
    volatile uint8_t voice = 0;
    volatile int32_t amountInt = 0;
    volatile float amountFloat = 0;

    // assemble size
    uint8_t sizeOfReadBuffer = *dmaInBufferPointer[currentInBufferSelect];

    if (sizeOfReadBuffer > INTERCHIPBUFFERSIZE) {
        // buffer too big, sth went wrong
        PolyError_Handler("ERROR | FATAL | com buffer too big");
        return 1;
    }

    fast_copy_f32((uint32_t *)(dmaInBufferPointer[currentOutBufferSelect]++),
                  (uint32_t *)(inBufferPointer[currentOutBufferSelect]++), (sizeOfReadBuffer - 1 + 3) >> 2);

    if ((inBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1] != LASTBYTE) {
        // last byte must be at this position, sort of CRC
        PolyError_Handler("ERROR | FATAL | com buffer last byte wrong");
        return 1;
    }

    // start with offset, as two bytes were size
    for (uint8_t i = 1; i < sizeOfReadBuffer; i++) {
        uint8_t currentByte = (inBufferPointer[currentInBufferSelect])[i];

        if (currentByte == LASTBYTE) {
            beginReceiveTransmission();
            HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);
            return 0;
        }

#ifdef POLYRENDER
        if (currentByte == SENDUPDATETOCONTROL) {
            // TODO send UI update shit

            continue;
        }
#endif

#ifdef POLYCONTROL

        // TODO implement with layer and chip encoding
        if (currentByte == SENDMESSAGE) {

            messagebuffer.clear();

            uint8_t messagesize = (inBufferPointer[currentInBufferSelect])[++i];

            for (uint8_t i = 0; i < messagesize; i++) {
                messagebuffer += (char)(inBufferPointer[currentInBufferSelect])[++i];
            }

            // TODO output to debug console
            println(messagebuffer);

            continue;
        }

#endif

        layerID = (currentByte & CMD_LAYERMASK) >> 7;
        module = (currentByte & CMD_MODULEMASK) >> 3;
        voice = currentByte & CMD_VOICEMASK;

        // convert voice 4-7 to 0-3 if on chip B
        voice = checkVoiceAgainstChipID(voice);

        currentByte = (inBufferPointer[currentInBufferSelect])[++i];
        switch (currentByte) {

#ifdef POLYRENDER
            case UPDATEINOUTPATCH:
                outputID = (inBufferPointer[currentInBufferSelect])[++i];
                inputID = (inBufferPointer[currentInBufferSelect])[++i];
                amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.updatePatchInOutByIdWithoutMapping(outputID, inputID, amountFloat);

                break;

            case CREATEINOUTPATCH:
                outputID = (inBufferPointer[currentInBufferSelect])[++i];
                inputID = (inBufferPointer[currentInBufferSelect])[++i];
                amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.addPatchInOutById(outputID, inputID, amountFloat);

                break;

            case DELETEINOUTPATCH:
                outputID = (inBufferPointer[currentInBufferSelect])[++i];
                inputID = (inBufferPointer[currentInBufferSelect])[++i];

                if (layerID == layerA.id)
                    layerA.removePatchInOutById(outputID, inputID);

                break;

            case DELETEALLPATCHES:
                // delete all patchesInOut at once
                if (layerID == layerA.id)
                    layerA.clearPatches();
                break;

            case NEWNOTE:
                note = (inBufferPointer[currentInBufferSelect])[++i];
                velocity = (inBufferPointer[currentInBufferSelect])[++i];

                if (voice != NOVOICE && layerID == layerA.id) {
                    // println("new note played");

                    layerA.setNote(voice, note, velocity);
                    layerA.gateOn(voice);
                }

                break;

            case OPENGATE:
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOn(voice);
                break;

            case CLOSEGATE:
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOff(voice);
                break;

            case RESETALL:
                // clear everything
                if (layerID == layerA.id)
                    layerA.resetLayer();
                break;

            case RETRIGGER:
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.getModules()[module]->resetPhase(voice);
                break;

            case UPDATESETTINGINT:
                setting = (inBufferPointer[currentInBufferSelect])[++i];
                amountInt = *(int32_t *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(int32_t) - 1

                if (layerID == layerA.id) {
                    layerA.getModules()[module]->getSwitches()[setting]->setValueWithoutMapping(amountInt);
                }

                break;

            case UPDATESETTINGFLOAT:
                setting = (inBufferPointer[currentInBufferSelect])[++i];

                amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id) {

                    layerA.getModules()[module]->getPotis()[setting]->setValueWithoutMapping(amountFloat);
                }

                break;
#endif

#ifdef POLYCONTROL

                // TODO add decoding here

#endif

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

// void COMinterChip::prepareSendMDMAHandle() {
//     hmdma_mdma_channel40_sw_0.Instance = MDMA_Channel0;
//     hmdma_mdma_channel40_sw_0.Init.Request = MDMA_REQUEST_SW;
//     hmdma_mdma_channel40_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
//     hmdma_mdma_channel40_sw_0.Init.Priority = MDMA_PRIORITY_HIGH;
//     hmdma_mdma_channel40_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
//     hmdma_mdma_channel40_sw_0.Init.SourceInc = MDMA_SRC_INC_WORD;
//     hmdma_mdma_channel40_sw_0.Init.DestinationInc = MDMA_DEST_INC_WORD;
//     hmdma_mdma_channel40_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
//     hmdma_mdma_channel40_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
//     hmdma_mdma_channel40_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
//     hmdma_mdma_channel40_sw_0.Init.BufferTransferLength = 64;
//     hmdma_mdma_channel40_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
//     hmdma_mdma_channel40_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
//     hmdma_mdma_channel40_sw_0.Init.SourceBlockAddressOffset = 0;
//     hmdma_mdma_channel40_sw_0.Init.DestBlockAddressOffset = 0;

//     HAL_MDMA_Init(&hmdma_mdma_channel40_sw_0);
// }

void COMinterChip::switchInBuffer() {
    currentInBufferSelect = !currentInBufferSelect;
}

void COMinterChip::switchOutBuffer() {
    currentOutBufferSelect = !currentOutBufferSelect;

    outBuffer[currentOutBufferSelect].clear();
    pushDummySizePlaceHolder();
}

// wrap up out buffers
uint8_t COMinterChip::appendLastByte() {
    uint8_t comCommand[LASTBYTECMDSIZE];
    comCommand[0] = LASTBYTE;
    outBuffer[currentOutBufferSelect].push_back(comCommand[0]);

    return 0;
}

// reserve place for size byte
void COMinterChip::pushDummySizePlaceHolder() {
    uint8_t dummysize = 0;
    pushOutBuffer((uint8_t *)&dummysize, 1);
}

// uint8_t COMinterChip::pushOutBuffer(uint8_t data) {
//     if (outBuffer[currentOutBufferSelect].size() >= INTERCHIPBUFFERSIZE - 1) {
//         uint8_t ret = invokeBufferFullSend();
//         if (ret) {
//             return ret;
//         }
//     }
//     outBuffer[currentOutBufferSelect].push_back(data);
//     return 0;
// }

uint8_t COMinterChip::pushOutBuffer(uint8_t *data, uint32_t length = 1) {
    if (outBuffer[currentOutBufferSelect].size() + length >= INTERCHIPBUFFERSIZE - 1) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    for (uint32_t i = 0; i < length; i++) {
        // println(data[i]);
        outBuffer[currentOutBufferSelect].push_back(data[i]);
    }
    return 0;
}

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