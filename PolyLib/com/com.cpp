#include "com.hpp"
#include "layer/layer.hpp"
#include <cstring>

extern Layer layerA;

#ifdef POLYCONTROL

extern void setCSLine(uint8_t layer, uint8_t chip, GPIO_PinState state);
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

#ifdef POLYCONTROL

// Patch functions
uint8_t COMinterChip::sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[CREATEINOUTPATCHCMDSIZE];

    comCommand[0] = CREATEINOUTPATCH;
    comCommand[1] = layerId << 7;
    comCommand[2] = outputId;
    comCommand[3] = inputId;
    *(float *)(&comCommand[4]) = amount;

    pushOutBuffer(comCommand, CREATEINOUTPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, CREATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    uint8_t comCommand[UPDATEINOUTPATCHCMDSIZE];

    comCommand[0] = UPDATEINOUTPATCH;
    comCommand[1] = layerId << 7;
    comCommand[2] = outputId;
    comCommand[3] = inputId;
    *(float *)(&comCommand[4]) = amount;

    pushOutBuffer(comCommand, UPDATEINOUTPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, UPDATEINOUTPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId) {
    uint8_t comCommand[DELETEPATCHCMDSIZE];

    comCommand[0] = DELETEINOUTPATCH;
    comCommand[1] = layerId << 7;
    comCommand[2] = outputId;
    comCommand[3] = inputId;

    pushOutBuffer(comCommand, DELETEPATCHCMDSIZE);
    // pushOutBufferChipB(comCommand, DELETEPATCHCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendDeleteAllPatches(uint8_t layerId) {
    uint8_t comCommand[DELETEALLPATCHESCMDSIZE];

    comCommand[0] = DELETEALLPATCHES;
    comCommand[1] = layerId << 7;

    pushOutBuffer(comCommand, DELETEALLPATCHESCMDSIZE);
    // pushOutBufferChipB(comCommand[0]);
    return 0;
}

uint8_t COMinterChip::sendNewNote(uint8_t layerId, uint8_t voiceID, uint8_t note, uint8_t velocity) {
    uint8_t comCommand[NEWNOTECMDSIZE];

    comCommand[0] = NEWNOTE;
    comCommand[1] = layerId << 7;
    comCommand[2] = note;
    comCommand[3] = velocity;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[1] |= voice;
            pushOutBuffer(comCommand, NEWNOTECMDSIZE);
        }
    }
    else {
        comCommand[1] |= voiceID;
        pushOutBuffer(comCommand, NEWNOTECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendOpenGate(uint8_t layerId, uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];

    comCommand[0] = OPENGATE;
    comCommand[1] = layerId << 7;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[1] |= voice;
            pushOutBuffer(comCommand, GATECMDSIZE);
        }
    }
    else {
        comCommand[1] |= voiceID;
        pushOutBuffer(comCommand, GATECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendCloseGate(uint8_t layerId, uint8_t voiceID) {
    uint8_t comCommand[GATECMDSIZE];

    comCommand[0] = CLOSEGATE;
    comCommand[1] = layerId << 7;

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[1] |= voice;
            pushOutBuffer(comCommand, GATECMDSIZE);
        }
    }
    else {
        comCommand[1] |= voiceID;
        pushOutBuffer(comCommand, GATECMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendRetrigger(uint8_t layerId, uint8_t modulID, uint8_t voiceID) {
    uint8_t comCommand[RETRIGGERCMDSIZE];

    comCommand[0] = RETRIGGER;
    comCommand[1] = (layerId << 7) | (modulID << 3);

    if (voiceID == VOICEALL) {
        for (uint16_t voice = 0; voice < VOICEALL; voice++) {
            comCommand[1] |= voice;

            pushOutBuffer(comCommand, RETRIGGERCMDSIZE);
        }
    }
    else {
        comCommand[1] |= voiceID;

        pushOutBuffer(comCommand, RETRIGGERCMDSIZE);
    }

    return 0;
}

uint8_t COMinterChip::sendResetAll(uint8_t layerId) {
    uint8_t comCommand[RESETALLCMDSIZE];

    comCommand[0] = RESETALL;
    comCommand[1] = layerId << 7;

    pushOutBuffer(comCommand, RESETALLCMDSIZE);

    return 0;
}

uint8_t COMinterChip::sendRequestUIData() {

    // invokeBufferFullSend();

    while (spi->state != BUS_READY)
        ;

    if (sentRequestUICommand == false) {
        uint8_t comCommand;

        comCommand = SENDUPDATETOCONTROL;

        // TODO set flags for data reception, separaate flags pls

        pushOutBuffer(comCommand);

        // invokeBufferFullSend();
        sentRequestUICommand = true;
    }

    return 0;
}

// this function now dows all settings, int & float settings
uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = UPDATESETTINGINT;
    comCommand[1] = (layerId << 7) | (modulID << 3);
    comCommand[2] = settingID;
    *(int32_t *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    // pushOutBufferChipB(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = UPDATESETTINGFLOAT;
    comCommand[1] = (layerId << 7) | (modulID << 3);
    comCommand[2] = settingID;
    *(float *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    return 0;
}

busState COMinterChip::beginReceiveTransmission(uint8_t layer, uint8_t chip) {
    if (spi->state != BUS_READY)
        return spi->state;
    receiveLayer = layer;
    receiveChip = chip;

    setCSLine(receiveLayer, receiveChip, GPIO_PIN_RESET);

    uint16_t receiveSize;

    requestSize = true;
    spi->receive(dmaInBufferPointer[!currentInBufferSelect], 2, true);
    while (spi->state != BUS_READY) {
    }

    receiveSize = *(uint16_t *)dmaInBufferPointer[!currentInBufferSelect];

    if (receiveSize > INTERCHIPBUFFERSIZE) {
        PolyError_Handler("ERROR | FATAL | com buffer too big");
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = INTERCHIPERROR;
        return BUS_ERROR;
    }

    if (receiveSize < 2) {
        println("retreive buffer was 0, chip", receiveChip);
        while (true)
            ;
        return BUS_OK;
    }

    spi->receive(&dmaInBufferPointer[!currentInBufferSelect][2], receiveSize - 2, true);
    return BUS_OK;
}

#endif

#ifdef POLYRENDER

uint8_t COMinterChip::sendString(std::string &message) {

    uint16_t messagesize = message.size();

    if (messagesize > 255)
        return 1; // not possible to send this

    messagebuffer = (char)SENDMESSAGE;
    messagebuffer += (char)messagesize;
    messagebuffer.append(message);

    pushOutBuffer((uint8_t *)messagebuffer.data(), messagebuffer.size());
    return 0;
}
uint8_t COMinterChip::sendString(const char *message) {

    uint16_t messagesize = std::strlen(message);

    if (messagesize > 255)
        return 1; // not possible to send this

    messagebuffer = (char)SENDMESSAGE;
    messagebuffer += (char)messagesize;
    messagebuffer.append(message);

    pushOutBuffer((uint8_t *)messagebuffer.data(), messagebuffer.size());

    return 0;
}

uint8_t COMinterChip::sendOutput(uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[OUTPUTCMDSIZE];
    comCommand[0] = UPDATEOUTPUTINT;
    comCommand[1] = (modulID << 4) | settingID;
    *(int32_t *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, OUTPUTCMDSIZE);
    return 0;
}
uint8_t COMinterChip::sendOutput(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[OUTPUTCMDSIZE];
    comCommand[0] = UPDATEOUTPUTFLOAT;
    comCommand[1] = (modulID << 4) | settingID;
    *(float *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, OUTPUTCMDSIZE);
    return 0;
}
uint8_t COMinterChip::sendInput(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[INPUTCMDSIZE];
    comCommand[0] = UPDATEINPUT;
    comCommand[1] = (modulID << 4) | settingID;
    *(float *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, INPUTCMDSIZE);
    return 0;
}

busState COMinterChip::beginReceiveTransmission() {
    busState ret = spi->receive(dmaInBufferPointer[!currentInBufferSelect], INTERCHIPBUFFERSIZE, true);
    if (ret == BUS_OK)
        HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);
    return ret;
}

#endif

/////////////////////////// MASTER & SLAVE ////////////////////////

busState COMinterChip::beginSendTransmission() {

#ifdef POLYCONTROL
    // don't send buffer if there is nothing except the size
    if (outBuffer[currentOutBufferSelect].size() < 3) {
        return BUS_OK;
    }
#endif

    // block new transmissions as long as a transmission is already running
    if (spi->state != BUS_READY) {
        return spi->state;
    }

#ifdef POLYCONTROL
    if (FlagHandler::layerActive[0]) {
        if ((FlagHandler::renderChip_State[0][0] != READY) || (FlagHandler::renderChip_State[0][1] != READY)) {
            return BUS_BUSY;
        }
    }

    if (FlagHandler::layerActive[1]) {
        if ((FlagHandler::renderChip_State[1][0] != READY) || (FlagHandler::renderChip_State[1][1] != READY)) {
            return BUS_BUSY;
        }
    }

    if (!(HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_1_Pin) &&
          HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_2_Pin))) {
        println("blocked send");
        return BUS_BUSY;
    }

    if (sentRequestUICommand) {
        for (uint8_t i = 0; i < 2; i++) {
            for (uint8_t j = 0; j < 2; j++) {
                FlagHandler::renderChipAwaitingData[i][j] = FlagHandler::layerActive[i];
            }
        }
    }

#endif

    switchOutBuffer();
    appendLastByte();

    // write size into first two bytes of outBuffers
    dmaOutCurrentBufferSize = outBuffer[!currentOutBufferSelect].size();
    *(uint16_t *)outBuffer[!currentOutBufferSelect].data() = dmaOutCurrentBufferSize;

    busState ret = startSendDMA();

#ifdef POLYRENDER
    if (ret == BUS_OK) {
        HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);
    }
#endif

    return ret;
}

