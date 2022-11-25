#ifdef POLYRENDER

#include "renderADSR.hpp"
#include "renderCVDef.h"

extern Layer layerA;

// inline float accumulateDelay(const ADSR &adsr, uint32_t voice) {
//     return std::clamp(adsr.iDelay[voice] + adsr.aDelay, adsr.aDelay.min, adsr.aDelay.max * 2);
// }
// inline float accumulateAttack(const ADSR &adsr, uint32_t voice) {
//     return std::clamp(adsr.iAttack[voice] + adsr.aAttack, adsr.aAttack.min, adsr.aAttack.max * 2);
// }
// inline float accumulateDecay(const ADSR &adsr, uint32_t voice) {
//     return std::clamp(adsr.iDecay[voice] + adsr.aDecay, adsr.aDecay.min, adsr.aDecay.max * 2);
// }
// inline vec<VOICESPERCHIP> accumulateSustain(const ADSR &adsr) {
//     return clamp(adsr.iSustain + adsr.aSustain, adsr.aSustain.min, adsr.aSustain.max);
// }
// inline float accumulateRelease(const ADSR &adsr, uint32_t voice) {
//     return std::clamp(adsr.iRelease[voice] + adsr.aRelease, adsr.aRelease.min, adsr.aRelease.max * 2);
// }
inline vec<VOICESPERCHIP> accumulateAmount(const ADSR &adsr) {
    return clamp(adsr.iAmount + adsr.aAmount, adsr.aAmount.min, adsr.aAmount.max);
}

inline float calcCoef(float rate, float targetRatio) {
    return (rate <= 0.0f) ? 0.0f : expf(-logf((1.0f + targetRatio) / targetRatio) / rate);
}

// goal value + speed; smaller values equal more log, big ones lin
inline float calcARatio(float shape) {
    return 0.1f + 0.001f * (expf(12.0f * (1.0f - shape)) - 1.0f);
}
inline float calcDRRatio(float shape) {
    return 0.0001f + 0.001f * (expf(12.0f * (1.0f - shape)) - 1.0f);
}

#define ADSRTHRESHOLD 0.001f
/**
 * @brief render next output sample of an ADSR module
 *
 * @param adsr
 */
void renderADSR(ADSR &adsr) {

    // float delay, decay, attack, release;
    const int32_t &loop = adsr.dLoop.valueMapped;
    const float &shape = adsr.aShape;
    static float cacheShape = adsr.aShape;
    static float targetRatioA = calcARatio(shape);
    static float targetRatioDR = calcDRRatio(shape);

    static float cacheTargetRatioA;
    static float cacheTargetRatioD;
    static float cacheTargetRatioR;

    static float cacheAttack;
    static float cacheDecay;
    static float cacheRelease;

    static float attackCoef;
    static float attackBase;
    static float decayCoef;
    static float releaseCoef;
    static float releaseBase;

    if (cacheShape != shape) {
        targetRatioA = calcARatio(shape);
        targetRatioDR = calcDRRatio(shape);
        cacheShape = shape;
    }

    // adsr.sustain = accumulateSustain(adsr);
    adsr.amount = accumulateAmount(adsr);

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {

        float &level = adsr.level[voice];
        float &currentTime = adsr.currentTime[voice];
        const bool &gate = adsr.gate[voice];
        const float &sustain = adsr.aSustain;
        bool &retriggered = adsr.retriggered[voice];

        const float &imperfection = layerA.adsrImperfection[voice];

        // extra if to reduce the input delay 0.5ms

        if (adsr.currentState[voice] == adsr.OFF) {

            if (gate == 1 || loop == 1) {
                adsr.setStatusDelay(voice);
            }
        }

        if (adsr.currentState[voice] == adsr.DELAY) {

            if (gate == 0 && loop == 0 && retriggered == 0) {
                adsr.setStatusOff(voice);
            }

            float delay = adsr.aDelay;
            currentTime += SECONDSPERCVRENDER * imperfection;
            if (currentTime >= delay)
                adsr.setStatusAttack(voice);
        }

        switch (adsr.currentState[voice]) {
            case adsr.OFF:
            case adsr.DELAY: break;

            case adsr.ATTACK: {
                float attack = adsr.aAttack * imperfection;

                if (attack != cacheAttack || targetRatioA != cacheTargetRatioA) {
                    float attackRate = attack * (1.0f / SECONDSPERCVRENDER);
                    attackCoef = calcCoef(attackRate, targetRatioA);
                    attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);

                    cacheAttack = attack;
                    cacheTargetRatioA = targetRatioA;
                }

                level = attackBase + level * attackCoef;

                if (level >= 1.0f) {
                    level = 1;
                    if (gate == 1)
                        adsr.setStatusDecay(voice);
                    else if (loop == 1 || retriggered == 1)
                        adsr.setStatusRelease(voice);
                }

                if (gate == 0 && loop == 0 && retriggered == 0)
                    adsr.setStatusRelease(voice);

                break;
            }

            case adsr.DECAY: {

                float decay = adsr.aDecay * imperfection;

                if (decay != cacheDecay || targetRatioDR != cacheTargetRatioD) {
                    float decayRate = decay * (1.0f / SECONDSPERCVRENDER);
                    decayCoef = calcCoef(decayRate, targetRatioDR);

                    cacheDecay = decay;
                    cacheTargetRatioD = targetRatioDR;
                }

                float decayBase = (sustain - targetRatioDR) * (1.0f - decayCoef);

                level = decayBase + level * decayCoef;

                if (level <= sustain) {
                    level = sustain;
                    adsr.setStatusSustain(voice);
                }

                if (gate == 0) {
                    adsr.setStatusRelease(voice);
                }
                break;
            }

            case adsr.SUSTAIN: {

                if (gate == 0) {
                    adsr.setStatusRelease(voice);
                }

                float decay = adsr.aDecay * imperfection;

                if (level < sustain - ADSRTHRESHOLD) {
                    level += (SECONDSPERCVRENDER / decay);
                    if (level >= sustain)
                        level = sustain;
                }
                else if (level > sustain + ADSRTHRESHOLD) {
                    level -= (SECONDSPERCVRENDER / decay);
                    if (level <= sustain)
                        level = sustain;
                }
                else
                    level = sustain;

                break;
            }

            case adsr.RELEASE: {
                float release = adsr.aRelease * imperfection;

                if (release != cacheRelease || targetRatioDR != cacheTargetRatioR) {
                    float releaseRate = release * (1.0f / SECONDSPERCVRENDER);
                    releaseCoef = calcCoef(releaseRate, targetRatioDR);
                    releaseBase = -targetRatioDR * (1.0f - releaseCoef);

                    cacheRelease = release;
                    cacheTargetRatioR = targetRatioDR;
                }

                level = releaseBase + level * releaseCoef;

                if (level <= ADSRTHRESHOLD) {
                    retriggered = 0;
                    level = 0;
                    if (loop == 0)
                        adsr.setStatusOff(voice);
                    else
                        adsr.setStatusDelay(voice);
                }

                if (gate == 1) {
                    adsr.setStatusAttack(voice);
                }

                break;
            }
            default: Error_Handler(); break;
        }
    }

    // adsr.level = clamp(adsr.level, 0.0f, 1.0f);

    // midi velocity
    adsr.out = faster_lerp_f32(adsr.level, adsr.level * layerA.midi.oVelocity, adsr.aVelocity);

    // keytrack
    adsr.out = faster_lerp_f32(adsr.out, adsr.out * layerA.midi.oNote, adsr.aKeytrack);

    adsr.currentSampleRAW = adsr.out;

    adsr.out = adsr.out * adsr.amount;
}

#endif