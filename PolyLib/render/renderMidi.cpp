#include "renderMidi.hpp"

extern Layer layerA;

void renderMidi() {
    layerA.midi.oMod = layerA.midi.aMod.valueMapped;
    layerA.midi.oAftertouch = layerA.midi.aAftertouch.valueMapped;
    layerA.midi.oPitchbend = layerA.midi.aPitchbend.valueMapped;
    layerA.midi.oNote = ((vec<VOICESPERCHIP, float>)layerA.midi.rawNote) / 127.0f;
    layerA.midi.oVelocity = ((vec<VOICESPERCHIP, float>)layerA.midi.rawVelocity) / 127.0f;
    layerA.midi.oGate = layerA.midi.rawGate;
}