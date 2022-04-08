#ifdef POLYRENDER

#include "renderADSR.hpp"
#include "renderCVDef.h"

extern Layer layerA;

LogCurve adsrConvertLog(128, 0.1);
LogCurve adsrConvertAntiLog(128, 0.9);

inline float accumulateDelay(ADSR &adsr, uint16_t voice) {
    return std::clamp(adsr.iDelay[voice] + adsr.aDelay, adsr.aDelay.min, adsr.aDelay.max * 2);
}
inline float accumulateAttack(ADSR &adsr, uint16_t voice) {
    return std::clamp(adsr.iAttack[voice] + adsr.aAttack, adsr.aAttack.min, adsr.aAttack.max * 2);
}
inline float accumulateDecay(ADSR &adsr, uint16_t voice) {
    return std::clamp(adsr.iDecay[voice] + adsr.aDecay, adsr.aDecay.min, adsr.aDecay.max * 2);
}
inline float accumulateSustain(ADSR &adsr, uint16_t voice) {
    return std::clamp(adsr.iSustain[voice] + adsr.aSustain, adsr.aSustain.min, adsr.aSustain.max);
}
inline float accumulateRelease(ADSR &adsr, uint16_t voice) {
    return std::clamp(adsr.iRelease[voice] + adsr.aRelease, adsr.aRelease.min, adsr.aRelease.max * 2);
}
inline float accumulateAmount(ADSR &adsr, uint16_t voice) {
    return std::clamp(adsr.iAmount[voice] + adsr.aAmount, adsr.aAmount.min, adsr.aAmount.max);
}

/**
 * @brief render next output sample of an ADSR module
 *
 * @param adsr
 */
void renderADSR(ADSR &adsr) {

    // TODO bezier
    float delay, decay, attack, sustain, release;
    int32_t &loop = adsr.dLoop.valueMapped;
    float &shape = adsr.aShape.valueMapped;

    vec<VOICESPERCHIP> sample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        float &nextSample = sample[voice];
        float &currentLevel = adsr.currentLevel[voice];
        float &currentTime = adsr.currentTime[voice];
        float &gate = layerA.midi.oGate[voice];

        float imperfection = 1 + layerA.adsrImperfection[voice] * layerA.feel.aImperfection.valueMapped;

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
                    currentTime += SECONDSPERCVRENDER * imperfection;
                    if (currentTime >= delay)
                        adsr.setStatusAttack(voice);
                }
                break;

            case adsr.ATTACK:
                attack = accumulateAttack(adsr, voice);
                currentLevel += (SECONDSPERCVRENDER / attack) * imperfection;

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
                currentLevel -= (SECONDSPERCVRENDER / decay) * imperfection;

                sustain = accumulateSustain(adsr, voice);

                // fix Sustain level
                if (shape < 1) {
                    // shape between 0 and 1, 1 is linear
                    sustain = fast_lerp_f32(adsrConvertAntiLog.mapValue(sustain), sustain, shape);
                }
                else {
                    // shape between 1 and 2, 1 is linear
                    sustain = fast_lerp_f32(sustain, adsrConvertLog.mapValue(sustain), shape - 1.0f);
                }
                //

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

                // fix Sustain level
                if (shape < 1) {
                    // shape between 0 and 1, 1 is linear
                    sustain = fast_lerp_f32(adsrConvertAntiLog.mapValue(sustain), sustain, shape);
                }
                else {
                    // shape between 1 and 2, 1 is linear
                    sustain = fast_lerp_f32(sustain, adsrConvertLog.mapValue(sustain), shape - 1.0f);
                }
                //

                if (currentLevel != sustain) {
                    decay = accumulateDecay(adsr, voice);

                    if (currentLevel < sustain) {
                        currentLevel += (SECONDSPERCVRENDER / decay) * imperfection;
                        if (currentLevel >= sustain) {
                            currentLevel = sustain;
                        }
                    }
                    else {
                        currentLevel -= (SECONDSPERCVRENDER / decay) * imperfection;
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
                currentLevel -= (SECONDSPERCVRENDER / release) * imperfection;

                if (currentLevel <= 0) {
                    currentLevel = 0;
                    if (loop == 0)
                        adsr.setStatusOff(voice);
                    else
                        adsr.setStatusDelay(voice);
                }

                if (gate == 1) {
                    adsr.setStatusAttack(voice);
                }

                break;
            default: Error_Handler(); break;
        }

        if (shape < 1) {
            // shape between 0 and 1, 1 is linear
            nextSample = fast_lerp_f32(adsrConvertLog.mapValue(currentLevel), currentLevel, shape);
        }
        else {
            // shape between 1 and 2, 1 is linear
            nextSample = fast_lerp_f32(currentLevel, adsrConvertAntiLog.mapValue(currentLevel), shape - 1.0f);
        }
    }

    // midi velocity
    sample = fast_lerp_f32(sample, sample * layerA.midi.oVeloctiy, adsr.aVelocity);

    // keytrack
    sample = fast_lerp_f32(sample, sample * layerA.midi.oNote, adsr.aKeytrack);

    sample = sample * adsr.aAmount;
}

#endif