busState COMinterChip::startSendDMA() {

    // set Interchip state and wait for response

#ifdef POLYCONTROL

    // TODO currently wrong? more defined state pls, response vs datareception (last should be separate i think)
    // !! see comInterchipStates !!
    if (FlagHandler::layerActive[0]) {
        for (uint8_t j = 0; j < 2; j++) {
            FlagHandler::renderChip_State[0][j] = WAITFORRESPONSE;
            FlagHandler::renderChip_StateTimeout[0][j] = 0;
        }
    }
    if (FlagHandler::layerActive[1]) {
        for (uint8_t j = 0; j < 2; j++) {
            FlagHandler::renderChip_State[1][j] = WAITFORRESPONSE;
            FlagHandler::renderChip_StateTimeout[1][j] = 0;
        }
    }
#endif

    // fast_copy_f32((uint32_t *)outBuffer[!currentOutBufferSelect].data(),
    //               (uint32_t *)dmaOutBufferPointer[!currentOutBufferSelect], (dmaOutCurrentBufferSize + 4) >> 2);

    fast_copy_byte(outBuffer[!currentOutBufferSelect].data(), dmaOutBufferPointer[!currentOutBufferSelect],
                   dmaOutCurrentBufferSize);

#ifdef POLYCONTROL
    // enable NSS to Render Chip A

    // TODO check CS pins
    if (FlagHandler::layerActive[0]) {
        HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_RESET);
    }
    if (FlagHandler::layerActive[1]) {
        HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_RESET);
    }

