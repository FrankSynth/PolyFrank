#include "renderADSR.hpp"
#include "renderCV.hpp"

extern Layer layerA;

#define INPUTWEIGHTING 1

inline float accumulateDelay(ADSR &adsr, uint16_t voice) {
    return testFloat(adsr.iDelay.currentSample[voice] * adsr.aDelay.valueMapped * INPUTWEIGHTING +
                         adsr.aDelay.valueMapped,
                     adsr.aDelay.min, adsr.aDelay.max * 2);
}
inline float accumulateAttack(ADSR &adsr, uint16_t voice) {
    return testFloat(adsr.iAttack.currentSample[voice] * adsr.aAttack.valueMapped * INPUTWEIGHTING +
                         adsr.aAttack.valueMapped,
                     adsr.aAttack.min, adsr.aAttack.max * 2);
}
inline float accumulateDecay(ADSR &adsr, uint16_t voice) {
    return testFloat(adsr.iDecay.currentSample[voice] * adsr.aDecay.valueMapped * INPUTWEIGHTING +
                         adsr.aDecay.valueMapped,
                     adsr.aDecay.min, adsr.aDecay.max * 2);
}
inline float accumulateSustain(ADSR &adsr, uint16_t voice) {
    return testFloat(adsr.iSustain.currentSample[voice] * adsr.aSustain.valueMapped * INPUTWEIGHTING +
                         adsr.aSustain.valueMapped,
                     adsr.aSustain.min, adsr.aSustain.max);
}
inline float accumulateRelease(ADSR &adsr, uint16_t voice) {
    return testFloat(adsr.iRelease.currentSample[voice] * adsr.aRelease.valueMapped * INPUTWEIGHTING +
                         adsr.aRelease.valueMapped,
                     adsr.aRelease.min, adsr.aRelease.max * 2);
}
inline float accumulateAmount(ADSR &adsr, uint16_t voice) {
    return testFloat(adsr.iAmount.currentSample[voice] * adsr.aAmount.valueMapped * INPUTWEIGHTING +
                         adsr.aAmount.valueMapped,
                     adsr.aAmount.min, adsr.aAmount.max);
}

/**
 * @brief render next output sample of an ADSR module
 *
 * @param adsr
 */
void renderADSR(ADSR &adsr) {

    float delay, decay, attack, sustain, release;
    int32_t &loop = adsr.dLoop.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        float &currentLevel = adsr.currentLevel[voice];
        float &currentTime = adsr.currentTime[voice];
        float &gate = layerA.midi.oGate.currentSample[voice];

        switch (adsr.getState(voice)) {
            case adsr.OFF:
                if (gate == 1 || loop == 1) {
                    adsr.setStatusDelay(voice);
                }
                break;

            case adsr.DELAY:
                if (gate == 0 && loop == 0) {
                    adsr.setStatusOff(voice);
                }
                else {
                    delay = accumulateDelay(adsr, voice);
                    currentTime += secondsPerCVRender;
                    if (currentTime >= delay)
                        adsr.setStatusAttack(voice);
                }
                break;

            case adsr.ATTACK:
                attack = accumulateAttack(adsr, voice);
                currentLevel += secondsPerCVRender / attack;

                if (currentLevel >= 1) {
                    currentLevel = 1;
                    if (gate == 1) {
                        adsr.setStatusDecay(voice);
                    }
                }

                if (gate == 0) {
                    // gate 0
                    if (loop == 0) {
                        // gate 0 loop 0
                        adsr.setStatusRelease(voice);
                    }
                    else {
                        // gate 0 loop 1
                        if (currentLevel == 1) {
                            adsr.setStatusRelease(voice);
                        }
                    }
                }

                break;

            case adsr.DECAY:
                decay = accumulateDecay(adsr, voice);
                currentLevel -= secondsPerCVRender / decay;

                sustain = accumulateSustain(adsr, voice);
                if (currentLevel <= sustain) {
                    currentLevel = sustain;
                    adsr.setStatusSustain(voice);
                }

                if (gate == 0) {
                    adsr.setStatusRelease(voice);
                }
                break;

            case adsr.SUSTAIN:
                sustain = accumulateSustain(adsr, voice);
                if (currentLevel != sustain) {
                    decay = accumulateDecay(adsr, voice);

                    if (currentLevel < sustain) {
                        currentLevel += secondsPerCVRender / decay;
                        if (currentLevel >= sustain) {
                            currentLevel = sustain;
                        }
                    }
                    else {
                        currentLevel -= secondsPerCVRender / decay;
                        if (currentLevel <= sustain) {
                            currentLevel = sustain;
                        }
                    }
                }

                if (gate == 0) {
                    adsr.setStatusRelease(voice);
                }
                break;

            case adsr.RELEASE:
                release = accumulateRelease(adsr, voice);
                currentLevel -= secondsPerCVRender / release;

                if (currentLevel <= 0) {
                    currentLevel = 0;
                    if (loop == 0)
                        adsr.setStatusOff(voice);
                    else
                        adsr.setStatusAttack(voice);
                }

                if (gate == 1) {
                    adsr.setStatusAttack(voice);
                }

                break;
            default: Error_Handler(); break;
        }

        // midi velocity
        adsr.out.nextSample[voice] = fast_lerp_f32(
            currentLevel, currentLevel * layerA.midi.oVeloctiy.currentSample[voice], adsr.aVelocity.valueMapped);

        // TODO keytrack MISSING, input note necessary

        adsr.out.nextSample[voice] = adsr.out.nextSample[voice] * adsr.aAmount.valueMapped;
    }
}