#include "renderMidi.hpp"

void renderMidi(Midi midi) {
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oMod.nextSample[voice] = midi.aMod.valueMapped;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oAftertouch.nextSample[voice] = midi.aAftertouch.valueMapped;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oPitchbend.nextSample[voice] = midi.aPitchbend.valueMapped;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oNote.nextSample[voice] = (float)midi.rawNote[voice] / 127.0f;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oVeloctiy.nextSample[voice] = (float)midi.rawVelocity[voice] / 127.0f;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oGate.nextSample[voice] = midi.rawGate[voice];
}