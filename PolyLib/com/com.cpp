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

    if (sentRequestUICommand == false) {
        uint8_t comCommand[SENDUPDATETOCONTROLSIZE];

        comCommand[0] = SENDUPDATETOCONTROL;

        // TODO set flags for data reception, separaate flags pls

        pushOutBuffer(comCommand, SENDUPDATETOCONTROLSIZE);

        sentRequestUICommand = true;
    }

    return 0;
}

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

busState COMinterChip::beginReceiveTransmission(uint8_t layer, uint8_t chip) {
    if (spi->state != BUS_READY)
        return spi->state;
    receiveLayer = layer;
    receiveChip = chip;

    setCSLine(receiveLayer, receiveChip, GPIO_PIN_RESET);

    uint16_t receiveSize;

    spi->receive(dmaInBufferPointer[!currentInBufferSelect], 2, false);

    receiveSize = *(uint16_t *)dmaInBufferPointer[!currentInBufferSelect];
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
    comCommand[0] = (modulID << 4) | settingID;
    comCommand[1] = UPDATEOUTPUTINT;
    *(int32_t *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, OUTPUTCMDSIZE);
    return 0;
}
uint8_t COMinterChip::sendOutput(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[OUTPUTCMDSIZE];
    comCommand[0] = (modulID << 4) | settingID;
    ;
    comCommand[1] = UPDATEOUTPUTFLOAT;
    *(float *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, OUTPUTCMDSIZE);
    return 0;
}
uint8_t COMinterChip::sendInput(uint8_t modulID, uint8_t settingID, float amount) {
    uint8_t comCommand[INPUTCMDSIZE];
    comCommand[0] = (modulID << 4) | settingID;
    ;
    comCommand[1] = UPDATEINPUT;
    *(float *)(&comCommand[2]) = amount;

    pushOutBuffer(comCommand, INPUTCMDSIZE);
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
        if (FlagHandler::renderChip_State[0][0] != READY && FlagHandler::renderChip_State[0][1] != READY) {
            return BUS_BUSY;
        }
    }

    if (FlagHandler::layerActive[1]) {
        if (FlagHandler::renderChip_State[1][0] != READY && FlagHandler::renderChip_State[1][1] != READY) {
            return BUS_BUSY;
        }
    }
#endif

    appendLastByte();

    // write size into first two bytes of outBuffers
    dmaOutCurrentBufferSize = outBuffer[currentOutBufferSelect].size();
    *(uint16_t *)outBuffer[currentOutBufferSelect].data() = dmaOutCurrentBufferSize;

    switchOutBuffer();

    busState ret = startSendDMA();

#ifdef POLYRENDER
    if (ret == BUS_OK)
        HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_SET);
#endif

    return ret;
}

busState COMinterChip::startSendDMA() {

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

    return spi->transmit(dmaOutBufferPointer[!currentOutBufferSelect], dmaOutCurrentBufferSize, true);
}

