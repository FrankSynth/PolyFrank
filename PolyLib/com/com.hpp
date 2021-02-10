#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "comCommands.h"
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "flagHandler/flagHandler.hpp"
#include "mdma.h"
#include "midiInterface/midi_Defs.h"

#include "midiInterface/midi_Namespace.h"

// #include "poly.hpp"
#include "spi.h"
#include <functional>
#include <inttypes.h>
#include <vector>

#define INPUTBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
// TODO smaller buffersize test
#define INTERCHIPBUFFERSIZE 128

#define ERRORCODE_SENDBLOCK 20
#define ERRORCODE_RECEPTORNOTREADY 21

// callback for MDMA operations
// void comMDMACallback(MDMA_HandleTypeDef *_hmdma);

//  global settings
#ifdef POLYCONTROL
#include "globalsettings/globalSettings.hpp"
extern GlobalSettings globalSettings;
#endif

namespace midiUSB {

typedef struct {
    struct {
        uint8_t header;
        uint8_t byte1;
        uint8_t byte2;
        uint8_t byte3;
    };
} midiEventPacket_t;

/*! Enumeration of MIDI types */
enum MidiType : uint8_t {
    InvalidType = 0x00,                     ///< For notifying errors
    NoteOff = 0x80,                         ///< Channel Message - Note Off
    NoteOn = 0x90,                          ///< Channel Message - Note On
    AfterTouchPoly = 0xA0,                  ///< Channel Message - Polyphonic AfterTouch
    ControlChange = 0xB0,                   ///< Channel Message - Control Change / Channel Mode
    ProgramChange = 0xC0,                   ///< Channel Message - Program Change
    AfterTouchChannel = 0xD0,               ///< Channel Message - Channel (monophonic) AfterTouch
    PitchBend = 0xE0,                       ///< Channel Message - Pitch Bend
    SystemExclusive = 0xF0,                 ///< System Exclusive
    SystemExclusiveStart = SystemExclusive, ///< System Exclusive Start
    TimeCodeQuarterFrame = 0xF1,            ///< System Common - MIDI Time Code Quarter Frame
    SongPosition = 0xF2,                    ///< System Common - Song Position Pointer
    SongSelect = 0xF3,                      ///< System Common - Song Select
    Undefined_F4 = 0xF4,
    Undefined_F5 = 0xF5,
    TuneRequest = 0xF6,        ///< System Common - Tune Request
    SystemExclusiveEnd = 0xF7, ///< System Exclusive End
    Clock = 0xF8,              ///< System Real Time - Timing Clock
    Undefined_F9 = 0xF9,
    Tick = Undefined_F9, ///< System Real Time - Timing Tick (1 tick = 10 milliseconds)
    Start = 0xFA,        ///< System Real Time - Start
    Continue = 0xFB,     ///< System Real Time - Continue
    Stop = 0xFC,         ///< System Real Time - Stop
    Undefined_FD = 0xFD,
    ActiveSensing = 0xFE, ///< System Real Time - Active Sensing
    SystemReset = 0xFF,   ///< System Real Time - System Reset
};

// static uint8_t type2cin[][2] = {
//     {MidiType::InvalidType, 0},         {MidiType::NoteOff, 8},         {MidiType::NoteOn, 9},
//     {MidiType::AfterTouchPoly, 0xA},    {MidiType::ControlChange, 0xB}, {MidiType::ProgramChange, 0xC},
//     {MidiType::AfterTouchChannel, 0xD}, {MidiType::PitchBend, 0xE}};

// static uint8_t system2cin[][2] = {{MidiType::SystemExclusive, 0},
//                                   {MidiType::TimeCodeQuarterFrame, 2},
//                                   {MidiType::SongPosition, 3},
//                                   {MidiType::SongSelect, 2},
//                                   {0, 0},
//                                   {0, 0},
//                                   {MidiType::TuneRequest, 5},
//                                   {MidiType::SystemExclusiveEnd, 0},
//                                   {MidiType::Clock, 0xF},
//                                   {0, 0},
//                                   {MidiType::Start, 0xF},
//                                   {MidiType::Continue, 0xF},
//                                   {MidiType::Stop, 0xF},
//                                   {0, 0},
//                                   {MidiType::ActiveSensing, 0xF},
//                                   {MidiType::SystemReset, 0xF}};

static int8_t cin2Len[][2] = {{0, 0}, {1, 0}, {2, 2},  {3, 3},  {4, 0},  {5, 0},  {6, 0},  {7, 0},
                              {8, 3}, {9, 3}, {10, 3}, {11, 3}, {12, 2}, {13, 2}, {14, 3}, {15, 1}};

#define GETCABLENUMBER(packet) (packet.header >> 4);
#define GETCIN(packet) (packet.header & 0x0f);
#define MAKEHEADER(cn, cin) (((cn & 0x0f) << 4) | cin)
#define RXBUFFER_PUSHBACK1                                                                                             \
    { mRxBuffer[mRxLength++] = mPacket.byte1; }
#define RXBUFFER_PUSHBACK2                                                                                             \
    {                                                                                                                  \
        mRxBuffer[mRxLength++] = mPacket.byte1;                                                                        \
        mRxBuffer[mRxLength++] = mPacket.byte2;                                                                        \
    }
#define RXBUFFER_PUSHBACK3                                                                                             \
    {                                                                                                                  \
        mRxBuffer[mRxLength++] = mPacket.byte1;                                                                        \
        mRxBuffer[mRxLength++] = mPacket.byte2;                                                                        \
        mRxBuffer[mRxLength++] = mPacket.byte3;                                                                        \
    }

#define RXBUFFER_POPFRONT(dataByte)                                                                                    \
    auto dataByte = mRxBuffer[mRxIndex++];                                                                             \
    mRxLength--;
#define SENDMIDI(packet)                                                                                               \
    {                                                                                                                  \
        MidiUSB.sendMIDI(packet);                                                                                      \
        MidiUSB.flush();                                                                                               \
    }

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

