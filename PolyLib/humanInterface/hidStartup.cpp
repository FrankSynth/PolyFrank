#ifdef POLYCONTROL

#include "hardware/device.hpp"
#include "hid.hpp"
#include "layer/layer.hpp"
#include "math/polyMath.hpp"

extern IS32FL3237 ledDriver[2][2];
extern IS31FL3205 ledDriverControl;

extern std::vector<Layer *> allLayers;
extern MAX11128 adcA;
extern MAX11128 adcB;
extern TS3A5017D multiplexer;

extern rotary encoders[NUMBERENCODERS];
extern tactileSwitch switches[NUMBERENCODERS];

potiFunctionStruct potiFunctionPointer[2][4][12];
uint16_t panelADCStates[2][4][12];
presetGrabState panelGrab[2][4][12];
float panelADCInterpolate[2][4][12];

inline float distanceBasedSin(float distance) {
    static elapsedMillis timer = 0;
    return (fast_sin_fade_f32((float)timer / 1500.f - distance / LEDMAXDISTANCE) + 1.f) / 2.f;
}

void LEDIntroFrame() {

    static elapsedMillis fadeTimer = 0;
    static float fade = 0;

    for (uint32_t i = 0; i < 2; i++) { // for both Layer
        // clear Data
        for (uint32_t x = 0; x < 2; x++) {
            ledDriver[i][x].clearPWMData(); // delete old data
        }
    }

    if (fadeTimer < 500) {
        fade = (float)fadeTimer / 500.;
    }
    if (fadeTimer > 2750) {
        fade = 1 - (float)(-2750 + (int32_t)fadeTimer) / 500.;

        if (fade < 0)
            fade = 0;
    }

    LEDOutput(&allLayers[0]->midi.oNote, distanceBasedSin(8) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->midi.oVelocity, distanceBasedSin(8.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->midi.oMod, distanceBasedSin(9.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->midi.oPitchbend, distanceBasedSin(9) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->midi.oAftertouch, distanceBasedSin(9.5) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->oscA.iFM, distanceBasedSin(13.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->oscA.iMorph, distanceBasedSin(13) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->oscA.iEffect, distanceBasedSin(14) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->oscA.dOctaveSwitchSub, 0, distanceBasedSin(14) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->oscA.iFM, distanceBasedSin(21) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->oscA.iMorph, distanceBasedSin(24) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->oscA.iEffect, distanceBasedSin(26) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->oscA.dOctaveSwitchSub, 0, distanceBasedSin(29) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->oscB.iFM, distanceBasedSin(17) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->oscB.iMorph, distanceBasedSin(16) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->oscB.iEffect, distanceBasedSin(18) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->oscB.dSync, 0, distanceBasedSin(19) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->oscB.iFM, distanceBasedSin(23) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->oscB.iMorph, distanceBasedSin(26) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->oscB.iEffect, distanceBasedSin(28) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->oscB.dSync, 0, distanceBasedSin(30) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->feel.iGlide, distanceBasedSin(17) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->feel.iDetune, distanceBasedSin(20) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->feel.oSpread, distanceBasedSin(24) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->feel.iGlide, distanceBasedSin(31) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->feel.iDetune, distanceBasedSin(33) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[1]->feel.oSpread, distanceBasedSin(35) * fade * LEDBRIGHTNESS_SETTING);

    LEDDigital(&allLayers[0]->mixer.dOSCADestSwitch, 0, distanceBasedSin(23) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dOSCADestSwitch, 1, distanceBasedSin(23.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dOSCBDestSwitch, 0, distanceBasedSin(22) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dOSCBDestSwitch, 1, distanceBasedSin(22.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dSUBDestSwitch, 0, distanceBasedSin(24) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dSUBDestSwitch, 1, distanceBasedSin(24.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dNOISEDestSwitch, 0, distanceBasedSin(26) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->mixer.dNOISEDestSwitch, 1, distanceBasedSin(26.5) * fade * LEDBRIGHTNESS_SETTING);

    LEDDigital(&allLayers[1]->mixer.dOSCADestSwitch, 0, distanceBasedSin(28) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dOSCADestSwitch, 1, distanceBasedSin(28.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dOSCBDestSwitch, 0, distanceBasedSin(30) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dOSCBDestSwitch, 1, distanceBasedSin(30.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dSUBDestSwitch, 0, distanceBasedSin(31.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dSUBDestSwitch, 1, distanceBasedSin(32) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dNOISEDestSwitch, 0, distanceBasedSin(33.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->mixer.dNOISEDestSwitch, 1, distanceBasedSin(34) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->steiner.iCutoff, distanceBasedSin(27) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->steiner.iResonance, distanceBasedSin(27) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->steiner.iLevel, distanceBasedSin(28) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->steiner.dMode, 0, distanceBasedSin(28) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->steiner.dMode, 1, distanceBasedSin(28.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->steiner.dMode, 2, distanceBasedSin(29) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->steiner.dMode, 3, distanceBasedSin(29.5) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->steiner.iCutoff, distanceBasedSin(33) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->steiner.iResonance, distanceBasedSin(34.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->steiner.iLevel, distanceBasedSin(36) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->steiner.dMode, 0, distanceBasedSin(37) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->steiner.dMode, 1, distanceBasedSin(37.3) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->steiner.dMode, 2, distanceBasedSin(37.8) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->steiner.dMode, 3, distanceBasedSin(38.2) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->ladder.iCutoff, distanceBasedSin(31.3) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->ladder.iResonance, distanceBasedSin(31) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->ladder.iLevel, distanceBasedSin(31.7) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->ladder.dSlope, 0, distanceBasedSin(30.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->ladder.dSlope, 1, distanceBasedSin(31) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->ladder.dSlope, 2, distanceBasedSin(31.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->ladder.dSlope, 3, distanceBasedSin(32) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->ladder.iCutoff, distanceBasedSin(36) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->ladder.iResonance, distanceBasedSin(37) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->ladder.iLevel, distanceBasedSin(38) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->ladder.dSlope, 0, distanceBasedSin(39) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->ladder.dSlope, 1, distanceBasedSin(39.3) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->ladder.dSlope, 2, distanceBasedSin(39.8) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->ladder.dSlope, 3, distanceBasedSin(40) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->lfoA.iFreq, distanceBasedSin(35) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->lfoA.iShape, distanceBasedSin(35) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->lfoA.iAmount, distanceBasedSin(36) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->lfoA.dGateTrigger, 0, distanceBasedSin(33) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->lfoA.dAlignLFOs, 0, distanceBasedSin(37) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->lfoA.out, distanceBasedSin(36) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[0]->lfoA, distanceBasedSin(35.5) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->lfoA.iFreq, distanceBasedSin(38) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->lfoA.iShape, distanceBasedSin(39) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->lfoA.iAmount, distanceBasedSin(40) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->lfoA.dGateTrigger, 0, distanceBasedSin(39) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->lfoA.dAlignLFOs, 0, distanceBasedSin(41) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[1]->lfoA.out, distanceBasedSin(42) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[1]->lfoA, distanceBasedSin(37) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->lfoB.iFreq, distanceBasedSin(40.3) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->lfoB.iShape, distanceBasedSin(40) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->lfoB.iAmount, distanceBasedSin(41) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->lfoB.dGateTrigger, 0, distanceBasedSin(39) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[0]->lfoB.dAlignLFOs, 0, distanceBasedSin(42) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->lfoB.out, distanceBasedSin(41) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[0]->lfoB, distanceBasedSin(40.6) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->lfoB.iFreq, distanceBasedSin(45) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->lfoB.iShape, distanceBasedSin(47) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->lfoB.iAmount, distanceBasedSin(48) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->lfoB.dGateTrigger, 0, distanceBasedSin(47) * fade * LEDBRIGHTNESS_SETTING);
    LEDDigital(&allLayers[1]->lfoB.dAlignLFOs, 0, distanceBasedSin(50) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[1]->lfoB.out, distanceBasedSin(51) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[1]->lfoB, distanceBasedSin(44) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->envF.iAmount, distanceBasedSin(54) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->envF.out, distanceBasedSin(54) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[0]->envF, distanceBasedSin(42) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->envA.iAmount, distanceBasedSin(55) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[0]->envA.out, distanceBasedSin(55.5) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[0]->envA, distanceBasedSin(47) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->envF.iAmount, distanceBasedSin(56) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[1]->envF.out, distanceBasedSin(57) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[1]->envF, distanceBasedSin(47) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->envA.iAmount, distanceBasedSin(59) * fade * LEDBRIGHTNESS_SETTING);
    LEDOutput(&allLayers[1]->envA.out, distanceBasedSin(60) * fade * LEDBRIGHTNESS_SETTING);
    LEDModule(&allLayers[1]->envA, distanceBasedSin(53) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[0]->out.iPan, distanceBasedSin(48) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[0]->out.iVCA, distanceBasedSin(50) * fade * LEDBRIGHTNESS_SETTING);

    LEDInput(&allLayers[1]->out.iPan, distanceBasedSin(57) * fade * LEDBRIGHTNESS_SETTING);
    LEDInput(&allLayers[1]->out.iVCA, distanceBasedSin(60) * fade * LEDBRIGHTNESS_SETTING);
}

void LEDMappingInit() {
    for (uint8_t i = 0; i < 2; i++) {

        // println("INFO | LED Mapping ", i);

        /////// Inputs

        // Steiner
        allLayers[i]->steiner.iCutoff.LEDPinID = 0;
        allLayers[i]->steiner.iCutoff.LEDPortID = 0;

        allLayers[i]->steiner.iResonance.LEDPinID = 35;
        allLayers[i]->steiner.iResonance.LEDPortID = 0;

        allLayers[i]->steiner.iLevel.LEDPinID = 34;
        allLayers[i]->steiner.iLevel.LEDPortID = 0;

        // Ladder
        allLayers[i]->ladder.iCutoff.LEDPinID = 31;
        allLayers[i]->ladder.iCutoff.LEDPortID = 0;

        allLayers[i]->ladder.iResonance.LEDPinID = 32;
        allLayers[i]->ladder.iResonance.LEDPortID = 0;

        allLayers[i]->ladder.iLevel.LEDPinID = 33;
        allLayers[i]->ladder.iLevel.LEDPortID = 0;

        // Feel
        allLayers[i]->feel.iGlide.LEDPinID = 11;
        allLayers[i]->feel.iGlide.LEDPortID = 1;

        allLayers[i]->feel.iDetune.LEDPinID = 12;
        allLayers[i]->feel.iDetune.LEDPortID = 1;

        // OSC A
        allLayers[i]->oscA.iFM.LEDPinID = 16;
        allLayers[i]->oscA.iFM.LEDPortID = 0;

        allLayers[i]->oscA.iMorph.LEDPinID = 15;
        allLayers[i]->oscA.iMorph.LEDPortID = 0;

        allLayers[i]->oscA.iEffect.LEDPinID = 14;
        allLayers[i]->oscA.iEffect.LEDPortID = 0;

        // OSC B
        allLayers[i]->oscB.iFM.LEDPinID = 12;
        allLayers[i]->oscB.iFM.LEDPortID = 0;

        allLayers[i]->oscB.iMorph.LEDPinID = 11;
        allLayers[i]->oscB.iMorph.LEDPortID = 0;

        allLayers[i]->oscB.iEffect.LEDPinID = 10;
        allLayers[i]->oscB.iEffect.LEDPortID = 0;

        // ENV
        allLayers[i]->envA.iAmount.LEDPinID = 31;
        allLayers[i]->envA.iAmount.LEDPortID = 1;

        allLayers[i]->envF.iAmount.LEDPinID = 34;
        allLayers[i]->envF.iAmount.LEDPortID = 1;

        // OUT
        allLayers[i]->out.iVCA.LEDPinID = 27;
        allLayers[i]->out.iVCA.LEDPortID = 1;

        allLayers[i]->out.iPan.LEDPinID = 26;
        allLayers[i]->out.iPan.LEDPortID = 1;

        // lfoA
        allLayers[i]->lfoA.iFreq.LEDPinID = 29;
        allLayers[i]->lfoA.iFreq.LEDPortID = 0;

        allLayers[i]->lfoA.iShape.LEDPinID = 28;
        allLayers[i]->lfoA.iShape.LEDPortID = 0;

        allLayers[i]->lfoA.iAmount.LEDPinID = 27;
        allLayers[i]->lfoA.iAmount.LEDPortID = 0;

        // lfoB
        allLayers[i]->lfoB.iFreq.LEDPinID = 23;
        allLayers[i]->lfoB.iFreq.LEDPortID = 0;

        allLayers[i]->lfoB.iShape.LEDPinID = 21;
        allLayers[i]->lfoB.iShape.LEDPortID = 0;

        allLayers[i]->lfoB.iAmount.LEDPinID = 20;
        allLayers[i]->lfoB.iAmount.LEDPortID = 0;

        // MIX  //check renderbuffer led output
        allLayers[i]->mixer.oscALevelSteiner.LEDPinID = 8;
        allLayers[i]->mixer.oscALevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.oscBLevelSteiner.LEDPinID = 6;
        allLayers[i]->mixer.oscBLevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.subLevelSteiner.LEDPinID = 4;
        allLayers[i]->mixer.subLevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.noiseLevelSteiner.LEDPinID = 2;
        allLayers[i]->mixer.noiseLevelSteiner.LEDPortID = 0;

        allLayers[i]->mixer.oscALevelLadder.LEDPinID = 7;
        allLayers[i]->mixer.oscALevelLadder.LEDPortID = 0;

        allLayers[i]->mixer.oscBLevelLadder.LEDPinID = 5;
        allLayers[i]->mixer.oscBLevelLadder.LEDPortID = 0;

        allLayers[i]->mixer.subLevelLadder.LEDPinID = 3;
        allLayers[i]->mixer.subLevelLadder.LEDPortID = 0;

        allLayers[i]->mixer.noiseLevelLadder.LEDPinID = 1;
        allLayers[i]->mixer.noiseLevelLadder.LEDPortID = 0;

        // MIX  //check renderbuffer led output
        allLayers[i]->mixer.iOSCALevel.LEDPinID = 8;
        allLayers[i]->mixer.iOSCALevel.LEDPortID = 0;
        allLayers[i]->mixer.iOSCALevel.LEDPinID2 = 7;
        allLayers[i]->mixer.iOSCALevel.LEDPortID2 = 0;

        allLayers[i]->mixer.iOSCBLevel.LEDPinID = 6;
        allLayers[i]->mixer.iOSCBLevel.LEDPortID = 0;
        allLayers[i]->mixer.iOSCBLevel.LEDPinID2 = 5;
        allLayers[i]->mixer.iOSCBLevel.LEDPortID2 = 0;

        allLayers[i]->mixer.iSUBLevel.LEDPinID = 4;
        allLayers[i]->mixer.iSUBLevel.LEDPortID = 0;
        allLayers[i]->mixer.iSUBLevel.LEDPinID2 = 3;
        allLayers[i]->mixer.iSUBLevel.LEDPortID2 = 0;

        allLayers[i]->mixer.iNOISELevel.LEDPinID = 2;
        allLayers[i]->mixer.iNOISELevel.LEDPortID = 0;
        allLayers[i]->mixer.iNOISELevel.LEDPinID2 = 1;
        allLayers[i]->mixer.iNOISELevel.LEDPortID2 = 0;

        /////// Outputs
        allLayers[i]->lfoA.out.LEDPinID = 24;
        allLayers[i]->lfoA.out.LEDPortID = 0;

        allLayers[i]->lfoB.out.LEDPinID = 22;
        allLayers[i]->lfoB.out.LEDPortID = 1;

        allLayers[i]->envF.out.LEDPinID = 33;
        allLayers[i]->envF.out.LEDPortID = 1;

        allLayers[i]->envA.out.LEDPinID = 30;
        allLayers[i]->envA.out.LEDPortID = 1;

        allLayers[i]->feel.oSpread.LEDPinID = 13;
        allLayers[i]->feel.oSpread.LEDPortID = 1;

        /////// Module Output
        allLayers[i]->lfoA.LEDPinID = 30;
        allLayers[i]->lfoA.LEDPortID = 0;

        allLayers[i]->lfoB.LEDPinID = 22;
        allLayers[i]->lfoB.LEDPortID = 0;

        allLayers[i]->envF.LEDPinID = 19;
        allLayers[i]->envF.LEDPortID = 0;

        allLayers[i]->envA.LEDPinID = 25;
        allLayers[i]->envA.LEDPortID = 1;

        /////// Configs
        allLayers[i]->oscA.dOctaveSwitchSub.configureNumberLEDs(1);
        allLayers[i]->oscA.dOctaveSwitchSub.LEDPinID[0] = 13;
        allLayers[i]->oscA.dOctaveSwitchSub.LEDPortID[0] = 0;

        allLayers[i]->oscB.dSync.configureNumberLEDs(1);
        allLayers[i]->oscB.dSync.LEDPinID[0] = 9;
        allLayers[i]->oscB.dSync.LEDPortID[0] = 0;

        allLayers[i]->mixer.dNOISEDestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPinID[0] = 2;
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPinID[1] = 1;
        allLayers[i]->mixer.dNOISEDestSwitch.LEDPortID[1] = 0;

        allLayers[i]->mixer.dOSCADestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dOSCADestSwitch.LEDPinID[0] = 8;
        allLayers[i]->mixer.dOSCADestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dOSCADestSwitch.LEDPinID[1] = 7;
        allLayers[i]->mixer.dOSCADestSwitch.LEDPortID[1] = 0;

        allLayers[i]->mixer.dOSCBDestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPinID[0] = 6;
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPinID[1] = 5;
        allLayers[i]->mixer.dOSCBDestSwitch.LEDPortID[1] = 0;

        allLayers[i]->mixer.dSUBDestSwitch.configureNumberLEDs(2);
        allLayers[i]->mixer.dSUBDestSwitch.LEDPinID[0] = 4;
        allLayers[i]->mixer.dSUBDestSwitch.LEDPortID[0] = 0;
        allLayers[i]->mixer.dSUBDestSwitch.LEDPinID[1] = 3;
        allLayers[i]->mixer.dSUBDestSwitch.LEDPortID[1] = 0;

        allLayers[i]->lfoA.dAlignLFOs.configureNumberLEDs(1);
        allLayers[i]->lfoA.dAlignLFOs.LEDPinID[0] = 25;
        allLayers[i]->lfoA.dAlignLFOs.LEDPortID[0] = 0;

        allLayers[i]->lfoA.dGateTrigger.configureNumberLEDs(1);
        allLayers[i]->lfoA.dGateTrigger.LEDPinID[0] = 26;
        allLayers[i]->lfoA.dGateTrigger.LEDPortID[0] = 0;

        allLayers[i]->lfoB.dAlignLFOs.configureNumberLEDs(1);
        allLayers[i]->lfoB.dAlignLFOs.LEDPinID[0] = 24;
        allLayers[i]->lfoB.dAlignLFOs.LEDPortID[0] = 1;

        allLayers[i]->lfoB.dGateTrigger.configureNumberLEDs(1);
        allLayers[i]->lfoB.dGateTrigger.LEDPinID[0] = 23;
        allLayers[i]->lfoB.dGateTrigger.LEDPortID[0] = 1;

        allLayers[i]->ladder.dSlope.configureNumberLEDs(4);
        allLayers[i]->ladder.dSlope.LEDPinID[0] = 18;
        allLayers[i]->ladder.dSlope.LEDPortID[0] = 1;
        allLayers[i]->ladder.dSlope.LEDPinID[1] = 19;
        allLayers[i]->ladder.dSlope.LEDPortID[1] = 1;
        allLayers[i]->ladder.dSlope.LEDPinID[2] = 20;
        allLayers[i]->ladder.dSlope.LEDPortID[2] = 1;
        allLayers[i]->ladder.dSlope.LEDPinID[3] = 21;
        allLayers[i]->ladder.dSlope.LEDPortID[3] = 1;

        allLayers[i]->steiner.dMode.configureNumberLEDs(4);
        allLayers[i]->steiner.dMode.LEDPinID[0] = 14;
        allLayers[i]->steiner.dMode.LEDPortID[0] = 1;
        allLayers[i]->steiner.dMode.LEDPinID[1] = 15;
        allLayers[i]->steiner.dMode.LEDPortID[1] = 1;
        allLayers[i]->steiner.dMode.LEDPinID[2] = 16;
        allLayers[i]->steiner.dMode.LEDPortID[2] = 1;
        allLayers[i]->steiner.dMode.LEDPinID[3] = 17;
        allLayers[i]->steiner.dMode.LEDPortID[3] = 1;

        //////MIDI

        allLayers[i]->midi.oNote.LEDPinID = 0;
        allLayers[i]->midi.oNote.LEDPortID = 2;

        allLayers[i]->midi.oVelocity.LEDPinID = 1;
        allLayers[i]->midi.oVelocity.LEDPortID = 2;

        allLayers[i]->midi.oMod.LEDPinID = 4;
        allLayers[i]->midi.oMod.LEDPortID = 2;

        allLayers[i]->midi.oPitchbend.LEDPinID = 3;
        allLayers[i]->midi.oPitchbend.LEDPortID = 2;

        allLayers[i]->midi.oAftertouch.LEDPinID = 2;
        allLayers[i]->midi.oAftertouch.LEDPortID = 2;
    }
}

void potiMapping() {
    for (uint32_t i = 0; i < 2; i++) { // register potis for both layer
        if (allLayers[i]->layerState == true) {

            potiFunctionPointer[i][0][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aParSer), std::placeholders::_1),
                &allLayers[i]->steiner.aParSer};
            potiFunctionPointer[i][1][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aSpread), std::placeholders::_1),
                &allLayers[i]->feel.aSpread};
            potiFunctionPointer[i][2][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aDetune), std::placeholders::_1),
                &allLayers[i]->feel.aDetune};
            potiFunctionPointer[i][3][0] = {
                std::bind(&Analog::setValue, &(allLayers[i]->feel.aGlide), std::placeholders::_1),
                &allLayers[i]->feel.aGlide};

            potiFunctionPointer[i][0][1] = {
                std::bind(&Digital::setValueRange, &(allLayers[i]->oscA.dOctave), std::placeholders::_1, 585, 4083),
                &allLayers[i]->oscA.dOctave};
            potiFunctionPointer[i][1][1] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aEffect), std::placeholders::_1),
                &allLayers[i]->oscA.aEffect};
            potiFunctionPointer[i][2][1] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMorph), std::placeholders::_1),
                &allLayers[i]->oscA.aMorph};
            potiFunctionPointer[i][3][1] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscA.aMasterTune), std::placeholders::_1),
                &allLayers[i]->oscA.aMasterTune};

            potiFunctionPointer[i][0][2] = {
                std::bind(&Digital::setValueRange, &(allLayers[i]->oscB.dOctave), std::placeholders::_1, 585, 4083),
                &allLayers[i]->oscB.dOctave};
            potiFunctionPointer[i][1][2] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aEffect), std::placeholders::_1),
                &allLayers[i]->oscB.aEffect};
            potiFunctionPointer[i][2][2] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aMorph), std::placeholders::_1),
                &allLayers[i]->oscB.aMorph};
            potiFunctionPointer[i][3][2] = {
                std::bind(&Analog::setValue, &(allLayers[i]->oscB.aTuning), std::placeholders::_1),
                &allLayers[i]->oscB.aTuning};

            potiFunctionPointer[i][0][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aNOISELevel), std::placeholders::_1),
                &allLayers[i]->mixer.aNOISELevel};
            potiFunctionPointer[i][1][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aSUBLevel), std::placeholders::_1),
                &allLayers[i]->mixer.aSUBLevel};
            potiFunctionPointer[i][2][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCBLevel), std::placeholders::_1),
                &allLayers[i]->mixer.aOSCBLevel};
            potiFunctionPointer[i][3][3] = {
                std::bind(&Analog::setValue, &(allLayers[i]->mixer.aOSCALevel), std::placeholders::_1),
                &allLayers[i]->mixer.aOSCALevel};

            potiFunctionPointer[i][1][4] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aLevel), std::placeholders::_1),
                &allLayers[i]->steiner.aLevel};
            potiFunctionPointer[i][2][4] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aResonance), std::placeholders::_1),
                &allLayers[i]->steiner.aResonance};
            potiFunctionPointer[i][3][4] = {
                std::bind(&Analog::setValue, &(allLayers[i]->steiner.aCutoff), std::placeholders::_1),
                &allLayers[i]->steiner.aCutoff};

            potiFunctionPointer[i][1][5] = {
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aLevel), std::placeholders::_1),
                &allLayers[i]->ladder.aLevel};
            potiFunctionPointer[i][2][5] = {
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aResonance), std::placeholders::_1),
                &allLayers[i]->ladder.aResonance};
            potiFunctionPointer[i][3][5] = {
                std::bind(&Analog::setValue, &(allLayers[i]->ladder.aCutoff), std::placeholders::_1),
                &allLayers[i]->ladder.aCutoff};

            potiFunctionPointer[i][1][6] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aAmount), std::placeholders::_1),
                &allLayers[i]->lfoA.aAmount};
            potiFunctionPointer[i][2][6] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aShape), std::placeholders::_1),
                &allLayers[i]->lfoA.aShape};
            potiFunctionPointer[i][3][6] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoA.aFreq), std::placeholders::_1),
                &allLayers[i]->lfoA.aFreq};

            potiFunctionPointer[i][0][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envF.aDelay), std::placeholders::_1),
                &allLayers[i]->envF.aDelay, true};
            potiFunctionPointer[i][1][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aAmount), std::placeholders::_1),
                &allLayers[i]->lfoB.aAmount};
            potiFunctionPointer[i][2][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aShape), std::placeholders::_1),
                &allLayers[i]->lfoB.aShape};
            potiFunctionPointer[i][3][7] = {
                std::bind(&Analog::setValue, &(allLayers[i]->lfoB.aFreq), std::placeholders::_1),
                &allLayers[i]->lfoB.aFreq};

            potiFunctionPointer[i][0][8] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envF.aAttack), std::placeholders::_1),
                &allLayers[i]->envF.aAttack, true};
            potiFunctionPointer[i][1][8] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envF.aDecay), std::placeholders::_1),
                &allLayers[i]->envF.aDecay, true};
            potiFunctionPointer[i][2][8] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envF.aSustain), std::placeholders::_1),
                &allLayers[i]->envF.aSustain, true};
            potiFunctionPointer[i][3][8] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envF.aRelease), std::placeholders::_1),
                &allLayers[i]->envF.aRelease, true};

            potiFunctionPointer[i][0][9] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envA.aSustain), std::placeholders::_1),
                &allLayers[i]->envA.aSustain, true};
            potiFunctionPointer[i][1][9] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envA.aRelease), std::placeholders::_1),
                &allLayers[i]->envA.aRelease, true};
            potiFunctionPointer[i][2][9] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envA.aAmount), std::placeholders::_1),
                &allLayers[i]->envA.aAmount, true};
            potiFunctionPointer[i][3][9] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envF.aAmount), std::placeholders::_1),
                &allLayers[i]->envF.aAmount, true};

            potiFunctionPointer[i][0][10] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envA.aDelay), std::placeholders::_1),
                &allLayers[i]->envA.aDelay, true};
            potiFunctionPointer[i][1][10] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envA.aAttack), std::placeholders::_1),
                &allLayers[i]->envA.aAttack, true};
            potiFunctionPointer[i][2][10] = {
                std::bind(&Analog::setValue, &(allLayers[i]->envA.aDecay), std::placeholders::_1),
                &allLayers[i]->envA.aDecay, true};
            potiFunctionPointer[i][3][10] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aMaster), std::placeholders::_1),
                &allLayers[i]->out.aMaster};

            potiFunctionPointer[i][0][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aDistort), std::placeholders::_1),
                &allLayers[i]->out.aDistort};
            potiFunctionPointer[i][1][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aPanSpread), std::placeholders::_1),
                &allLayers[i]->out.aPanSpread};
            potiFunctionPointer[i][2][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aPan), std::placeholders::_1),
                &allLayers[i]->out.aPan};
            potiFunctionPointer[i][3][11] = {
                std::bind(&Analog::setValue, &(allLayers[i]->out.aVCA), std::placeholders::_1),
                &allLayers[i]->out.aVCA};
        }
    }
}

