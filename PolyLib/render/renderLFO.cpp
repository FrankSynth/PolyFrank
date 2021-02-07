#include "renderLFO.hpp"
#include "renderCV.hpp"
#include "rng.h"

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

inline float calcSquare(float phase) {
    return phase < 0.5f ? -1 : 1;
}

// TODO random number
inline float calcRandom() {
    uint32_t randomNumber;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber) != HAL_OK) {
        /* Random number generation error */
        Error_Handler();
    }
    // map number between 0 and 1 float
    return (float)randomNumber / 4294967295.0f;
}

inline float accumulateSpeed(LFO &lfo, uint16_t voice) {
    return testFloat(lfo.iFreq.currentSample[voice] * lfo.aFreq.valueMapped + lfo.aFreq.valueMapped, lfo.aFreq.min,
                     lfo.aFreq.max);
}

void renderLFO(LFO &lfo) {

    float &shape = lfo.aShape.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        float &phase = lfo.currentTime[voice];
        float speed = accumulateSpeed(lfo, voice);
        float &nextSample = lfo.out.nextSample[voice];
        float &currentSample = lfo.out.currentSample[voice];
        bool &newPhase = lfo.newPhase[voice];

        phase += (1 / speed) / secondsPerCVRender;
        if (phase > 1) {
            phase -= 1;
            newPhase = true;
        }
        else {
            newPhase = false;
        }

        if (shape < 1) {
            nextSample = calcSin(phase);
        }
        else if (shape < 2) {
            nextSample = calcRamp(phase);
        }
        else if (shape < 3) {
            nextSample = calcInvRamp(phase);
        }
        else if (shape < 4) {
            nextSample = calcTriangle(phase);
        }
        else if (shape < 5) {
            nextSample = calcSquare(phase);
        }
        else {
            if (newPhase)
                nextSample = calcRandom();
            else
                nextSample = currentSample;
        }
    }
}
