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
    const float &shape = 1.0f - adsr.aShape;

    float exp;

    if (shape > 1.0f)
        exp = shape;
    else
        exp = 1.0f / (1.5f - (shape / 2.0f));

    adsr.sustain = accumulateSustain(adsr);

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {

        // float &nextSample = level[voice];
        float &level = adsr.level[voice];
        float &currentTime = adsr.currentTime[voice];
        const float &gate = layerA.midi.oGate[voice];
        const float &sustain = adsr.sustain[voice];

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
                level = std::max(ADSRTHRESHOLD, level);
                level += (exp * powf(level * SECONDSPERCVRENDER / attack, exp) / level);

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
                level -= (exp * (1.0f - sustain) * (powf(level * SECONDSPERCVRENDER / decay, exp)) / level);

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

                level = std::min(0.9999f, level);

                level -= (exp * sustain * powf(level * SECONDSPERCVRENDER / release, exp) / level);

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