#endif
#ifdef POLYRENDER

    // hspi1.Instance = SPI1;
    // hspi1.Init.Mode = SPI_MODE_SLAVE;
    // hspi1.Init.Direction = SPI_DIRECTION_1LINE;
    // hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    // hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    // hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    // hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
    // hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    // hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    // hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    // hspi1.Init.CRCPolynomial = 0x0;
    // hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    // hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    // hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    // hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    // hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    // hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    // hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    // hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    // hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    // hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    // if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    //     Error_Handler();
    // }
#endif

    return spi->transmit(dmaOutBufferPointer[!currentOutBufferSelect], dmaOutCurrentBufferSize, true);
}

#ifdef POLYRENDER

// check if voices beling to current render chip, and shift if necessary
ALWAYS_INLINE inline void checkVoiceAgainstChipID(uint8_t *voice) {
    if (layerA.chipID == 0) {
        if (*voice >= VOICESPERCHIP)
            *voice = NOVOICE;
    }
    else {
        if (*voice < VOICESPERCHIP)
            *voice = NOVOICE;
        else
            *voice = *voice - VOICESPERCHIP;
    }
}

ALWAYS_INLINE inline void readLayerModuleVoice(uint8_t *layerID, uint8_t *module, uint8_t *voice,
                                               uint8_t *currentByte) {
    *layerID = (*currentByte & CMD_LAYERMASK) >> 7;
    *module = (*currentByte & CMD_MODULEMASK) >> 3;
    *voice = *currentByte & CMD_VOICEMASK;
    checkVoiceAgainstChipID(voice);
}

