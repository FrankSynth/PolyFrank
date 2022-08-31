#include "com.hpp"
#include "layer/layer.hpp"
#include <cstring>

extern Layer layerA;

#ifdef POLYCONTROL
extern Layer layerB;
extern std::vector<Layer *> allLayers;
extern void setCSLine(uint8_t layer, uint8_t chip, GPIO_PinState state);
#endif

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
    for (uint32_t i = 0; i < size; i++) {
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
    for (uint32_t i = 0; i < size; i++) {
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

    // println(voiceID);

    if (voiceID == VOICEALL) {
        for (uint32_t voice = 0; voice < VOICEALL; voice++) {
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
        for (uint32_t voice = 0; voice < VOICEALL; voice++) {
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
        for (uint32_t voice = 0; voice < VOICEALL; voice++) {
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
    comCommand[1] = layerId << 7;
    comCommand[2] = modulID;

    if (voiceID == VOICEALL) {
        for (uint32_t voice = 0; voice < VOICEALL; voice++) {
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

uint8_t COMinterChip::sendRequestUIData(uint8_t layer, uint8_t chip) {

    if (singleChipRequested == false) {
        receiveLayer = layer;
        receiveChip = chip;
        uint8_t comCommand[SENDUPDATETOCONTROLSIZE];

        comCommand[0] = SENDUPDATETOCONTROL;
        comCommand[1] = (layer << 7) | chip;
        pushOutBuffer(comCommand, SENDUPDATETOCONTROLSIZE);
        singleChipRequested = true;
        requestState[receiveLayer][receiveChip] = RQ_DATAREQUESTED;
    }

    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = UPDATESETTINGINT;
    comCommand[1] = layerId << 7;
    comCommand[2] = modulID;
    comCommand[3] = settingID;
    *(int32_t *)(&comCommand[4]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = UPDATESETTINGFLOAT;
    comCommand[1] = layerId << 7;
    comCommand[2] = modulID;
    comCommand[3] = settingID;
    *(float *)(&comCommand[4]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
    return 0;
}

busState COMinterChip::beginReceiveTransmission(uint8_t layer, uint8_t chip) {
    uint16_t receiveSize;

    if (!requestSize) {

        if (spi->state != BUS_READY)
            return spi->state;

        if (layerA.layerState.value) {
            if (!((chipState[0][0] == CHIP_READY || chipState[0][0] == CHIP_DATAREADY) &&
                  (chipState[0][1] == CHIP_READY || chipState[0][1] == CHIP_DATAREADY))) {
                return BUS_BUSY;
            }
        }

        if (layerB.layerState.value) {
            if (!((chipState[1][0] == CHIP_READY || chipState[1][0] == CHIP_DATAREADY) &&
                  (chipState[1][1] == CHIP_READY || chipState[1][1] == CHIP_DATAREADY))) {
                return BUS_BUSY;
            }
        }

        chipState[receiveLayer][receiveChip] = CHIP_DATAFLOWING;

        setCSLine(receiveLayer, receiveChip, GPIO_PIN_RESET);

        requestSize = true;
        spi->receive(dmaInBufferPointer[!currentInBufferSelect], 2, true);
    }
    else {

        if (spi->state != BUS_READY)
            return spi->state;

        requestSize = false;

        receiveSize = *(uint16_t *)dmaInBufferPointer[!currentInBufferSelect];

        if (receiveSize > INTERCHIPBUFFERSIZE) {
            println(receiveSize);
            PolyError_Handler("ERROR | FATAL | com buffer too big");
            chipState[receiveLayer][receiveChip] = CHIP_ERROR;
            return BUS_ERROR;
        }

        if (receiveSize < 2) {
            println("retreive buffer was 0, chip", receiveChip);
            return BUS_OK;
        }

        spi->receive(&dmaInBufferPointer[!currentInBufferSelect][2], receiveSize - 2, true);
    }
    return BUS_OK;
}

#endif

#ifdef POLYRENDER

uint8_t COMinterChip::sendString(const std::string &message) {

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

uint8_t COMinterChip::sendOutput(uint8_t modulID, uint8_t settingID, vec<VOICESPERCHIP> &amount) {
    uint8_t comCommand[OUTPUTCMDSIZE];
    comCommand[0] = UPDATEOUTPUTFLOAT;
    comCommand[1] = modulID;
    comCommand[2] = settingID;

    *(float *)(&comCommand[3]) = amount[0];
    *(float *)(&comCommand[7]) = amount[1];
    *(float *)(&comCommand[11]) = amount[2];
    *(float *)(&comCommand[15]) = amount[3];

    pushOutBuffer(comCommand, OUTPUTCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendRenderbuffer(uint8_t modulID, uint8_t settingID, vec<VOICESPERCHIP> &amount) {
    uint8_t comCommand[RENDERBUFFERCMDSIZE];
    comCommand[0] = UPDATERENDERBUFFER;
    comCommand[1] = modulID;
    comCommand[2] = settingID;
    *(float *)(&comCommand[3]) = amount[0];
    *(float *)(&comCommand[7]) = amount[1];
    *(float *)(&comCommand[11]) = amount[2];
    *(float *)(&comCommand[15]) = amount[3];
    pushOutBuffer(comCommand, RENDERBUFFERCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendAudioBuffer(int8_t *audioData) {

    if (dmaOutCurrentBufferSize[currentOutBufferSelect] + UPDATEAUDIOBUFFERSIZE >=
        INTERCHIPBUFFERSIZE - LASTBYTECMDSIZE) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }
    __disable_irq();
    dmaOutBufferPointer[currentOutBufferSelect][dmaOutCurrentBufferSize[currentOutBufferSelect]++] =
        (uint8_t)UPDATEAUDIOBUFFER;

    for (uint32_t i = 0; i < 300; i++) {
        dmaOutBufferPointer[currentOutBufferSelect][dmaOutCurrentBufferSize[currentOutBufferSelect]++] =
            *(uint8_t *)&audioData[i];
    }

    __enable_irq();

    return 0;
}

busState COMinterChip::beginReceiveTransmission() {
    busState ret = spi->receive(dmaInBufferPointer[!currentInBufferSelect], INTERCHIPBUFFERSIZE, true);
    if (ret == BUS_OK)
        HAL_GPIO_WritePin(SPI_READY_GPIO_Port, SPI_READY_Pin, GPIO_PIN_SET);
    return ret;
}

#endif

/////////////////////////// MASTER & SLAVE ////////////////////////

busState COMinterChip::beginSendTransmission() {

#ifdef POLYCONTROL
    // don't send buffer if there is nothing except the size
    if (dmaOutCurrentBufferSize[currentOutBufferSelect] < 3) {
        return BUS_OK;
    }
#endif

    // block new transmissions as long as a transmission is already running
    if (spi->state != BUS_READY) {
        return spi->state;
    }

#ifdef POLYCONTROL
    if (layerA.layerState.value) {
        if ((chipState[0][0] != CHIP_READY) || (chipState[0][1] != CHIP_READY)) {
            return BUS_BUSY;
        }
    }

    if (layerB.layerState.value) {
        if ((chipState[1][0] != CHIP_READY) || (chipState[1][1] != CHIP_READY)) {
            return BUS_BUSY;
        }
    }

    if (((!(HAL_GPIO_ReadPin(SPI_READY_LAYER_1A_GPIO_Port, SPI_READY_LAYER_1A_Pin) &&
            HAL_GPIO_ReadPin(SPI_READY_LAYER_1B_GPIO_Port, SPI_READY_LAYER_1B_Pin))) &&
         layerA.layerState.value) ||

        ((!(HAL_GPIO_ReadPin(SPI_READY_LAYER_2A_GPIO_Port, SPI_READY_LAYER_2A_Pin) &&
            HAL_GPIO_ReadPin(SPI_READY_LAYER_2B_GPIO_Port, SPI_READY_LAYER_2B_Pin))) &&
         layerB.layerState.value)) {
        println("blocked send");
        return BUS_BUSY;
    }
#endif

    __disable_irq();

    switchOutBuffer();
    appendLastByte();
    *(uint16_t *)dmaOutBufferPointer[!currentOutBufferSelect] = dmaOutCurrentBufferSize[!currentOutBufferSelect];

    busState ret = startSendDMA();

#ifdef POLYRENDER
    if (ret == BUS_OK) {
        HAL_GPIO_WritePin(SPI_READY_GPIO_Port, SPI_READY_Pin, GPIO_PIN_SET);
    }
#endif

    __enable_irq();

    return ret;
}

busState COMinterChip::startSendDMA() {

    // set Interchip state and wait for response

#ifdef POLYCONTROL

    if (layerA.layerState.value) {
        chipState[0][0] = CHIP_DATASENT;
        chipState[0][1] = CHIP_DATASENT;
        chipStateTimeout[0][0] = 0;
        chipStateTimeout[0][1] = 0;
    }

    if (layerB.layerState.value) {
        chipState[1][0] = CHIP_DATASENT;
        chipState[1][1] = CHIP_DATASENT;
        chipStateTimeout[1][0] = 0;
        chipStateTimeout[1][1] = 0;
    }

    if (singleChipRequested) {
        chipState[receiveLayer][receiveChip] = CHIP_WAITFORDATA;
    }

    if (layerA.layerState.value) {
        HAL_GPIO_WritePin(SPI_CS_Layer_1A_GPIO_Port, SPI_CS_Layer_1A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SPI_CS_Layer_1B_GPIO_Port, SPI_CS_Layer_1B_Pin, GPIO_PIN_RESET);
    }
    if (layerB.layerState.value) {
        HAL_GPIO_WritePin(SPI_CS_Layer_2A_GPIO_Port, SPI_CS_Layer_2A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SPI_CS_Layer_2B_GPIO_Port, SPI_CS_Layer_2B_Pin, GPIO_PIN_RESET);
    }
#endif

    return spi->transmit(dmaOutBufferPointer[!currentOutBufferSelect], dmaOutCurrentBufferSize[!currentOutBufferSelect],
                         true);
}

#ifdef POLYRENDER

// check if voices beling to current render chip, and shift if necessary
inline uint8_t checkVoiceAgainstChipID(uint8_t voice) {
    if (layerA.chipID == 0) {
        if (voice >= VOICESPERCHIP)
            voice = NOVOICE;
    }
    else {
        if (voice < VOICESPERCHIP)
            voice = NOVOICE;
        else
            voice = voice - VOICESPERCHIP;
    }
    return voice;
}

// extracts layer, module, voice from currentByte
inline void readLayerVoice(uint8_t &layerID, uint8_t &voice, const uint8_t currentByte) {
    layerID = (currentByte & CMD_LAYERMASK) >> 7;
    voice = currentByte & CMD_VOICEMASK;
    voice = checkVoiceAgainstChipID(voice);
}

#endif

uint8_t COMinterChip::decodeCurrentInBuffer() {

    state = COM_DECODE;

    switchInBuffer();

    // assemble size
    uint16_t sizeOfReadBuffer = *((uint16_t *)(dmaInBufferPointer[currentInBufferSelect]));

    // no data received
    if (sizeOfReadBuffer == 0) {

        state = COM_READY;

#ifdef POLYRENDER
        PolyError_Handler("ERROR | FATAL | size=0");

        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | size=0, bus occuppied");
        }
#endif
        return 0;
    }

    if (sizeOfReadBuffer > INTERCHIPBUFFERSIZE) {
        state = COM_READY;
        println("com size: ", sizeOfReadBuffer);
        PolyError_Handler("ERROR | FATAL | decode com buffer too big");
#ifdef POLYCONTROL
        chipState[receiveLayer][receiveChip] = CHIP_ERROR;
#endif
#ifdef POLYRENDER
        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | too big, bus occuppied");
        }
#endif
        return 1;
    }

    // quick check of valid data, buffer should always end with LASTBYTE
    if ((dmaInBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1] != LASTBYTE) {
        PolyError_Handler("ERROR | FATAL | com buffer last byte wrong");
#ifdef POLYCONTROL
        println("chip: ", receiveChip);
        println("lastbyte: ", (dmaInBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1]);
        chipState[receiveLayer][receiveChip] = CHIP_ERROR;

#endif
#ifdef POLYRENDER
        state = COM_READY;
        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | last byte wrong, receive bus occuppied");
        }
#endif
        return 1;
    }

    // start with offset, as two bytes were size
    for (uint32_t i = 2; i < sizeOfReadBuffer; i++) {
        uint8_t currentByte = (dmaInBufferPointer[currentInBufferSelect])[i];
        switch (currentByte) {

#ifdef POLYRENDER

            case CREATEINOUTPATCH: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);

                volatile uint8_t outputID = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile uint8_t inputID = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile float amountFloat = *(float *)&(dmaInBufferPointer[currentInBufferSelect])[++i];

                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.addPatchInOutById(outputID, inputID, amountFloat);

                break;
            }

            case UPDATEINOUTPATCH: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);

                volatile uint8_t outputID = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile uint8_t inputID = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile float amountFloat = *(float *)&(dmaInBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.updatePatchInOutByIdWithoutMapping(outputID, inputID, amountFloat);

                break;
            }

            case DELETEINOUTPATCH: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);

                volatile uint8_t outputID = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile uint8_t inputID = (dmaInBufferPointer[currentInBufferSelect])[++i];

                if (layerID == layerA.id)
                    layerA.removePatchInOutById(outputID, inputID);

                break;
            }

            case DELETEALLPATCHES: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                // delete all patchesInOut at once
                if (layerID == layerA.id)
                    layerA.clearPatches();
                break;
            }

            case NEWNOTE: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                volatile uint8_t note = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile uint8_t velocity = (dmaInBufferPointer[currentInBufferSelect])[++i];

                if (voice != NOVOICE && layerID == layerA.id) {
                    // println("new note played");

                    layerA.setNote(voice, note, velocity);
                    layerA.gateOn(voice);
                }

                break;
            }

            case OPENGATE: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);

                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOn(voice);
                break;
            }

            case CLOSEGATE: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOff(voice);
                break;
            }

            case RESETALL: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                // clear everything
                if (layerID == layerA.id)
                    layerA.resetLayer();
                break;
            }

            case RETRIGGER: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                volatile uint8_t module = (dmaInBufferPointer[currentInBufferSelect])[++i];
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.getModules()[module]->retrigger(voice);
                break;
            }

            case UPDATESETTINGINT: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                volatile uint8_t module = (dmaInBufferPointer[currentInBufferSelect])[++i];

                volatile uint8_t setting = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile int32_t amountInt = *(int32_t *)&(dmaInBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(int32_t) - 1

                if (layerID == layerA.id) {
                    layerA.getModules()[module]->getSwitches()[setting]->setValueWithoutMapping(amountInt);
                }

                break;
            }

            case UPDATESETTINGFLOAT: {
                uint8_t layerID, voice;
                readLayerVoice(layerID, voice, (dmaInBufferPointer[currentInBufferSelect])[++i]);
                volatile uint8_t module = (dmaInBufferPointer[currentInBufferSelect])[++i];

                volatile uint8_t setting = (dmaInBufferPointer[currentInBufferSelect])[++i];

                float amountFloat = *(float *)&(dmaInBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id) {

                    layerA.getModules()[module]->getPotis()[setting]->setValueWithoutMapping(amountFloat);
                }

                break;
            }

            case SENDUPDATETOCONTROL: {
                volatile uint8_t layerID, chip;
                currentByte = (dmaInBufferPointer[currentInBufferSelect])[++i];
                layerID = (currentByte & CMD_LAYERMASK) >> 7;
                chip = currentByte & 0x1;

                if (layerID == layerA.id && chip == layerA.chipID) {

                    if (FlagHandler::outputReady == false) {
                        sendString("out not collected");
                    }

                    if (beginSendTransmission() != BUS_OK)
                        PolyError_Handler("ERROR | FATAL | send command, send bus occuppied");
                    FlagHandler::outputCollect = true; // collect next Sample packet;
                    FlagHandler::outputReady = false;
                }
                break;
            }

            case LASTBYTE: {

                state = COM_READY;

                if (spi->state == BUS_READY)
                    if (beginReceiveTransmission() != BUS_OK)
                        PolyError_Handler("ERROR | FATAL | last received, receive bus occuppied");
                return 0;
            }

#endif
#ifdef POLYCONTROL

            case SENDMESSAGE: {

                messagebuffer.clear();

                uint8_t messagesize = (dmaInBufferPointer[currentInBufferSelect])[++i];

                for (uint8_t m = 0; m < messagesize; m++) {
                    messagebuffer += (char)(dmaInBufferPointer[currentInBufferSelect])[++i];
                }

                print("layer ", receiveLayer);
                print(", chip ", receiveChip, ": ");
                println(messagebuffer);

                break;
            }

            case UPDATERENDERBUFFER: {
                volatile uint8_t module = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile uint8_t setting = (dmaInBufferPointer[currentInBufferSelect])[++i];
                // __disable_irq();
                for (int v = 0; v < 4; v++) {
                    float amountFloat = *(float *)&(dmaInBufferPointer[currentInBufferSelect])[++i];
                    allLayers[receiveLayer]->modules[module]->renderBuffer[setting]->currentSample[v +
                                                                                                   4 * (receiveChip)] =
                        amountFloat;
                    i += 3;
                }
                // __enable_irq();

                break;
            }

            case UPDATEOUTPUTFLOAT: {

                volatile uint8_t module = (dmaInBufferPointer[currentInBufferSelect])[++i];
                volatile uint8_t setting = (dmaInBufferPointer[currentInBufferSelect])[++i];
                // __disable_irq();
                for (int v = 0; v < 4; v++) {
                    float amountFloat = *(float *)&(dmaInBufferPointer[currentInBufferSelect])[++i];
                    allLayers[receiveLayer]->modules[module]->outputs[setting]->currentSample[v + 4 * (receiveChip)] =
                        amountFloat;
                    i += 3;
                }
                // __enable_irq();

                break;
            }
            case UPDATEAUDIOBUFFER: {

                for (uint32_t k = 0; k < 300; k++) {
                    allLayers[receiveLayer]->renderedAudioWaves[k] =
                        *(int8_t *)&(dmaInBufferPointer[currentInBufferSelect])[++i];
                }
                break;
            }

            case LASTBYTE:
                state = COM_READY;
                return 0;
                break;

#endif

            case NOCOMMAND: PolyError_Handler("ERROR | FATAL | command was empty"); break;

            default:

                println(currentByte);
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
    // *(uint16_t *)dmaInBufferPointer[!currentOutBufferSelect] = 2;
}

void COMinterChip::switchOutBuffer() {
    currentOutBufferSelect = !currentOutBufferSelect;

    // outBuffer[currentOutBufferSelect].clear();
    dmaOutCurrentBufferSize[currentOutBufferSelect] = 0;
    pushDummySizePlaceHolder();
}

// wrap up out buffers
uint8_t COMinterChip::appendLastByte() {
    uint8_t comCommand = LASTBYTE;

    // buffer has been switched already
    dmaOutBufferPointer[!currentOutBufferSelect][dmaOutCurrentBufferSize[!currentOutBufferSelect]++] = comCommand;
    return 0;
}

// reserve place for size byte
void COMinterChip::pushDummySizePlaceHolder() {
    uint16_t dummysize = 2;
    pushOutBuffer((uint8_t *)&dummysize, 2);
}

// add single byte to out buffer
uint8_t COMinterChip::pushOutBuffer(uint8_t data) {
    if (dmaOutCurrentBufferSize[currentOutBufferSelect] + 1 >= INTERCHIPBUFFERSIZE - LASTBYTECMDSIZE) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    // println("Index : ", dmaOutCurrentBufferSize[currentOutBufferSelect]);
    // println("pointer : ", (uint32_t)dmaOutBufferPointer[currentOutBufferSelect]);

    __disable_irq();

    dmaOutBufferPointer[currentOutBufferSelect][dmaOutCurrentBufferSize[currentOutBufferSelect]++] = data;

    __enable_irq();
    return 0;
}

// add byte buffer to out buffer
uint8_t COMinterChip::pushOutBuffer(uint8_t *data, uint32_t length) {

    if (dmaOutCurrentBufferSize[currentOutBufferSelect] + length >= INTERCHIPBUFFERSIZE - LASTBYTECMDSIZE) {
        uint8_t ret = invokeBufferFullSend();
        if (ret) {
            return ret;
        }
    }

    // println("Index : ", dmaOutCurrentBufferSize[currentOutBufferSelect]);
    // println("pointer : ", (uint32_t)dmaOutBufferPointer[currentOutBufferSelect]);

    __disable_irq();
    for (uint32_t i = 0; i < length; i++) {
        dmaOutBufferPointer[currentOutBufferSelect][dmaOutCurrentBufferSize[currentOutBufferSelect]++] = data[i];
    }
    __enable_irq();

    return 0;
}

void COMinterChip::resetCom() {

#ifdef POLYCONTROL
    PolyError_Handler("ERROR | COMMUNICATION | COM -> TIMEOUT > 1000ms ");
#endif
}

busState COMinterChip::invokeBufferFullSend() {
#ifdef POLYRENDER
    PolyError_Handler("ERROR | COMMUNICATION | COM -> Buffer full ");
    return BUS_OK;
#endif
    busState ret = beginSendTransmission();

    if (ret != BUS_OK) {
        elapsedMillis timer = 0;
        while (ret != BUS_OK) { // wait...
            if (timer > 1000) {
                resetCom();
                return BUS_OK;
            }
            ret = beginSendTransmission();
        }
    }

    return ret;
}