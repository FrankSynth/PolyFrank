#pragma once

// first byte
#define CMD_LAYERMASK 0b10000000
#define CMD_MODULEMASK 0b01111000
#define CMD_VOICEMASK 0b00000111

#define CREATEINOUTPATCHCMDSIZE 8
#define UPDATEINOUTPATCHCMDSIZE 8
#define DELETEPATCHCMDSIZE 4
#define DELETEALLPATCHESCMDSIZE 2
#define SETTINGCMDSIZE 8
#define INPUTCMDSIZE 6
#define RENDERBUFFERCMDSIZE 19
#define UPDATEAUDIOBUFFERSIZE 301
#define OUTPUTCMDSIZE 19
#define GATECMDSIZE 2
#define NEWNOTECMDSIZE 4
#define RETRIGGERCMDSIZE 3
#define LASTBYTECMDSIZE 1
#define RESETALLCMDSIZE 2
#define SENDUPDATETOCONTROLSIZE 2
#define MESSAGECMDSIZE 1
#define BPMCMDSIZE 5

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

// Patches
// LCCCCVVV | COMMAND | IIIIIIII | OOOOOOOO | 32Bit Float (Amount)

// Settings
// LCCCCVVV | COMMAND | Setting | 32Bit Float/Int (Amount)

// first two bytes of each buffer have to determine the amount of bytes

// second byte all commands
enum comCommands {
    NOCOMMAND = 0,
    UPDATEINOUTPATCH,   // update patchesInOut
    CREATEINOUTPATCH,   // create patchesInOut
    DELETEINOUTPATCH,   // delete patchesInOut
    DELETEALLPATCHES,   // delete patchesInOut
    NEWNOTE,            // received new Note
    OPENGATE,           // received NoteOn
    CLOSEGATE,          // received NoteOff
    RESETALL,           // received Reset All
    RETRIGGER,          // retrigger
    UPDATESETTINGINT,   // next bytes are setting
    UPDATESETTINGFLOAT, // next bytes are settings
    // UPDATEINPUT,             // next bytes are settings
    UPDATEOUTPUTFLOAT, // next bytes are settings
    // UPDATEOUTPUTINT,         // next bytes are settings
    SENDUPDATETOCONTROL, // pull req from control
    SENDMESSAGE,         // send a string
    UPDATERENDERBUFFER,  // next bytes are settings
    // UPDATERENDERBUFFERVOICE, // next bytes are settings
    UPDATEAUDIOBUFFER,     // next bytes are settings
    BPM,                   // next bytes are settings
    LASTBYTE = 0b01111111, // LAST BYTE of transmission
};
