#include "com.hpp"
#include "layer/layer.hpp"
#include <cstring>

extern Layer layerA;

#ifdef POLYCONTROL
extern Layer layerB;
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

    // println(voiceID);

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

    if (sentRequestUICommand == false) {
        uint8_t comCommand;

        comCommand = SENDUPDATETOCONTROL;
        pushOutBuffer(comCommand);

        sentRequestUICommand = true;
    }

    return 0;
}

uint8_t COMinterChip::sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, int32_t amount) {
    uint8_t comCommand[SETTINGCMDSIZE];
    comCommand[0] = UPDATESETTINGINT;
    comCommand[1] = (layerId << 7) | (modulID << 3);
    comCommand[2] = settingID;
    *(int32_t *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, SETTINGCMDSIZE);
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
        chipState[receiveLayer][receiveChip] = CHIP_ERROR;
        return BUS_ERROR;
    }

    if (receiveSize < 2) {
        println("retreive buffer was 0, chip", receiveChip);
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

uint8_t COMinterChip::sendRenderbuffer(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[RENDERBUFFERCMDSIZE];
    comCommand[0] = UPDATERENDERBUFFER;
    comCommand[1] = (modulID << 4) | settingID;
    *(float *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, RENDERBUFFERCMDSIZE);
    return 0;
}

uint8_t COMinterChip::sendRenderbufferVoice(uint8_t modulID, uint8_t settingID, uint8_t voice, float amount) {
    uint8_t comCommand[RENDERBUFFERCMDSIZEVOICE];
    comCommand[0] = UPDATERENDERBUFFERVOICE;
    comCommand[1] = (modulID << 4) | settingID;
    comCommand[2] = voice;

    *(float *)(&comCommand[3]) = amount;

    pushOutBuffer(comCommand, RENDERBUFFERCMDSIZEVOICE);
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

    if (!(HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_1_Pin) &&
          HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_2_Pin))) {
        // println("blocked send");
        return BUS_BUSY;
    }

//  if (((!(HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_1_Pin) &&
//          HAL_GPIO_ReadPin(Layer_1_READY_1_GPIO_Port, Layer_1_READY_2_Pin))) &&
//       layerA.layerState.value) ||
//
//      ((!(HAL_GPIO_ReadPin(Layer_2_READY_1_GPIO_Port, Layer_2_READY_1_Pin) &&
//          HAL_GPIO_ReadPin(Layer_2_READY_1_GPIO_Port, Layer_2_READY_2_Pin))) &&
//       layerB.layerState.value)) {
//      println("blocked send");
//      return BUS_BUSY;
//  }
//
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

    if (sentRequestUICommand) {
        if (layerA.layerState.value) {
            chipState[0][0] = CHIP_WAITFORDATA;
            chipState[0][1] = CHIP_WAITFORDATA;
            chipStateTimeout[0][0] = 0;
            chipStateTimeout[0][1] = 0;
        }

        if (layerB.layerState.value) {
            chipState[1][0] = CHIP_WAITFORDATA;
            chipState[1][1] = CHIP_WAITFORDATA;
            chipStateTimeout[1][0] = 0;
            chipStateTimeout[1][1] = 0;
        }
    }
    else {
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
    }
#endif

    fast_copy_f32((uint32_t *)outBuffer[!currentOutBufferSelect].data(),
                  (uint32_t *)dmaOutBufferPointer[!currentOutBufferSelect], (dmaOutCurrentBufferSize + 4) >> 2);

    // fast_copy_byte(outBuffer[!currentOutBufferSelect].data(), dmaOutBufferPointer[!currentOutBufferSelect],
    //                dmaOutCurrentBufferSize);

#ifdef POLYCONTROL
    // enable NSS to Render Chip A

    // TODO check CS pins
    if (layerA.layerState.value) {
        HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_RESET);
    }
    if (layerB.layerState.value) {
        HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_RESET);
    }

