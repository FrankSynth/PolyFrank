#ifdef POLYRENDER

#include "renderADSR.hpp"
#include "renderCVDef.h"

extern Layer layerA;

LogCurve adsrConvertLog(128, 0.1);
LogCurve adsrConvertAntiLog(128, 0.9);

extern uint8_t sendString(const char *message);
extern uint8_t sendString(std::string &message);
extern uint8_t sendString(std::string &&message);

inline float accumulateDelay(ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iDelay[voice] + adsr.aDelay, adsr.aDelay.min, adsr.aDelay.max * 2);
}
inline float accumulateAttack(ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iAttack[voice] + adsr.aAttack, adsr.aAttack.min, adsr.aAttack.max * 2);
}
inline float accumulateDecay(ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iDecay[voice] + adsr.aDecay, adsr.aDecay.min, adsr.aDecay.max * 2);
}
inline vec<VOICESPERCHIP> accumulateSustain(ADSR &adsr) {
    return clamp(adsr.iSustain + adsr.aSustain, adsr.aSustain.min, adsr.aSustain.max);
}
inline float accumulateRelease(ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iRelease[voice] + adsr.aRelease, adsr.aRelease.min, adsr.aRelease.max * 2);
}
inline float accumulateAmount(ADSR &adsr, uint32_t voice) {
    return std::clamp(adsr.iAmount[voice] + adsr.aAmount, adsr.aAmount.min, adsr.aAmount.max);
}

inline void setStatusOff(ADSR &adsr, uint32_t voice) {
    adsr.currentState[voice] = adsr.OFF;
}
inline void setStatusDelay(ADSR &adsr, uint32_t voice) {
    adsr.currentState[voice] = adsr.DELAY;
    adsr.currentTime[voice] = 0;
}
inline void setStatusAttack(ADSR &adsr, uint32_t voice) {
    adsr.currentState[voice] = adsr.ATTACK;
    // adsr.currentTime[voice] = reverseSimpleBezier1D(adsr.level[voice], adsr.aShape);
}
inline void setStatusDecay(ADSR &adsr, uint32_t voice) {
    if (adsr.level[voice] < adsr.sustain[voice]) {
        setStatusAttack(adsr, voice);
        return;
    }

    adsr.currentState[voice] = adsr.DECAY;
    // adsr.currentTime[voice] = 0;
}
inline void setStatusSustain(ADSR &adsr, uint32_t voice) {
    adsr.currentState[voice] = adsr.SUSTAIN;
}
inline void setStatusRelease(ADSR &adsr, uint32_t voice) {
    adsr.currentState[voice] = adsr.RELEASE;
    // adsr.currentTime[voice] = reverseBezier1D(adsr.level[voice], 1.0f, 1.0f - adsr.aShape, 0.0f);
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

    // TODO bezier
    float delay, decay, attack, release;
    const int32_t &loop = adsr.dLoop.valueMapped;
    const float &shape = adsr.aShape;
    static float cacheShape = adsr.aShape;
    static float targetRatioA = calcARatio(shape);
    static float targetRatioDR = calcDRRatio(shape);

    if (cacheShape != shape) {
        targetRatioA = calcARatio(shape);
        targetRatioDR = calcDRRatio(shape);
        cacheShape = shape;
    }

    adsr.sustain = accumulateSustain(adsr);

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {

        // float &nextSample = level[voice];
        float &level = adsr.level[voice];
        float &currentTime = adsr.currentTime[voice];
        float &gate = layerA.midi.oGate[voice];
        float &sustain = adsr.sustain[voice];
        float attackBase;
        float attackCoef;
        float attackRate;
        float decayBase;
        float decayCoef;
        float decayRate;
        float releaseBase;
        float releaseCoef;
        float releaseRate;

        float imperfection = 1.0f;

        switch (adsr.currentState[voice]) {
            case adsr.OFF:
                if (gate == 1 || loop == 1) {
                    setStatusDelay(adsr, voice);
                }
                break;

            case adsr.DELAY:
                if (gate == 0 && loop == 0) {
                    setStatusOff(adsr, voice);
                }
                delay = accumulateDelay(adsr, voice);
                currentTime += SECONDSPERCVRENDER * imperfection;
                if (currentTime >= delay)
                    setStatusAttack(adsr, voice);
                break;

            case adsr.ATTACK:
                attack = accumulateAttack(adsr, voice) * imperfection;

                attackRate = attack * (1.0f / SECONDSPERCVRENDER);
                attackCoef = calcCoef(attackRate, targetRatioA);
                attackBase = (1.0 + targetRatioA) * (1.0 - attackCoef);

                // level = fast_lerp_f32(attackBase + level * attackCoef, level + SECONDSPERCVRENDER / attack, shape);
                level = attackBase + level * attackCoef;

                if (level >= 1.0f) {
                    level = 1;
                    if (gate == 1)
                        setStatusDecay(adsr, voice);
                    else if (loop == 1)
                        setStatusRelease(adsr, voice);
                }

                if (gate == 0 && loop == 0)
                    setStatusRelease(adsr, voice);

                // level = simpleBezier1D(shape, currentTime);

                break;

            case adsr.DECAY:
                decay = accumulateDecay(adsr, voice) * imperfection;

                decayRate = decay * (1.0f / SECONDSPERCVRENDER);
                decayCoef = calcCoef(decayRate, targetRatioDR);
                decayBase = (sustain - targetRatioDR) * (1.0f - decayCoef);

                level = decayBase + level * decayCoef;

                if (level <= sustain) {
                    level = sustain;
                    setStatusSustain(adsr, voice);
                }

                if (gate == 0) {
                    setStatusRelease(adsr, voice);
                }

                // level = bezier1D(1.0f, fast_lerp_f32(1.0f, sustain, shape), sustain, currentTime);
                break;

            case adsr.SUSTAIN:

                if (gate == 0) {
                    setStatusRelease(adsr, voice);
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

                releaseRate = release * (1.0f / SECONDSPERCVRENDER);
                releaseCoef = calcCoef(releaseRate, targetRatioDR);
                releaseBase = -targetRatioDR * (1.0f - releaseCoef);

                level = releaseBase + level * releaseCoef;

                if (level <= ADSRTHRESHOLD) {
                    level = 0;
                    if (loop == 0)
                        setStatusOff(adsr, voice);
                    else
                        setStatusDelay(adsr, voice);
                }

                if (gate == 1) {
                    setStatusAttack(adsr, voice);
                }

                break;
            default: Error_Handler(); break;
        }
    }

    adsr.level = clamp(adsr.level, 0.0f, 1.0f);

    // midi velocity
    adsr.level = fast_lerp_f32(adsr.level, adsr.level * layerA.midi.oVeloctiy, adsr.aVelocity);

    // keytrack
    adsr.level = fast_lerp_f32(adsr.level, adsr.level * layerA.midi.oNote, adsr.aKeytrack);

    adsr.out = adsr.level * adsr.aAmount;
}

#endif