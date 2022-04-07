#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "comCommands.h"
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "flagHandler/flagHandler.hpp"
#include "hardware/device.hpp"
#include "mdma.h"
#include "midiInterface/midi_Defs.h"

#include "midiInterface/midi_Namespace.h"

// #include "poly.hpp"
#include <functional>
#include <inttypes.h>
#include <vector>

#ifdef POLYCONTROL
#define INPUTBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
#endif

#define INTERCHIPBUFFERSIZE 512

#ifdef POLYCONTROL
//  global settings
#include "globalsettings/globalSettings.hpp"
extern GlobalSettings globalSettings;
#endif

#ifdef POLYCONTROL

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

#endif

// class for interchip communication in both directions

#ifdef POLYCONTROL

enum comInterchipState { COM_READY, COM_AWAITINGDATA, COM_DECODE, COM_ERROR };
#endif
#ifdef POLYRENDER
enum comInterchipState { COM_READY, COM_DECODE };
#endif

class COMinterChip {
#ifdef POLYCONTROL

  public:
    // init as Output Com
    // Append to out buffer
    uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount = 0);
    uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
    uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
    uint8_t sendDeleteAllPatches(uint8_t layerId);
    uint8_t sendNewNote(uint8_t layerId, uint8_t voiceID, uint8_t note, uint8_t velocity);
    uint8_t sendOpenGate(uint8_t layerId, uint8_t voiceID);
    uint8_t sendCloseGate(uint8_t layerId, uint8_t voiceID);
    uint8_t sendRetrigger(uint8_t layerId, uint8_t modulID, uint8_t voiceID);
    uint8_t sendResetAll(uint8_t layerId);
    uint8_t sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, int32_t amount);
    uint8_t sendSetting(uint8_t layerId, uint8_t modulID, uint8_t settingID, float amount);
    uint8_t sendRequestUIData();
    busState beginReceiveTransmission(uint8_t layer, uint8_t chip);
    volatile bool sentRequestUICommand = false;
    volatile bool requestSize = false;
    volatile uint8_t receiveLayer;
    volatile uint8_t receiveChip;

  private:
#elif POLYRENDER
  public:
    uint8_t sendString(std::string &message);
    uint8_t sendString(const char *message);
    uint8_t sendOutput(uint8_t modulID, uint8_t settingID, int32_t amount);
    uint8_t sendOutput(uint8_t modulID, uint8_t settingID, float amount);
    uint8_t sendInput(uint8_t modulID, uint8_t settingID, float amount);

    busState beginReceiveTransmission();

  private:
    bool sendUpdateToRender = false;

#endif

  public:
    COMinterChip(spiBus *spi, uint8_t *dmaInBuffer, uint8_t *dmaOutBuffer) {
        inBufferPointer[0] = inBuffer;
        inBufferPointer[1] = inBuffer + INTERCHIPBUFFERSIZE + 4;

        outBuffer[0].reserve(INTERCHIPBUFFERSIZE + 4);
        outBuffer[1].reserve(INTERCHIPBUFFERSIZE + 4);

        messagebuffer.reserve(INTERCHIPBUFFERSIZE + 4);

        dmaInBufferPointer[0] = dmaInBuffer;
        dmaInBufferPointer[1] = dmaInBuffer + INTERCHIPBUFFERSIZE + 4;

        dmaOutBufferPointer[0] = dmaOutBuffer;
        dmaOutBufferPointer[1] = dmaOutBuffer + INTERCHIPBUFFERSIZE + 4;

        this->spi = spi;

        pushDummySizePlaceHolder();

        state = COM_READY;
    }

    void resetCom();

    // send out current out buffer
    busState beginSendTransmission();

    uint8_t decodeCurrentInBuffer();

    spiBus *spi;
    volatile comInterchipState state;

  private:
    std::string messagebuffer;

    // after first MDMA finished, start DMA transfer to render chip A
    busState startSendDMA();

    // wrap up send buffers with closing bytes
    uint8_t appendLastByte();

    // start new send buffers with a 2-byte placeholder for size
    void pushDummySizePlaceHolder();

    // push data into send out buffer
    uint8_t pushOutBuffer(uint8_t *data, uint32_t size);
    uint8_t pushOutBuffer(uint8_t data);

    // buffer full, send now
    busState invokeBufferFullSend();

    // flag to block new send commands while send task is in progress
    // uint8_t blockNewSendBeginCommand = 0;

    uint8_t *dmaOutBufferPointer[2];
    uint16_t dmaOutCurrentBufferSize;
    std::vector<uint8_t> outBuffer[2];

    uint8_t *inBufferPointer[2];
    uint8_t *dmaInBufferPointer[2];
    uint8_t inBuffer[(INTERCHIPBUFFERSIZE + 4) * 2];

    volatile uint8_t currentInBufferSelect = 0;
    volatile uint8_t currentOutBufferSelect = 0;

    void switchInBuffer();
    void switchOutBuffer();
};