#endif

uint8_t COMinterChip::decodeCurrentInBuffer() {

    state = COM_DECODE;

    // necessary decoding vars
    volatile uint8_t outputID = 0;
    volatile uint8_t inputID = 0;
    volatile uint8_t note = 0;
    volatile uint8_t velocity = 0;
    volatile uint8_t layerID = 0;
    volatile uint8_t setting = 0;
    volatile uint8_t module = 0;
    volatile uint8_t voice = 0;
    volatile uint8_t messagesize = 0;
    volatile int32_t amountInt = 0;
    volatile float amountFloat = 0;

    switchInBuffer();

    // assemble size
    uint16_t sizeOfReadBuffer = *((uint16_t *)(dmaInBufferPointer[currentInBufferSelect]));

    // no data received
    if (sizeOfReadBuffer == 0) {

        state = COM_READY;
#ifdef POLYCONTROL
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = INTERCHIPERROR;
#endif
#ifdef POLYRENDER
        PolyError_Handler("ERROR | FATAL | size=0");

        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | size=0, receive bus occuppied");
        }
#endif
        return 0;
    }

    if (sizeOfReadBuffer > INTERCHIPBUFFERSIZE) {
        state = COM_READY;
        println("com size: ", sizeOfReadBuffer);
        PolyError_Handler("ERROR | FATAL | decode com buffer too big");
#ifdef POLYCONTROL
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = INTERCHIPERROR;
#endif
#ifdef POLYRENDER
        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | too big, receive bus occuppied");
        }
#endif
        return 1;
    }

    // copy dma buffer to local space with word speed
    // fast_copy_f32((uint32_t *)(dmaInBufferPointer[currentInBufferSelect]),
    //               (uint32_t *)(inBufferPointer[currentInBufferSelect]), (sizeOfReadBuffer + 4) >> 2);
    fast_copy_byte(dmaInBufferPointer[currentInBufferSelect], inBufferPointer[currentInBufferSelect], sizeOfReadBuffer);

    // quick check of valid data, buffer should always end with LASTBYTE
    if ((inBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1] != LASTBYTE) {
        PolyError_Handler("ERROR | FATAL | com buffer last byte wrong");
#ifdef POLYCONTROL
        println("chip: ", receiveChip);
        println("lastbyte: ", (inBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1]);
        // FlagHandler::renderChip_State[receiveLayer][receiveChip] = READY;
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = INTERCHIPERROR;
#endif
#ifdef POLYRENDER
        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | last byte wrong, receive bus occuppied");
        }