#endif

    return spi->transmit(dmaOutBufferPointer[!currentOutBufferSelect], dmaOutCurrentBufferSize, true);
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
inline void readLayerModuleVoice(uint8_t &layerID, uint8_t &module, uint8_t &voice, const uint8_t currentByte) {
    layerID = (currentByte & CMD_LAYERMASK) >> 7;
    module = (currentByte & CMD_MODULEMASK) >> 3;
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

    // copy dma buffer to local space with word speed
    fast_copy_f32((uint32_t *)(dmaInBufferPointer[currentInBufferSelect]),
                  (uint32_t *)(inBufferPointer[currentInBufferSelect]), (sizeOfReadBuffer + 4) >> 2);
    // fast_copy_byte(dmaInBufferPointer[currentInBufferSelect], inBufferPointer[currentInBufferSelect],
    // sizeOfReadBuffer);

    // quick check of valid data, buffer should always end with LASTBYTE
    if ((inBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1] != LASTBYTE) {
        PolyError_Handler("ERROR | FATAL | com buffer last byte wrong");
#ifdef POLYCONTROL
        println("chip: ", receiveChip);
        println("lastbyte: ", (inBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1]);
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
    for (uint16_t i = 2; i < sizeOfReadBuffer; i++) {
        uint8_t currentByte = (inBufferPointer[currentInBufferSelect])[i];
        switch (currentByte) {

#ifdef POLYRENDER

            case CREATEINOUTPATCH: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);

                uint8_t outputID = (inBufferPointer[currentInBufferSelect])[++i];
                uint8_t inputID = (inBufferPointer[currentInBufferSelect])[++i];
                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.addPatchInOutById(outputID, inputID, amountFloat);

                break;
            }

            case UPDATEINOUTPATCH: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);

                uint8_t outputID = (inBufferPointer[currentInBufferSelect])[++i];
                uint8_t inputID = (inBufferPointer[currentInBufferSelect])[++i];
                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id)
                    layerA.updatePatchInOutByIdWithoutMapping(outputID, inputID, amountFloat);

                break;
            }

            case DELETEINOUTPATCH: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);

                uint8_t outputID = (inBufferPointer[currentInBufferSelect])[++i];
                uint8_t inputID = (inBufferPointer[currentInBufferSelect])[++i];

                if (layerID == layerA.id)
                    layerA.removePatchInOutById(outputID, inputID);

                break;
            }

            case DELETEALLPATCHES: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                // delete all patchesInOut at once
                if (layerID == layerA.id)
                    layerA.clearPatches();
                break;
            }

            case NEWNOTE: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                uint8_t note = (inBufferPointer[currentInBufferSelect])[++i];
                uint8_t velocity = (inBufferPointer[currentInBufferSelect])[++i];

                if (voice != NOVOICE && layerID == layerA.id) {
                    // println("new note played");

                    layerA.setNote(voice, note, velocity);
                    layerA.gateOn(voice);
                }

                break;
            }

            case OPENGATE: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOn(voice);
                break;
            }

            case CLOSEGATE: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.gateOff(voice);
                break;
            }

            case RESETALL: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                // clear everything
                if (layerID == layerA.id)
                    layerA.resetLayer();
                break;
            }

            case RETRIGGER: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                if (voice != NOVOICE && layerID == layerA.id)
                    layerA.getModules()[module]->retrigger(voice);
                break;
            }

            case UPDATESETTINGINT: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                uint8_t setting = (inBufferPointer[currentInBufferSelect])[++i];
                int32_t amountInt = *(int32_t *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(int32_t) - 1

                if (layerID == layerA.id) {
                    layerA.getModules()[module]->getSwitches()[setting]->setValueWithoutMapping(amountInt);
                }

                break;
            }

            case UPDATESETTINGFLOAT: {
                uint8_t layerID, module, voice;
                readLayerModuleVoice(layerID, module, voice, (inBufferPointer[currentInBufferSelect])[++i]);
                uint8_t setting = (inBufferPointer[currentInBufferSelect])[++i];

                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];
                i += 3; // sizeof(float) - 1

                if (layerID == layerA.id) {

                    layerA.getModules()[module]->getPotis()[setting]->setValueWithoutMapping(amountFloat);
                }

                break;
            }

            case SENDUPDATETOCONTROL: {
                // println("send trans");
                if (FlagHandler::outputReady == false) {
                    sendString("out not collected");
                }

                if (beginSendTransmission() != BUS_OK)
                    PolyError_Handler("ERROR | FATAL | send command, send bus occuppied");
                FlagHandler::outputCollect = true; // collect next Sample packet;

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

                uint8_t messagesize = (inBufferPointer[currentInBufferSelect])[++i];

                for (uint8_t m = 0; m < messagesize; m++) {
                    messagebuffer += (char)(inBufferPointer[currentInBufferSelect])[++i];
                }

                print("layer ", receiveLayer);
                println(", chip ", receiveChip, ":");
                println(messagebuffer);

                break;
            }

            case UPDATEINPUT: {
                uint8_t module = (inBufferPointer[currentInBufferSelect])[++i];

                uint8_t setting = module & 0xF;
                module = module >> 4;

                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                if (receiveLayer == layerA.id)
                    layerA.modules[module]->renderBuffer[setting]->currentSample[0] = amountFloat;

                if (receiveLayer == layerB.id)
                    layerB.modules[module]->renderBuffer[setting]->currentSample[0] = amountFloat;

                i += 3;

                break;
            }

            case UPDATERENDERBUFFER: {
                uint8_t module = (inBufferPointer[currentInBufferSelect])[++i];

                uint8_t setting = module & 0xF;
                module = module >> 4;

                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                if (receiveLayer == layerA.id)
                    layerA.modules[module]->renderBuffer[setting]->currentSample[0] = amountFloat;

                if (receiveLayer == layerB.id)
                    layerB.modules[module]->renderBuffer[setting]->currentSample[0] = amountFloat;

                i += 3;

                break;
            }

            case UPDATERENDERBUFFERVOICE: {
                uint8_t module = (inBufferPointer[currentInBufferSelect])[++i];

                uint8_t setting = module & 0xF;
                module = module >> 4;
                uint8_t voice = (inBufferPointer[currentInBufferSelect])[++i];

                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                if (receiveLayer == layerA.id)
                    layerA.modules[module]->renderBuffer[setting]->currentSample[voice] = amountFloat;

                if (receiveLayer == layerB.id)
                    layerB.modules[module]->renderBuffer[setting]->currentSample[voice] = amountFloat;

                i += 3;

                break;
            }

            case UPDATEOUTPUTINT: println("UPDATEOUTPUTINT"); break;

            case UPDATEOUTPUTFLOAT: {

                uint8_t module = (inBufferPointer[currentInBufferSelect])[++i];

                uint8_t setting = module & 0xF;
                module = module >> 4;

                float amountFloat = *(float *)&(inBufferPointer[currentInBufferSelect])[++i];

                if (receiveLayer == layerA.id)
                    layerA.modules[module]->outputs[setting]->currentSample[0] = amountFloat;

                if (receiveLayer == layerB.id)
                    layerB.modules[module]->outputs[setting]->currentSample[0] = amountFloat;

                i += 3;

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
    __disable_irq();
    outBuffer[!currentOutBufferSelect].push_back(comCommand[0]);
    __enable_irq();
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
    __disable_irq();
    outBuffer[currentOutBufferSelect].push_back(data);
    __enable_irq();
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
    __disable_irq();
    for (uint32_t i = 0; i < length; i++) {
        outBuffer[currentOutBufferSelect].push_back(data[i]);
    }
    __enable_irq();

    return 0;
}

