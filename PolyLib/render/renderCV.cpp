
#ifdef POLYRENDER

#include "renderCV.hpp"

extern MCP4728 cvDacA;
extern MCP4728 cvDacB;
extern MCP4728 cvDacC;

extern Layer layerA;

float secondsPerRender;
void initCVRendering() {
    secondsPerRender = (float)CVTIMERINTERVALUS / 1000000.0;
}

void collectAllCurrentInputs() {
    for (BaseModule *m : layerA.modules) {
        for (Input *i : m->inputs) {
            i->collectCurrentSample();
        }
    }
}

void updateAllOutputSamples() {
    for (BaseModule *m : layerA.modules) {
        for (Output *o : m->outputs) {
            o->updateToNextSample();
        }
    }
}

// Render Functions

void renderMidiModule(Midi midi) {

    midi.oMod.nextSample[0] = midi.aMod.valueMapped;
    midi.oMod.nextSample[1] = midi.aMod.valueMapped;
    midi.oMod.nextSample[2] = midi.aMod.valueMapped;
    midi.oMod.nextSample[3] = midi.aMod.valueMapped;
    midi.oAftertouch.nextSample[0] = midi.aAftertouch.valueMapped;
    midi.oAftertouch.nextSample[1] = midi.aAftertouch.valueMapped;
    midi.oAftertouch.nextSample[2] = midi.aAftertouch.valueMapped;
    midi.oAftertouch.nextSample[3] = midi.aAftertouch.valueMapped;
    midi.oPitchbend.nextSample[0] = midi.aPitchbend.valueMapped;
    midi.oPitchbend.nextSample[1] = midi.aPitchbend.valueMapped;
    midi.oPitchbend.nextSample[2] = midi.aPitchbend.valueMapped;
    midi.oPitchbend.nextSample[3] = midi.aPitchbend.valueMapped;
    midi.oVeloctiy.nextSample[0] = midi.aVelocity.valueMapped;
    midi.oVeloctiy.nextSample[1] = midi.aVelocity.valueMapped;
    midi.oVeloctiy.nextSample[2] = midi.aVelocity.valueMapped;
    midi.oVeloctiy.nextSample[3] = midi.aVelocity.valueMapped;
    midi.oGate.nextSample[0] = midi.dGate.valueMapped;
    midi.oGate.nextSample[1] = midi.dGate.valueMapped;
    midi.oGate.nextSample[2] = midi.dGate.valueMapped;
    midi.oGate.nextSample[3] = midi.dGate.valueMapped;
}