#endif
        return 1;
    }

    // start with offset, as two bytes were size
    for (uint16_t i = 2; i < sizeOfReadBuffer; i++) {
        uint8_t currentByte = (inBufferPointer[currentInBufferSelect])[i];

        switch (currentByte) {

#ifdef POLYRENDER
            case UPDATEINOUTPATCH:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);

                outputID = (inBufferPointer[currentInBufferSelect])[++i];
                inputID = (inBufferPointer[currentInBufferSelect])[++i];
                amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.updatePatchInOutByIdWithoutMapping(outputID, inputID, amountFloat);

                break;

            case CREATEINOUTPATCH:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                outputID = (inBufferPointer[currentInBufferSelect])[++i];
                inputID = (inBufferPointer[currentInBufferSelect])[++i];
                amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.addPatchInOutById(outputID, inputID, amountFloat);

                break;

            case DELETEINOUTPATCH:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                outputID = (inBufferPointer[currentInBufferSelect])[++i];
                inputID = (inBufferPointer[currentInBufferSelect])[++i];

                if (layerID == layerA.id)
                    layerA.removePatchInOutById(outputID, inputID);

                break;

            case DELETEALLPATCHES:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                // delete all patchesInOut at once
                if (layerID == layerA.id)
                    layerA.clearPatches();
                break;

            case NEWNOTE:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                note = (inBufferPointer[currentInBufferSelect])[++i];
                velocity = (inBufferPointer[currentInBufferSelect])[++i];

                if (voice != NOVOICE && layerID == layerA.id) {
                    // println("new note played");

                    layerA.setNote(voice, note, velocity);
                    layerA.gateOn(voice);
                }

                break;

            case OPENGATE:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOn(voice);
                break;

            case CLOSEGATE:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOff(voice);
                break;

            case RESETALL:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                // clear everything
                if (layerID == layerA.id)
                    layerA.resetLayer();
                break;

            case RETRIGGER:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.getModules()[module]->resetPhase(voice);
                break;

            case UPDATESETTINGINT:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                setting = (inBufferPointer[currentInBufferSelect])[++i];
                amountInt = *(int32_t *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(int32_t) - 1

                if (layerID == layerA.id) {
                    layerA.getModules()[module]->getSwitches()[setting]->setValueWithoutMapping(amountInt);
                }

                break;

            case UPDATESETTINGFLOAT:
                readLayerModuleVoice((uint8_t *)&layerID, (uint8_t *)&module, (uint8_t *)&voice,
                                     &(inBufferPointer[currentInBufferSelect])[++i]);
                setting = (inBufferPointer[currentInBufferSelect])[++i];

                amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id) {

                    layerA.getModules()[module]->getPotis()[setting]->setValueWithoutMapping(amountFloat);
                }

                break;

            case SENDUPDATETOCONTROL:
                // TODO send UI update shit
                // HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_RESET);
                sendString("Update to Control"); // temporary stuff
                println("send trans");
                if (beginSendTransmission() != BUS_OK)
                    PolyError_Handler("ERROR | FATAL | send command, send bus occuppied");
                // sendUpdateToRender = true;
                break;

            case LASTBYTE:
                // *((uint16_t *)(dmaInBufferPointer[currentInBufferSelect])) = 0;

                state = COM_READY;

                if (spi->state == BUS_READY)
                    if (beginReceiveTransmission() != BUS_OK)
                        PolyError_Handler("ERROR | FATAL | last received, receive bus occuppied");
                return 0;

#endif
#ifdef POLYCONTROL
                // TODO add decoding here

            case SENDMESSAGE:

                messagebuffer.clear();

                messagesize = (inBufferPointer[currentInBufferSelect])[++i];

                for (uint8_t m = 0; m < messagesize; m++) {
                    messagebuffer += (char)(inBufferPointer[currentInBufferSelect])[++i];
                }

                // print("layer ", receiveLayer);
                // println(", chip ", receiveChip, ":");
                // println(messagebuffer);

                break;

            case UPDATEINPUT: println("UPDATEINPUT"); break;
            case UPDATEOUTPUTINT: println("UPDATEOUTPUTINT"); break;
            case UPDATEOUTPUTFLOAT: println("UPDATEOUTPUTFLOAT"); break;
            case LASTBYTE:
                // *((uint16_t *)(dmaInBufferPointer[currentInBufferSelect])) = 0;
                state = COM_READY;
                // FlagHandler::renderChip_State[receiveLayer][receiveChip] = READY;
                return 0;
                break;

#endif

            case NOCOMMAND: PolyError_Handler("ERROR | FATAL | command was empty"); break;

            default:
                // something went wrong here
                PolyError_Handler("ERROR | FATAL | Decode error");
                state = COM_READY;

                return 1;
        }
    }

    // we should always exit with LASTBYTE
    PolyError_Handler("no LASTBYTE received");
    state = COM_READY;

    return 1;
}

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
    // buffer has been switched already
    outBuffer[!currentOutBufferSelect].push_back(comCommand[0]);

    return 0;
}

