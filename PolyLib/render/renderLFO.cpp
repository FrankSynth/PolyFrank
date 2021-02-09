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

inline float calcSquare(float phase) {
    return phase < 0.5f ? -1 : 1;
}

inline float calcRandom() {
    uint32_t randomNumber;

    randomNumber = std::rand();
    // map number between -1 and 1 float
    return ((float)((double)randomNumber / (double)RAND_MAX)) * 2 - 1;
}

inline float accumulateSpeed(LFO &lfo, uint16_t voice) {
    return testFloat(lfo.iFreq.currentSample[voice] * lfo.aFreq.valueMapped + lfo.aFreq.valueMapped, lfo.aFreq.min,
                     lfo.aFreq.max);
}

void renderLFO(LFO &lfo) {

    static bool alignedRandom = false;
    float &shape = lfo.aShape.valueMapped;
    int32_t &alignLFOs = lfo.dAlignLFOs.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        float &phase = lfo.currentTime[voice];
        float speed = accumulateSpeed(lfo, voice);
        float &nextSample = lfo.out.nextSample[voice];
        float &currentSample = lfo.out.currentSample[voice];
        bool &newPhase = lfo.newPhase[voice];

        phase += speed * secondsPerCVRender;
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
                nextSample = calcRandom();
            }
            else
                nextSample = currentSample;
        }
        // check if all voices should output the same LFO
        if (alignLFOs) {
            for (uint16_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++) {
                lfo.out.nextSample[otherVoice] = nextSample;
                lfo.newPhase[otherVoice] = newPhase;
            }
            break;
        }
    }
}

#endif