void renderOSC_A(OSC_A &osc_A) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderOSC_B(OSC_B &osc_B) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderSub(Sub &sub) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderNoise(Noise &noise) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderSteiner(Steiner &steiner) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderLadder(Ladder &ladder) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderDistortion(Distortion &distort) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderLFO(LFO &lfo) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderADSR(ADSR &adsr) {

    // TODO render not time based, but pitch

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        switch (adsr.currentState[voice]) {
            case adsr.OFF:
                if (layerA.midi.oGate.currentSample[voice] == 1 || adsr.dLoop.valueMapped == 1)
                    adsr.currentState[voice] = adsr.DELAY;
                break;

            case adsr.DELAY:
                if (layerA.midi.oGate.currentSample[voice] == 0 && adsr.dLoop.valueMapped == 0)
                    adsr.currentState[voice] = adsr.OFF;

                float delay = adsr.iDelay.currentSample[voice] * adsr.aDelay.minMaxDifference + adsr.aDelay.valueMapped;

                adsr.currentTime[voice] += secondsPerRender;
                adsr.out.nextSample[voice] = 0;

                if (adsr.currentTime[voice] >= delay) {
                    adsr.currentState[voice] = adsr.ATTACK;
                    adsr.currentTime[voice] = 0;
                }

                break;

            case adsr.ATTACK:
                float attack =
                    adsr.iAttack.currentSample[voice] * adsr.aAttack.minMaxDifference + adsr.aAttack.valueMapped;

                adsr.currentTime[voice] += secondsPerRender;

                adsr.out.nextSample[voice] = testFloat(adsr.currentTime[voice] / attack, 0, 1);

                if (adsr.dLoop.valueMapped) {
                    if (adsr.nextSample[voice] >= 1) {
                        adsr.currentState[voice] = adsr.RELEASE;
                        adsr.currentTime[voice] = 0;
                    }
                }
                else {
                    if (layerA.midi.oGate.currentSample[voice] == 0) {
                        adsr.currentState[voice] = adsr.RELEASE;
                        adsr.currentTime[voice] = 0;
                    }
                    else if (adsr.currentTime[voice] >= attack) {
                        adsr.currentState[voice] = adsr.DECAY;
                        adsr.currentTime[voice] = 0;
                    }
                }

                break;
            case adsr.DECAY:
                float decay = adsr.iDecay.currentSample[voice] * adsr.aDecay.minMaxDifference + adsr.aDecay.valueMapped;
                float sustain =
                    adsr.iSustain.currentSample[voice] * adsr.aSustain.minMaxDifference + adsr.aSustain.valueMapped;

                adsr.currentTime[voice] += secondsPerRender;

                if (adsr.currentTime[voice] >= attack) {
                    adsr.currentState[voice] = adsr.SUSTAIN;
                    adsr.currentTime[voice] = 0;
                }

                adsr.out.nextSample[voice] = testFloat(1 - (adsr.currentTime[voice] / decay) * (1 - sustain), 0, 1);
                break;
            case adsr.SUSTAIN:
                float sustain =
                    adsr.iSustain.currentSample[voice] * adsr.aSustain.minMaxDifference + adsr.aSustain.valueMapped;

                if (layerA.midi.oGate.currentSample[voice] == 0)
                    adsr.currentState[voice] = adsr.RELEASE;
                adsr.out.nextSample[voice] = sustain;
                break;

            case adsr.RELEASE: break;
            default: Error_Handler(); break;
        }

        adsr.currentLevel[voice] = fast_lerp_f32(adsr.currentLevel[voice],
                                                 adsr.currentLevel[voice] * layerA.midi.oVeloctiy.currentSample[voice],
                                                 adsr.aVelocity.valueMapped);

        // TODO keytrack MISSING

        adsr.out.nextSample[voice] = adsr.currentLevel[voice] * adsr.aAmount.valueMapped;
    }
}

void renderGlobalModule(GlobalModule globalModule) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

// TODO remove this later
void tempRendering() {
    static float dacStep = 0;

    dacStep += layerA.test.aCutoff.valueMapped / (1000.0f / (float)CVTIMERINTERVALUS); // 100hz sinus

    cvDacB.data[3] = (fast_sin_f32(dacStep) + 1) * 2047; // 100hz sinus
    // cvDacB.data[3] = 4095; // const

    if (dacStep > 1) {
        dacStep -= 1;
    }

    cvDacA.data[0] = layerA.test.aCutoff.valueMapped * 4095;
    cvDacA.data[1] = (1 - layerA.test.aResonance.valueMapped) * 4095;
    cvDacA.data[2] = layerA.test.aFreq.valueMapped * 4095;
    cvDacA.data[3] = (1 - layerA.test.aDistort.valueMapped) * 4095;
}

void renderCVs() {

    collectAllCurrentInputs();

    renderMidiModule(layerA.midi);
    renderOSC_A(layerA.oscA);
    renderOSC_B(layerA.oscB);
    renderSub(layerA.sub);
    renderNoise(layerA.noise);
    renderSteiner(layerA.steiner);
    renderLadder(layerA.ladder);
    renderDistortion(layerA.distort);
    renderLFO(layerA.lfoA);
    renderLFO(layerA.lfoB);
    renderADSR(layerA.adsrA);
    renderADSR(layerA.adsrB);
    renderGlobalModule(layerA.globalModule);

    // TODO ? still true? need to render global stuff like Filter Out Levels here

    updateAllOutputSamples();

    // TODO copy all output.currentSamples to their DAC data storage here

    // TODO remove this later
    tempRendering();

    cvDacA.setLatchPin();
    cvDacB.setLatchPin();
    cvDacC.setLatchPin();

    cvDacA.fastUpdate();

    FlagHandler::cvDacAStarted = true;
    // cvDacB.fastUpdate();
    // cvDacC.fastUpdate();
}

#endif