void HIDConfig() {

    // Control
    FlagHandler::Control_Touch_ISR = std::bind(processControlTouch);
    FlagHandler::Control_Encoder_ISR = std::bind(processEncoder);

    // Panels
    FlagHandler::Panel_0_RXTX_ISR = std::bind(processPanelPotis, adcA.adcData, 0);
    FlagHandler::Panel_1_RXTX_ISR = std::bind(processPanelPotis, adcB.adcData, 1);

    FlagHandler::Panel_0_EOC_ISR = std::bind(&MAX11128::fetchNewData, &adcA);
    FlagHandler::Panel_1_EOC_ISR = std::bind(&MAX11128::fetchNewData, &adcB);
    FlagHandler::Panel_nextChannel_ISR = std::bind(&TS3A5017D::nextChannel, &multiplexer);

    FlagHandler::Panel_0_Touch_ISR = std::bind(processPanelTouch, 0);
    FlagHandler::Panel_1_Touch_ISR = std::bind(processPanelTouch, 1);

    // register encoder

    // MENU
    encoders[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 0),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 0));

    encoders[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 1),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 1));

    encoders[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 2),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 2));

    encoders[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 3),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 3));
    // SCROLL
    encoders[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 4),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 4));
    // AMOUNT
    encoders[5].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_CW, &actionHandler, 5),
                                       std::bind(&actionMapping::callActionEncoder_CCW, &actionHandler, 5));

    switches[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 0), nullptr);
    switches[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 1), nullptr);
    switches[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 2), nullptr);
    switches[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 3), nullptr);
    switches[4].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 4), nullptr);
    switches[5].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_Push, &actionHandler, 5), nullptr);

    // register transmit functions for the led Driver
    FlagHandler::ledDriverA_ISR[0] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[0][0]);
    FlagHandler::ledDriverA_ISR[1] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[0][1]);

    FlagHandler::ledDriverB_ISR[0] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[1][0]);
    FlagHandler::ledDriverB_ISR[1] = std::bind(&IS32FL3237::sendPWMData, &ledDriver[1][1]);

    FlagHandler::ledDriverControl_ISR = std::bind(&IS31FL3205::sendPWMData, &ledDriverControl);

    FlagHandler::ledDriverUpdateCurrent_ISR = std::bind(updateLEDDriverCurrent);

    LEDMappingInit();

    // clear all poti Register

    for (size_t a = 0; a < 2; a++) {
        for (size_t b = 0; b < 4; b++) {
            for (size_t c = 0; c < 12; c++) {
                potiFunctionPointer[a][b][c] = {nullptr, nullptr};
                panelADCStates[a][b][c] = 0;
                panelGrab[a][b][c] = VALUEGRABED;
                panelADCInterpolate[a][b][c] = 0;
            }
        }
    }

    potiMapping();

    // activate FlagHandling for HID ICs
    FlagHandler::HID_Initialized = true;
}

#endif