    uint8_t read() {
        RXBUFFER_POPFRONT(dataByte);
        return dataByte;
    };

    midiEventPacket_t readPacket() {

        midiEventPacket_t data;
        if (rBuffer.empty()) {
            data.header = 0;
        }
        else {
            data.header = rBuffer.front();
            rBuffer.pop_front();
            data.byte1 = rBuffer.front();
            rBuffer.pop_front();
            data.byte2 = rBuffer.front();
            rBuffer.pop_front();
            data.byte3 = rBuffer.front();
            rBuffer.pop_front();
        }

        return data;
    }

    uint8_t available() {
        // consume mRxBuffer first, before getting a new packet
        if (mRxLength > 0) {
            return mRxLength;
        }
        mRxIndex = 0;

        mPacket = readPacket();
        if (mPacket.header != 0) {
            auto cn = GETCABLENUMBER(mPacket);
            if (cn != cableNumber)
                return 0;

            auto cin = GETCIN(mPacket);
            auto len = cin2Len[cin][1];

            switch (len) {
                case 0:
                    if (cin == 0x4 || cin == 0x7)
                        RXBUFFER_PUSHBACK3
                    else if (cin == 0x5)
                        RXBUFFER_PUSHBACK1
                    else if (cin == 0x6)
                        RXBUFFER_PUSHBACK2
                    break;
                case 1: RXBUFFER_PUSHBACK1 break;
                case 2: RXBUFFER_PUSHBACK2 break;
                case 3: RXBUFFER_PUSHBACK3 break;
                default: break; // error
            }
        }

        return mRxLength;
    }

  private:
    CircularBuffer<uint8_t, INPUTBUFFERSIZE> rBuffer; // read buffer
    std::vector<uint8_t> wBuffer[2];                  // write buffer, size of BUFFERSIZE gets reserved in allocator
    uint8_t writeBufferSelect = 0;                    // switch output buffers

    uint8_t mRxBuffer[4];
    uint8_t mRxLength;
    uint8_t mRxIndex;

    midiEventPacket_t mPacket;
    uint8_t cableNumber;
};
} // namespace midiUSB

