#ifdef POLYRENDER

#include "renderADSR.hpp"
#include "renderCVDef.h"

extern Layer layerA;

inline float accumulateDelay(const ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iDelay[voice] + adsr.aDelay, adsr.aDelay.min, adsr.aDelay.max * 2);
}
inline float accumulateAttack(const ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iAttack[voice] + adsr.aAttack, adsr.aAttack.min, adsr.aAttack.max * 2);
}
inline float accumulateDecay(const ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iDecay[voice] + adsr.aDecay, adsr.aDecay.min, adsr.aDecay.max * 2);
}
inline vec<VOICESPERCHIP> accumulateSustain(const ADSR &adsr) {
    return clamp(adsr.iSustain + adsr.aSustain, adsr.aSustain.min, adsr.aSustain.max);
}
inline float accumulateRelease(const ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iRelease[voice] + adsr.aRelease, adsr.aRelease.min, adsr.aRelease.max * 2);
}
inline float accumulateAmount(const ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iAmount[voice] + adsr.aAmount, adsr.aAmount.min, adsr.aAmount.max);
}

inline float calcCoef(float rate, float targetRatio) {
    return (rate <= 0.0f) ? 0.0f : expf(-logf((1.0f + targetRatio) / targetRatio) / rate);
}
inline float calcARatio(float shape) {
    return 0.0001f + 0.001f * (expf(12.0f * shape) - 1.0f);
}
inline float calcDRRatio(float shape) {
    return 0.00001f + 0.0001f * (expf(12.0f * shape) - 1.0f);
}

#define ADSRTHRESHOLD 0.001f
/**
 * @brief render next output sample of an ADSR module
 *
 * @param adsr
 */
void renderADSR(ADSR &adsr) {

    float delay, decay, attack, release;
    const int32_t &loop = adsr.dLoop.valueMapped;
    const float &shape = adsr.aShape;
    static float cacheShape = adsr.aShape;
    static float targetRatioA = calcARatio(shape);
    static float targetRatioDR = calcDRRatio(shape);

    static float cacheTargetRatioA = 0;
    static float cacheTargetRatioD = 0;
    static float cacheTargetRatioR = 0;

    static vec<VOICESPERCHIP> cacheAttack = 0;
    static vec<VOICESPERCHIP> cacheDecay = 0;
    static vec<VOICESPERCHIP> cacheRelease = 0;

    static vec<VOICESPERCHIP> attackCoef = 0;
    static vec<VOICESPERCHIP> attackBase = 0;
    static vec<VOICESPERCHIP> decayCoef = 0;
    static vec<VOICESPERCHIP> releaseCoef = 0;
    static vec<VOICESPERCHIP> releaseBase = 0;

    if (cacheShape != shape) {
        targetRatioA = calcARatio(shape);
        targetRatioDR = calcDRRatio(shape);
        cacheShape = shape;
    }

    adsr.sustain = accumulateSustain(adsr);

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {

        float &level = adsr.level[voice];
        float &currentTime = adsr.currentTime[voice];
        const bool &gate = adsr.gate[voice];
        float &sustain = adsr.sustain[voice];
        bool &retriggered = adsr.retriggered[voice];

        float imperfection = 1.0f;

        switch (adsr.currentState[voice]) {
            case adsr.OFF:
                if (gate == 1 || loop == 1) {
                    adsr.setStatusDelay(voice);
                }
                break;

            case adsr.DELAY:
                if (gate == 0 && loop == 0 && retriggered == 0) {
                    adsr.setStatusOff(voice);
                }
                delay = accumulateDelay(adsr, voice);
                currentTime += SECONDSPERCVRENDER * imperfection;
                if (currentTime >= delay)
                    adsr.setStatusAttack(voice);
                break;

            case adsr.ATTACK:
                attack = accumulateAttack(adsr, voice) * imperfection;

                if (attack != cacheAttack[voice] || targetRatioA != cacheTargetRatioA) {
                    float attackRate = attack * (1.0f / SECONDSPERCVRENDER);
                    attackCoef[voice] = calcCoef(attackRate, targetRatioA);
                    attackBase[voice] = (1.0 + targetRatioA) * (1.0 - attackCoef[voice]);

                    cacheAttack[voice] = attack;
                    cacheTargetRatioA = targetRatioA;
                }

                level = attackBase[voice] + level * attackCoef[voice];

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

            case adsr.DECAY: {

                decay = accumulateDecay(adsr, voice) * imperfection;

                if (decay != cacheDecay[voice] || targetRatioDR != cacheTargetRatioD) {
                    float decayRate = decay * (1.0f / SECONDSPERCVRENDER);
                    decayCoef[voice] = calcCoef(decayRate, targetRatioDR);

                    cacheDecay[voice] = decay;
                    cacheTargetRatioD = targetRatioDR;
                }

                float decayBase = (sustain - targetRatioDR) * (1.0f - decayCoef[voice]);

                level = decayBase + level * decayCoef[voice];

                if (level <= sustain) {
                    level = sustain;
                    adsr.setStatusSustain(voice);
                }

                if (gate == 0) {
                    adsr.setStatusRelease(voice);
                }
                break;
            }

            case adsr.SUSTAIN:

                if (gate == 0) {
                    adsr.setStatusRelease(voice);
                }

                decay = accumulateDecay(adsr, voice) * imperfection;

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

            case adsr.RELEASE:
                release = accumulateRelease(adsr, voice) * imperfection;

                if (release != cacheRelease[voice] || targetRatioDR != cacheTargetRatioR) {
                    float releaseRate = release * (1.0f / SECONDSPERCVRENDER);
                    releaseCoef[voice] = calcCoef(releaseRate, targetRatioDR);
                    releaseBase[voice] = -targetRatioDR * (1.0f - releaseCoef[voice]);

                    cacheRelease[voice] = release;
                    cacheTargetRatioR = targetRatioDR;
                }

                level = releaseBase[voice] + level * releaseCoef[voice];

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
            default: Error_Handler(); break;
        }
    }

    adsr.level = clamp(adsr.level, 0.0f, 1.0f);

    // midi velocity
    adsr.out = fast_lerp_f32(adsr.level, adsr.level * layerA.midi.oVelocity, adsr.aVelocity);

    // keytrack
    adsr.out = fast_lerp_f32(adsr.out.nextSample, adsr.out.nextSample * layerA.midi.oNote, adsr.aKeytrack);

    adsr.out = adsr.out.nextSample * adsr.aAmount;
}

#endif