void COMinterChip::resetCom() {
    // PolyError_Handler("ERROR | COMMUNICATION | COM -> TIMEOUT > 100ms ");

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
    spi->transmit((uint8_t *)dataIn, 50, false);
    setCSLine(0, 0, GPIO_PIN_SET);

    setCSLine(0, 1, GPIO_PIN_RESET);
    spi->transmit((uint8_t *)dataIn, 50, false);
    setCSLine(0, 1, GPIO_PIN_SET);

    setCSLine(0, 0, GPIO_PIN_RESET);
    spi->transmit((uint8_t *)dataIn, 50, false);
    setCSLine(0, 0, GPIO_PIN_SET);

    setCSLine(0, 1, GPIO_PIN_RESET);
    spi->transmit((uint8_t *)dataIn, 50, false);
    setCSLine(0, 1, GPIO_PIN_SET);

    setCSLine(0, 0, GPIO_PIN_RESET);
    setCSLine(0, 1, GPIO_PIN_RESET);

    spi->transmit((uint8_t *)data, 50, false);

    chipState[0][0] = CHIP_READY;
    chipState[0][1] = CHIP_READY;

    chipStateTimeout[0][0] = 0;
    chipStateTimeout[0][1] = 0;

    setCSLine(0, 0, GPIO_PIN_SET);
    setCSLine(0, 1, GPIO_PIN_SET);

    pushDummySizePlaceHolder();

    __enable_irq();
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