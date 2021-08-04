#pragma once

#define CMD_TYPEMASK 0b10000000
#define CMD_PATCHMASK 0b01111000
#define CMD_VOICEMASK 0b00000111
#define CMD_MODULEMASK 0b01111000
#define CMD_SETTINGSCMDMASK 0b11000000
#define CMD_SETTINGSMASK 0b00111111

#define CREATEINOUTPATCHCMDSIZE 7
#define UPDATEINOUTPATCHCMDSIZE 7
#define DELETEPATCHCMDSIZE 3
#define DELETEALLPATCHESCMDSIZE 1
#define SETTINGCMDSIZE 6
#define GATECMDSIZE 1
#define NEWNOTECMDSIZE 3
#define RETRIGGERCMDSIZE 2
#define LASTBYTECMDSIZE 1
#define RESETALLCMDSIZE 1

#define VOICE0 0
#define VOICE1 1
#define VOICE2 2
#define VOICE3 3
#define VOICE4 4
#define VOICE5 5
#define VOICE6 6
#define VOICE7 7
#define VOICEALL 8

#define NOVOICE 255

// first two bytes of each buffer have to determine the amount of bytes

// first byte all commands
enum comCommands {
    // Patches
    // 1CCCCVVV | IIIIIIII | OOOOOOOO | 32Bit Float (Amount)
    PATCHCMDTYPE = 0b10000000, // next bytes are setting

    UPDATEINOUTPATCH = 0b00000000, // update patchesInOut
    CREATEINOUTPATCH = 0b00001000, // create patchesInOut
    DELETEINOUTPATCH = 0b00010000, // delete patchesInOut
    DELETEALLPATCHES = 0b00110000, // delete patchesInOut
    LASTBYTE = 0b01111000,         // LAST BYTE of transmission

    // Trigger
    // 1CCCCVVV
    NEWNOTE = 0b01100000,   // received new Note
    OPENGATE = 0b01000000,  // received NoteOn
    CLOSEGATE = 0b01001000, // received NoteOff
    CLOCK = 0b01010000,     // received NoteOff
    RESETALL = 0b01011000,  // received NoteOff

    // Setting
    // 0MMMMVVV| CCSSSSSS | 0 or 32Bit Data
    SETTINGTYPE = 0b00000000, // next bytes are setting

};

// 2nd byte settings

enum settingCommands {
    // Setting
    // 0MMMMVVV| CCSSSSSS | 0 or 32Bit Data
    UPDATESETTINGINT = 0b00000000,   // 4 Bytes Following
    UPDATESETTINGFLOAT = 0b01000000, // 4 Bytes Following
    RETRIGGER = 0b10000000,          // 0 Bytes Following
};