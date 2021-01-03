#include "modules.hpp"

// NameLists for switches
const std::vector<std::string> nlOnOff{"OFF", "ON"};
const std::vector<std::string> nlVCFDest{"STEINER", "LADDER", "BOTH", "OFF"};
const std::vector<std::string> nlSteinerModes{"LP", "HP", "BP", "AP"};
const std::vector<std::string> nlLadderSlopes{"6", "12", "18", "24"};

// Render Functions
void BaseModule::render() {}

void Midi::render() {
    oMod.nextSample[0] = aMod.valueMapped;
    oMod.nextSample[1] = aMod.valueMapped;
    oMod.nextSample[2] = aMod.valueMapped;
    oMod.nextSample[3] = aMod.valueMapped;
    oAftertouch.nextSample[0] = aAftertouch.valueMapped;
    oAftertouch.nextSample[1] = aAftertouch.valueMapped;
    oAftertouch.nextSample[2] = aAftertouch.valueMapped;
    oAftertouch.nextSample[3] = aAftertouch.valueMapped;
    oPitchbend.nextSample[0] = aPitchbend.valueMapped;
    oPitchbend.nextSample[1] = aPitchbend.valueMapped;
    oPitchbend.nextSample[2] = aPitchbend.valueMapped;
    oPitchbend.nextSample[3] = aPitchbend.valueMapped;
    oVeloctiy.nextSample[0] = aVelocity.valueMapped;
    oVeloctiy.nextSample[1] = aVelocity.valueMapped;
    oVeloctiy.nextSample[2] = aVelocity.valueMapped;
    oVeloctiy.nextSample[3] = aVelocity.valueMapped;
}

void OSC_A::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void OSC_B::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void Sub::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void Noise::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void Steiner::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void Ladder::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void Distortion::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void LFO::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void ADSR::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void GlobalModule::render() {
    for (Input *input : inputs) {
        input->collectCurrentSample();
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}