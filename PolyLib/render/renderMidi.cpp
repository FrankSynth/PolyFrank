#include "renderMidi.hpp"

void renderMidi(Midi &midi) {
    midi.oMod = midi.aMod.valueMapped;
    midi.oAftertouch = midi.aAftertouch.valueMapped;
    midi.oPitchbend = midi.aPitchbend.valueMapped;
    midi.oNote = (vec<VOICESPERCHIP, float>)midi.rawNote / 127.0f;
    midi.oVeloctiy = (vec<VOICESPERCHIP, float>)midi.rawVelocity / 127.0f;
    midi.oGate = midi.rawGate;
}