// reserve place for size byte
void COMinterChip::pushDummySizePlaceHolder() {
    uint16_t dummysize = 2;
    pushOutBuffer((uint8_t *)&dummysize, 2);
}

// add single byte to out buffer
uint8_t COMinterChip::pushOutBuffer(uint8_t data) {
    if (outBuffer[currentOutBufferSelect].size() + 1 >= INTERCHIPBUFFERSIZE - LASTBYTECMDSIZE) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    outBuffer[currentOutBufferSelect].push_back(data);
    return 0;
}

// add byte buffer to out buffer
uint8_t COMinterChip::pushOutBuffer(uint8_t *data, uint32_t length) {
    if (outBuffer[currentOutBufferSelect].size() + length >= INTERCHIPBUFFERSIZE - LASTBYTECMDSIZE) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    for (uint32_t i = 0; i < length; i++) {
        outBuffer[currentOutBufferSelect].push_back(data[i]);
    }
    return 0;
}

void COMinterChip::resetCom() {
    println("COMERROR");

#ifdef POLYCONTROL

    __disable_irq();
    volatile uint8_t data[50] = {0};
    volatile uint8_t dataIn[50] = {0};
    *(uint16_t *)data = 50;
    spi->state = BUS_READY;
    state = COM_READY;
    outBuffer[0].clear();
    outBuffer[1].clear();

    setCSLine(0, 0, GPIO_PIN_RESET);
    spi->receive((uint8_t *)dataIn, 50, false);
    setCSLine(0, 0, GPIO_PIN_SET);

    setCSLine(0, 1, GPIO_PIN_RESET);
    spi->receive((uint8_t *)dataIn, 50, false);
    setCSLine(0, 1, GPIO_PIN_SET);

    setCSLine(0, 0, GPIO_PIN_RESET);
    spi->receive((uint8_t *)dataIn, 50, false);
    setCSLine(0, 0, GPIO_PIN_SET);

    setCSLine(0, 1, GPIO_PIN_RESET);
    spi->receive((uint8_t *)dataIn, 50, false);
    setCSLine(0, 1, GPIO_PIN_SET);

    setCSLine(0, 0, GPIO_PIN_RESET);
    setCSLine(0, 1, GPIO_PIN_RESET);

    spi->transmit((uint8_t *)data, 50, false);

    setCSLine(0, 0, GPIO_PIN_SET);
    setCSLine(0, 1, GPIO_PIN_SET);

    pushDummySizePlaceHolder();

    __enable_irq();
#endif
}

busState COMinterChip::invokeBufferFullSend() {

#ifdef POLYRENDER
    PolyError_Handler("ERROR | COMMUNICATION | COM -> Buffer full ");
    return BUS_ERROR;
#endif

    volatile busState ret = beginSendTransmission();

    if (ret != BUS_OK) {
        elapsedMillis timer = 0;
        while (ret != BUS_OK) { // wait...
            if (timer > 100) {
                // PolyError_Handler("ERROR | COMMUNICATION | COM -> TIMEOUT > 100ms ");
                // resetCom();
                return BUS_OK;
            }

            // FlagHandler::handleFlags(); // so we don't get stuck
            ret = beginSendTransmission();
        }
    }

    return ret;
}