#include "modules.hpp"

void ADSR::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void Midi::render() {
    oMod.currentSample[0] = aMod.valueMapped;
    oMod.currentSample[1] = aMod.valueMapped;
    oMod.currentSample[2] = aMod.valueMapped;
    oMod.currentSample[3] = aMod.valueMapped;
    oAftertouch.currentSample[0] = aAftertouch.valueMapped;
    oAftertouch.currentSample[1] = aAftertouch.valueMapped;
    oAftertouch.currentSample[2] = aAftertouch.valueMapped;
    oAftertouch.currentSample[3] = aAftertouch.valueMapped;
    oPitchbend.currentSample[0] = aPitchbend.valueMapped;
    oPitchbend.currentSample[1] = aPitchbend.valueMapped;
    oPitchbend.currentSample[2] = aPitchbend.valueMapped;
    oPitchbend.currentSample[3] = aPitchbend.valueMapped;
    oVeloctiy.currentSample[0] = aVelocity.valueMapped;
    oVeloctiy.currentSample[1] = aVelocity.valueMapped;
    oVeloctiy.currentSample[2] = aVelocity.valueMapped;
    oVeloctiy.currentSample[3] = aVelocity.valueMapped;
}
