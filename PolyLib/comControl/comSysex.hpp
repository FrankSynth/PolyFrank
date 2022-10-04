#include "com/com.hpp"
#include "datacore/datalocation.hpp"
#include "midiInterface/MIDIInterface.h"

#include <stdio.h>

extern midi::MidiInterface<midiUSB::COMusb> midiDeviceUSB;

// #define SYS_RECEIVE_PRESET 0x01
// #define SYS_RECEIVE_PRESETTABLE 0x02

#define SYS_SEND_PRESET 0x10
#define SYS_SEND_PRESETTABLE 0x20

// Encode and Decode of Sysex messages
#define MAXMESSAGESIZE 4096 //  ->512bytes /8*9

#define SYSEXPACKETSIZE MAXMESSAGESIZE * 2 //

RAM1 uint8_t sysexMessageBuffer[SYSEXPACKETSIZE];

void sendSysexCommand(uint8_t command) {
    uint32_t dataLength = midi::encodeSysEx(&command, sysexMessageBuffer, 1);
    midiDeviceUSB.sendSysEx(dataLength, sysexMessageBuffer);
}

void sendSysexCommand(uint8_t commandA, uint8_t commandB) {
    uint32_t dataLength = midi::encodeSysEx(&commandA, sysexMessageBuffer, 1);
    dataLength += midi::encodeSysEx(&commandB, sysexMessageBuffer + dataLength, 1);

    midiDeviceUSB.sendSysEx(dataLength, sysexMessageBuffer);
}

void sendSysecData(uint8_t *data, uint32_t length) {
    uint32_t dataLength = midi::encodeSysEx(data, sysexMessageBuffer, length);
    midiDeviceUSB.sendSysEx(dataLength, sysexMessageBuffer);
}

void sendPresetTable() {
    // send sysex command
    sendSysexCommand(SYS_SEND_PRESETTABLE);

    // read presetTable from EEPROM
    updatePresetList();

    // send sysex data
    sendSysecData(blockBuffer, TABLE_BLOCKSIZE);
}

void sendPreset(uint32_t presetID) {
    // send sysex command
    sendSysexCommand(SYS_SEND_PRESET, presetID);
    readPreset(presetsSorted[presetID]);
    // send sysex data
    sendSysecData(blockBuffer, PRESET_BLOCKSIZE);
}