class COMdin {
  public:
    COMdin(uint8_t (*sendViaUSB)(uint8_t *, uint16_t) = nullptr) {
        this->sendViaUSB = sendViaUSB;
        wBuffer[0].reserve(OUTPUTBUFFERSIZE);
        wBuffer[1].reserve(OUTPUTBUFFERSIZE);
    }
    ~COMdin() {}

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
#ifdef POLYCONTROL
    // init as Output Com
    void initOutTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaTransmitFunc, uint8_t *dmaBuffer,
                             uint8_t layer);
    // Append to out buffer
    uint8_t sendCreatePatchInOut(uint8_t outputId, uint8_t inputId, float amount = 0);
    uint8_t sendUpdatePatchInOut(uint8_t outputId, uint8_t inputId, float amount);
    uint8_t sendDeletePatchInOut(uint8_t outputId, uint8_t inputId);
    uint8_t sendCreatePatchOutOut(uint8_t outputOutId, uint8_t OutputInId, float amount = 0, float offset = 0);
    uint8_t sendUpdatePatchOutOut(uint8_t outputOutId, uint8_t OutputInId, float amount = 0, float offset = 0);
    uint8_t sendDeletePatchOutOut(uint8_t outputOutId, uint8_t OutputInId);
    uint8_t sendDeleteAllPatches();
    uint8_t sendSetting(uint8_t modulID, uint8_t settingID, int32_t amount);
    uint8_t sendSetting(uint8_t modulID, uint8_t settingID, float amount);
    uint8_t sendNewNote(uint8_t voiceID, uint8_t note, uint8_t velocity);
    uint8_t sendOpenGate(uint8_t voiceID);
    uint8_t sendCloseGate(uint8_t voiceID);
    uint8_t sendRetrigger(uint8_t modulID, uint8_t voiceID);
    uint8_t sendResetAll();

    // after first MDMA finished, start DMA transfer to render chip A
    uint8_t startFirstDMA();

    // after first DMA to render chip A finished, start copying data for render chip B via MDMA
    uint8_t startSecondMDMA();

    // after second MDMA finished, start DMA transfer to render chip B
    uint8_t startSecondDMA();

    // second DMA was successfull
    uint8_t sendTransmissionSuccessfull();

    // send out current out buffer, used by master chip
    uint8_t beginSendTransmission();

#elif POLYRENDER
    // init as Input Com
    void initInTransmission(std::function<uint8_t(uint8_t *, uint16_t)> dmaReceiveFunc,
                            std::function<uint8_t()> dmaStopReceiveFunc, uint8_t *dmaBuffer);

    // start DMA reception, user by render chip
    uint8_t beginReceiveTransmission();
#endif

  private:
#ifdef POLYCONTROL

    // wrap up send buffers with closing bytes
    uint8_t appendLastByte();

    // start new send buffers with a 2-byte placeholder for size
    void pushDummySizePlaceHolder();

    // buffer full, send now
    uint8_t invokeBufferFullSend();

    std::function<uint8_t(uint8_t *, uint16_t)> sendViaDMA;

    uint8_t *dmaOutBufferPointer[2];

    uint16_t dmaOutCurrentBufferASize;
    uint16_t dmaOutCurrentBufferBSize;

    // flag to block new send commands while send task is in progress
    uint8_t blockNewSendBeginCommand = 0;

    uint8_t pushOutBufferChipA(uint8_t data);
    uint8_t pushOutBufferChipA(uint8_t *data, uint32_t size);
    uint8_t pushOutBufferChipB(uint8_t data);
    uint8_t pushOutBufferChipB(uint8_t *data, uint32_t size);

    // TODO optimize speed with non-vector buffer
    std::vector<uint8_t> outBufferChipA[2];
    std::vector<uint8_t> outBufferChipB[2];

#elif POLYRENDER

    // decode received Buffer
    uint8_t decodeCurrentInBuffer();

    // received new Data, copy from DMA buffer to RAM
    uint8_t copyReceivedInBuffer();

    std::function<uint8_t(uint8_t *, uint16_t)> receiveViaDMA;
    std::function<uint8_t()> stopReceiveViaDMA;
    uint8_t *dmaInBufferPointer[2];
    ALIGN_32BYTES(uint8_t inBuffer[INTERCHIPBUFFERSIZE * 2]);
    uint8_t *inBufferPointer[2];

#endif

    // setup MDMA for DMA to RAM transfers and vice versa
    void prepareMDMAHandle();

    uint8_t currentBufferSelect = 0;

    uint8_t layer;

    void switchBuffer();
};
