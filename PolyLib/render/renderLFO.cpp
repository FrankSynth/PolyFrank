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
    return linlogMapping.mapValue((lfo.iFreq[voice] + lfo.aFreq)) * 100; // max 200 Hz
    // return std::clamp(lfo.iFreq.currentSample[voice] + lfo.aFreq.valueMapped, lfo.aFreq.min, lfo.aFreq.max);
}
inline vec<VOICESPERCHIP> accumulateShape(LFO &lfo) {
    return clamp((vec<VOICESPERCHIP> &)lfo.iShape + lfo.aShape, lfo.aShape.min, lfo.aShape.max);
}
inline vec<VOICESPERCHIP> accumulateAmount(LFO &lfo) {
    return clamp((vec<VOICESPERCHIP> &)lfo.iAmount + lfo.aAmount, lfo.aAmount.min, lfo.aAmount.max);
}

void renderLFO(LFO &lfo) {
    int32_t &alignLFOs = lfo.dAlignLFOs.valueMapped;
    vec<VOICESPERCHIP> &currentRandom = lfo.currentRandom;
    vec<VOICESPERCHIP> &phase = lfo.currentTime;
    bool *newPhase = lfo.newPhase;
    vec<VOICESPERCHIP> shape;
    vec<VOICESPERCHIP> speed;
    vec<VOICESPERCHIP> amount;
    vec<VOICESPERCHIP> fract;

    vec<VOICESPERCHIP> sample;

    shape = accumulateShape(lfo);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        speed[voice] = accumulateSpeed(lfo, voice) * (1 + layerA.lfoImperfection[voice] * layerA.feel.aImperfection);

    amount = accumulateAmount(lfo);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        if (newPhase[voice] == false) {
            phase[voice] += speed[voice] * SECONDSPERCVRENDER;
            newPhase[voice] = phase[voice] > 1.0f;
            phase[voice] -= std::floor(phase[voice]);
        }

    fract = shape - floor(shape);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (shape[voice] < 1) {
            sample[voice] = fast_lerp_f32(calcSin(phase[voice]), calcRamp(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 2) {
            sample[voice] = fast_lerp_f32(calcRamp(phase[voice]), calcTriangle(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 3) {
            sample[voice] = fast_lerp_f32(calcTriangle(phase[voice]), calcInvRamp(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 4) {
            sample[voice] =
                fast_lerp_f32(calcInvRamp(phase[voice]), calcSquare(phase[voice], shape[voice]), fract[voice]);
        }
        else if (shape[voice] < 5) {
            sample[voice] = calcSquare(phase[voice], shape[voice]);
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
                sample[voice] = random;
            else
                sample[voice] = fast_lerp_f32(-1.0f, random, fract[voice]);
        }

        if (alignLFOs)
            break;
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        newPhase[voice] = false;

    // check if all voices should output the same LFO
    if (alignLFOs) {
        for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++)
            sample[otherVoice] = sample[0];
        for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++)
            lfo.newPhase[otherVoice] = newPhase[0];
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        sample[voice] = sample[voice] * amount[voice];

    lfo.out = sample;
}

#endif