uint8_t COMinterChip::decodeCurrentInBuffer() {

    state = COM_DECODE;

    spi->stopDMA();

#ifdef POLYCONTROL
    setCSLine(receiveLayer, receiveChip, GPIO_PIN_SET);
#endif

#ifdef POLYRENDER
    // necessary decoding vars
    volatile uint8_t outputID = 0;
    volatile uint8_t inputID = 0;
    volatile uint8_t note = 0;
    volatile uint8_t velocity = 0;
    volatile uint8_t layerID = 0;
    volatile uint8_t setting = 0;
    volatile uint8_t module = 0;
    volatile uint8_t voice = 0;
#endif
    volatile int32_t amountInt = 0;
    volatile float amountFloat = 0;

    switchInBuffer();

    // assemble size
    uint16_t sizeOfReadBuffer = *((uint16_t *)(dmaInBufferPointer[currentInBufferSelect]));

    if (sizeOfReadBuffer == 0) {
        state = COM_READY;

#ifdef POLYCONTROL
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = READY;
#endif
#ifdef POLYRENDER
        if (spi->state == BUS_READY) {
            if (beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | size=0, receive bus occuppied");
        }
#endif
        return 0;
    }

    if (sizeOfReadBuffer > INTERCHIPBUFFERSIZE) {
        state = COM_READY;

        // buffer too big, sth went wrong
        PolyError_Handler("ERROR | FATAL | com buffer too big");
#ifdef POLYCONTROL
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = READY;
#endif
        return 1;
    }

    fast_copy_f32((uint32_t *)(dmaInBufferPointer[currentInBufferSelect]),
                  (uint32_t *)(inBufferPointer[currentInBufferSelect]), (sizeOfReadBuffer + 3) >> 2);

    if ((inBufferPointer[currentInBufferSelect])[sizeOfReadBuffer - 1] != LASTBYTE) {
        // last byte must be at this position, sort of CRC
        PolyError_Handler("ERROR | FATAL | com buffer last byte wrong");
#ifdef POLYCONTROL
        FlagHandler::renderChip_State[receiveLayer][receiveChip] = READY;
#endif
        state = COM_READY;

        return 1;
    }

    // start with offset, as two bytes were size
    for (uint16_t i = 2; i < sizeOfReadBuffer; i++) {
        uint8_t currentByte = (inBufferPointer[currentInBufferSelect])[i];

        // this would mean we're done
        if (currentByte == LASTBYTE) {
            *((uint16_t *)(dmaInBufferPointer[currentInBufferSelect])) = 0;

            state = COM_READY;

#ifdef POLYRENDER
            if (spi->state == BUS_READY)
                if (beginReceiveTransmission() != BUS_OK)
                    PolyError_Handler("ERROR | FATAL | receive bus occuppied");
#endif
#ifdef POLYCONTROL
            FlagHandler::renderChip_State[receiveLayer][receiveChip] = READY;
#endif
            return 0;
        }

#ifdef POLYRENDER
        if (currentByte == SENDUPDATETOCONTROL) {
            // TODO send UI update shit
            sendString("Update to Control"); // temporary stuff
            if (beginSendTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | send bus occuppied");
            continue;
        }
#endif
#ifdef POLYCONTROL
        if (currentByte == SENDMESSAGE) {

            messagebuffer.clear();

            uint8_t messagesize = (inBufferPointer[currentInBufferSelect])[++i];

            for (uint8_t m = 0; m < messagesize; m++) {
                messagebuffer += (char)(inBufferPointer[currentInBufferSelect])[++i];
            }

            print("layer ", receiveLayer);
            println(", chip ", receiveChip, ":");
            println(messagebuffer);

            continue;
        }

#endif

#ifdef POLYRENDER
        layerID = (currentByte & CMD_LAYERMASK) >> 7;
        module = (currentByte & CMD_MODULEMASK) >> 3;
        voice = currentByte & CMD_VOICEMASK;

        // convert voice 4-7 to 0-3 if on chip B
        voice = checkVoiceAgainstChipID(voice);
#endif

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

            case UPDATEINPUT: println("UPDATEINPUT"); break;
            case UPDATEOUTPUTINT: println("UPDATEOUTPUTINT"); break;
            case UPDATEOUTPUTFLOAT: println("UPDATEOUTPUTFLOAT"); break;

#endif

            case NOCOMMAND: PolyError_Handler("ERROR | FATAL | command was empty"); return 1;

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

busState COMinterChip::invokeBufferFullSend() {
    busState ret = beginSendTransmission();

    if (ret != BUS_OK) {
        uint32_t timer = millis();
        while (ret != BUS_OK) { // wait...
            if ((millis() - timer) > 100) {
                PolyError_Handler("ERROR | COMMUNICATION | COM -> TIMEOUT > 100ms ");
            }

            FlagHandler::handleFlags(); // so we don't get stuck
            ret = beginSendTransmission();
        }
    }

    return ret;
}