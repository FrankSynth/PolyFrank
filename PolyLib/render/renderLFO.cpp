#ifdef POLYRENDER

#include "renderLFO.hpp"
#include "renderCV.hpp"

extern Layer layerA;

LogCurve linlogMapping(32, 0.01);

inline float calcSin(float phase) {
    return fast_sin_f32(phase);
}

inline float calcRamp(float phase) {
    return phase * 2.0f - 1.0f;
}

inline float calcInvRamp(float phase) {
    return calcRamp(phase) * -1.0f;
}

inline float calcTriangle(float phase) {
    if (phase < 0.5f) {
        return calcRamp(phase * 2.0f);
    }
    else {
        return calcInvRamp((phase - 0.5f) * 2.0f);
    }
}

inline float calcSquare(float phase, float shape) {
    if (shape < 4) {
        return phase < 0.5f ? 1.0f : -1.0f;
    }
    else if (shape < 5) {
        shape -= floor(shape);
        return phase < (0.5f - shape / 2.0f) ? 1.0f : -1.0f;
    }
    else {
        return -1.0f;
    }
}

inline float accumulateSpeed(LFO &lfo, uint16_t voice) {
    return linlogMapping.mapValue((lfo.iFreq.currentSample[voice] + lfo.aFreq.valueMapped)) * 100; // max 200 Hz
    // return std::clamp(lfo.iFreq.currentSample[voice] + lfo.aFreq.valueMapped, lfo.aFreq.min, lfo.aFreq.max);
}
inline float accumulateShape(LFO &lfo, uint16_t voice) {
    return std::clamp(lfo.iShape.currentSample[voice] + lfo.aShape.valueMapped, lfo.aShape.min, lfo.aShape.max);
}
inline float accumulateAmount(LFO &lfo, uint16_t voice) {
    return std::clamp(lfo.iAmount.currentSample[voice] + lfo.aAmount.valueMapped, lfo.aAmount.min, lfo.aAmount.max);
}

void renderLFO(LFO &lfo) {

    int32_t &alignLFOs = lfo.dAlignLFOs.valueMapped;
    float *currentRandom = lfo.currentRandom;
    float *phase = lfo.currentTime;
    bool *newPhase = lfo.newPhase;
    float shape[VOICESPERCHIP];
    float speed[VOICESPERCHIP];
    float amount[VOICESPERCHIP];
    float fract[VOICESPERCHIP];
    float *nextSample = lfo.out.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        shape[voice] = accumulateShape(lfo, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        speed[voice] =
            accumulateSpeed(lfo, voice) * (1 + layerA.lfoImperfection[voice] * layerA.feel.aImperfection.valueMapped);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        amount[voice] = accumulateAmount(lfo, voice);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        if (newPhase[voice] == false) {
            phase[voice] += speed[voice] * SECONDSPERCVRENDER;
            newPhase[voice] = phase[voice] > 1.0f;
            phase[voice] -= std::floor(phase[voice]);
        }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        fract[voice] = shape[voice] - std::floor(shape[voice]);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (shape[voice] < 1) {
            nextSample[voice] = fast_lerp_f32(calcSin(phase[voice]), calcRamp(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 2) {
            nextSample[voice] = fast_lerp_f32(calcRamp(phase[voice]), calcTriangle(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 3) {
            nextSample[voice] = fast_lerp_f32(calcTriangle(phase[voice]), calcInvRamp(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 4) {
            nextSample[voice] =
                fast_lerp_f32(calcInvRamp(phase[voice]), calcSquare(phase[voice], shape[voice]), fract[voice]);
        }
        else if (shape[voice] < 5) {
            nextSample[voice] = calcSquare(phase[voice], shape[voice]);
        }
        else {
            float random;
            if (newPhase[voice]) {
                if (voice == 0) {
                    lfo.alignedRandom = false;
                    // re-seed once when they should be aligned
                    if (alignLFOs && lfo.alignedRandom == false) {
                        std::srand(lfo.randSeed++);
                        lfo.alignedRandom = true;
                    }
                    else {
                        lfo.alignedRandom = false;
                    }
                }
                random = calcRandom();
                currentRandom[voice] = random;
            }
            else
                random = currentRandom[voice];
            if (shape[voice] == 6.0f)
                nextSample[voice] = random;
            else
                nextSample[voice] = fast_lerp_f32(-1.0f, random, fract[voice]);
        }

        if (alignLFOs)
            break;
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        newPhase[voice] = false;

    // check if all voices should output the same LFO
    if (alignLFOs) {
        for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++)
            lfo.out.nextSample[otherVoice] = nextSample[0];
        for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++)
            lfo.newPhase[otherVoice] = newPhase[0];
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        nextSample[voice] = nextSample[voice] * amount[voice];
}

#endif
