#ifdef POLYRENDER

#include "renderLFO.hpp"
#include "renderCV.hpp"

inline float calcSin(float phase) {
    return fast_sin_f32(phase);
}

inline float calcRamp(float phase) {
    return phase * 2 - 1;
}

inline float calcInvRamp(float phase) {
    return calcRamp(phase) * -1;
}

inline float calcTriangle(float phase) {
    if (phase < 0.5f) {
        return calcRamp(phase * 2);
    }
    else {
        return calcInvRamp((phase - 0.5) * 2);
    }
}

inline float calcSquare(float phase, float shape) {
    if (shape < 4) {
        return phase < 0.5f ? 1 : -1;
    }
    else if (shape < 5) {
        shape -= floor(shape);
        return phase < (0.5f - shape / 2.0f) ? 1 : -1;
    }
    else {
        return -1;
    }
}

inline float calcRandom() {
    uint32_t randomNumber;

    randomNumber = std::rand();
    randomNumber = randomNumber & 0x00FFFFFF;

    // map to -1, 1
    return ((float)randomNumber / 8388607.0f) - 1.0f;
}

inline float accumulateSpeed(LFO &lfo, uint16_t voice) {
    return testFloat(lfo.iFreq.currentSample[voice] * lfo.aFreq.valueMapped + lfo.aFreq.valueMapped, lfo.aFreq.min,
                     lfo.aFreq.max);
}
inline float accumulateShape(LFO &lfo, uint16_t voice) {
    return testFloat(lfo.iShape.currentSample[voice] * lfo.aShape.valueMapped + lfo.aShape.valueMapped, lfo.aShape.min,
                     lfo.aShape.max);
}

void renderLFO(LFO &lfo) {

    static bool alignedRandom = false;
    int32_t &alignLFOs = lfo.dAlignLFOs.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        float &currentRandom = lfo.currentRandom[voice];
        float &phase = lfo.currentTime[voice];
        float shape = accumulateShape(lfo, voice);
        float speed = accumulateSpeed(lfo, voice);
        float &nextSample = lfo.out.nextSample[voice];
        // float &currentSample = lfo.out.currentSample[voice];
        bool &newPhase = lfo.newPhase[voice];

        if (newPhase == false) {
            phase += speed * secondsPerCVRender;
            if (phase > 1) {
                phase -= 1;
                newPhase = true;
            }
        }

        float fract = shape - std::floor(shape);

        if (shape < 1) {
            nextSample = fast_lerp_f32(calcSin(phase), calcRamp(phase), fract);
        }
        else if (shape < 2) {
            nextSample = fast_lerp_f32(calcRamp(phase), calcTriangle(phase), fract);
        }
        else if (shape < 3) {
            nextSample = fast_lerp_f32(calcTriangle(phase), calcInvRamp(phase), fract);
        }
        else if (shape < 4) {
            nextSample = fast_lerp_f32(calcInvRamp(phase), calcSquare(phase, shape), fract);
        }
        else if (shape < 5) {
            nextSample = calcSquare(phase, shape);
        }
        else {
            float random;
            if (newPhase) {
                if (voice == 0) {
                    // re-seed once when they should be aligned
                    if (alignLFOs && alignedRandom == false) {
                        static uint32_t randSeed = 1;
                        std::srand(randSeed++);
                        alignedRandom = true;
                    }
                    else {
                        alignedRandom = false;
                    }
                }
                random = calcRandom();
                currentRandom = random;
            }
            else
                random = currentRandom;
            if (shape == 6.0f)
                nextSample = random;
            else
                nextSample = fast_lerp_f32(-1.0f, random, fract);
        }

        // check if all voices should output the same LFO
        if (alignLFOs) {
            for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++)
                lfo.out.nextSample[otherVoice] = nextSample;
            for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++)
                lfo.newPhase[otherVoice] = newPhase;
            break;
        }

        newPhase = false;
    }
}

#endif
