#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "comCommands.h"
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "flagHandler/flagHandler.hpp"
#include "mdma.h"
#include "poly.hpp"
#include "spi.h"
#include <functional>
#include <inttypes.h>
#include <vector>

#define INPUTBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
#define INTERCHIPBUFFERSIZE 1024

#define ERRORCODE_SENDBLOCK 20

// callback for MDMA operations
void comMDMACallback(MDMA_HandleTypeDef *_hmdma);

// class for USB communication
// beginUSBTransmission has to be executed to send Buffers via USB
// Constructor needs Transmit function
class COMusb {
  public:
    COMusb(uint8_t (*sendViaUSB)(uint8_t *, uint16_t) = nullptr) {
        this->sendViaUSB = sendViaUSB;
        wBuffer[0].reserve(OUTPUTBUFFERSIZE);
        wBuffer[1].reserve(OUTPUTBUFFERSIZE);
    }
    ~COMusb() {}

    // function pointer to USB Transmit function
    uint8_t (*sendViaUSB)(uint8_t *, uint16_t);

    // return if new Data is in rBuffer
    bool available();

    // return oldest data
    const uint8_t read();

    // add single byte to output buffer
    // call beginUSBTransmission() to start USB transmit
    uint8_t write(uint8_t data);

    // add array of bytes to output buffer
    // call beginUSBTransmission() to start USB transmit
    uint8_t write(uint8_t *data, uint16_t size);

    // send output buffer via usb
    uint8_t beginUSBTransmission();

    // push byte array in Input Buffer
    uint8_t push(uint8_t *data, uint32_t length);

    // push single byte in Input Buffer
    uint8_t push(uint8_t data);

  private:
    CircularBuffer<uint8_t, INPUTBUFFERSIZE> rBuffer; // read buffer
    std::vector<uint8_t> wBuffer[2];                  // write buffer, size of BUFFERSIZE gets reserved in allocator
    uint8_t writeBufferSelect = 0;                    // switch output buffers
};

// class for interchip communication in both directions
class COMinterChip {
  public:
    COMinterChip() {}

    // init as Output Com
    void initOutTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaTransmitFunc, uint8_t *dmaBuffer,
                             uint8_t layer);

    // init as Input Com
    void initInTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaReceiveFunc,
                            std::function<uint8_t()> dmaStopReceiveFunc, uint8_t *dmaBuffer);

    // Append to out buffer
    uint8_t sendCreatePatchInOut(uint8_t outputId, uint8_t inputId, float amount = 0);
    uint8_t sendUpdatePatchInOut(uint8_t outputId, uint8_t inputId, float amount);
    uint8_t sendDeletePatchInOut(uint8_t outputId, uint8_t inputId);
    uint8_t sendCreatePatchOutOut(uint8_t outputOutId, uint8_t OutputInId, float amount = 0, float offset = 0);
    uint8_t sendUpdatePatchOutOut(uint8_t outputOutId, uint8_t OutputInId, float amount = 0, float offset = 0);
    uint8_t sendDeletePatchOutOut(uint8_t outputOutId, uint8_t OutputInId);
    uint8_t sendDeleteAllPatches();
    uint8_t sendSetting(uint8_t modulID, uint8_t settingID, uint8_t *amount);
    uint8_t sendNewNote(uint8_t modulID, uint8_t voiceID, uint8_t settingID, int32_t amount);
    uint8_t sendOpenGate(uint8_t voiceID);
    uint8_t sendCloseGate(uint8_t voiceID);
    uint8_t sendRetrigger(uint8_t modulID, uint8_t voiceID);
    uint8_t sendResetAll();

    // send out current out buffer, used by master chip
    uint8_t beginSendTransmission();

    // start DMA reception, user by render chip
    uint8_t beginReceiveTransmission();

  private:
    // decode received Buffer
    uint8_t decodeCurrentInBuffer();

    // after first MDMA finished, start DMA transfer to render chip A
    uint8_t startFirstDMA();

    // after first DMA to render chip A finished, start copying data for render chip B via MDMA
    uint8_t startSecondMDMA();

    // after second MDMA finished, start DMA transfer to render chip B
    uint8_t startSecondDMA();

    // second DMA was successfull
    uint8_t sendTransmissionSuccessfull();

    // received new Data, copy from DMA buffer to RAM
    uint8_t copyReceivedInBuffer();

    // wrap up send buffers with closing bytes
    uint8_t appendLastByte();

    // start new send buffers with a 2-byte placeholder for size
    void pushDummySizePlaceHolder();

    // setup MDMA for DMA to RAM transfers and vice versa
    void prepareMDMAHandle();

    // buffer full, send now
    uint8_t invokeBufferFullSend();

    std::function<uint8_t(uint8_t *, uint16_t)> sendViaDMA;
    std::function<uint8_t(uint8_t *, uint16_t)> receiveViaDMA;
    std::function<uint8_t()> stopReceiveViaDMA;

    uint8_t *dmaInBuffer[2];
    uint8_t *dmaOutBuffer[2];

    uint16_t dmaOutCurrentBufferASize;
    uint16_t dmaOutCurrentBufferBSize;

    // flag to block new send commands while send task is in progress
    uint8_t blockNewSendBeginCommand = 0;

    uint8_t pushOutBufferChipA(uint8_t data);
    uint8_t pushOutBufferChipA(uint8_t *data, uint32_t size);
    uint8_t pushOutBufferChipB(uint8_t data);
    uint8_t pushOutBufferChipB(uint8_t *data, uint32_t size);

    std::vector<uint8_t> inBuffer[2];
    std::vector<uint8_t> outBufferChipA[2];
    std::vector<uint8_t> outBufferChipB[2];

    uint8_t currentBufferSelect = 0;

    uint8_t layer;

    void switchBuffer();
};
