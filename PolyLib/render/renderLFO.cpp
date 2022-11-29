

#include "renderLFO.hpp"
#include "renderCV.hpp"

// for snapping /////////////////////

#define DOT 1.0f / 1.5f
#define TRI 3.0f / 2.0f

#define QUARTER 1.0f
#define EIGHT 2.0f * QUARTER
#define SIXTEENTH 2.0f * EIGHT
#define THIRTYTWOTH 2.0f * SIXTEENTH
#define SIXTYFOURTH 2.0f * THIRTYTWOTH
#define HALF QUARTER / 2.0f
#define FULL HALF / 2.0f
#define TWOFULL FULL / 2.0f
#define FOURFULL TWOFULL / 2.0f

const float multTime[23] = {
    FOURFULL,         TWOFULL *DOT,    FOURFULL *TRI,  TWOFULL,      FULL *DOT, TWOFULL *TRI,   FULL,
    HALF *DOT,        FULL *TRI,       HALF,           QUARTER *DOT, HALF *TRI, QUARTER,        EIGHT *DOT,
    QUARTER *TRI,     EIGHT,           SIXTEENTH *DOT, EIGHT *TRI,   SIXTEENTH, SIXTEENTH *TRI, THIRTYTWOTH,
    THIRTYTWOTH *TRI, SIXTYFOURTH *TRI};

/////////////////////////////////

inline float calcSin(float phase) {
    return fast_sin_f32(phase);
}

inline float calcRamp(float phase) {
    return phase * 2.0f - 1.0f;
}

inline float calcRampTriangle(float phase, float shape) {
    float midPoint = std::clamp((shape - 1.f) / 2.f, 0.000001f, 1.f); // prevent division by 0 in fastMap

    if (phase < midPoint)
        return fastMap(phase, 0.f, midPoint, -1.f, 1.f);
    else
        return fastMap(phase, midPoint, 1.f, 1.f, -1.f);
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
    if (shape < 6) {
        return phase < 0.5f ? 1.0f : -1.0f;
    }
    else {
        shape = (shape - 6.0f) * 0.995f; // smallest possible square
        return phase < (0.5f - shape / 2.0f) ? 1.0f : -1.0f;
    }
}
#ifdef POLYRENDER

extern Layer layerA;

LogCurve linlogMapping(64, 0.01);

inline vec<VOICESPERCHIP> accumulateSpeed(const LFO &lfo) {
    if (lfo.dFreqSnap == 0)
        return (lfo.iFreq + lfo.aFreq); // max 200 Hz

    return clamp(lfo.iFreq + lfo.aFreq, lfo.aFreq.min, lfo.aFreq.max);
}
inline vec<VOICESPERCHIP> accumulateShape(const LFO &lfo) {
    return clamp(lfo.iShape + lfo.aShape, lfo.aShape.min, lfo.aShape.max);
}
inline vec<VOICESPERCHIP> accumulateAmount(const LFO &lfo) {
    return clamp(lfo.iAmount + lfo.aAmount, lfo.aAmount.min, lfo.aAmount.max);
}

void renderLFO(LFO &lfo) {
    int32_t &alignLFOs = lfo.dAlignLFOs.valueMapped;
    vec<VOICESPERCHIP> &sampleRAW = lfo.currentSampleRAW;
    vec<VOICESPERCHIP> &currentRandom = lfo.currentRandom;
    vec<VOICESPERCHIP> &prevRandom = lfo.prevRandom;
    vec<VOICESPERCHIP> &phase = lfo.currentTime;
    bool *newPhase = lfo.newPhase;
    vec<VOICESPERCHIP> &shape = lfo.shape;
    vec<VOICESPERCHIP> &shapeRAW = lfo.shapeRAW;
    vec<VOICESPERCHIP> &speed = lfo.speed;
    vec<VOICESPERCHIP> &speedRAW = lfo.speedRAW;

    vec<VOICESPERCHIP> &amount = lfo.amount;
    vec<VOICESPERCHIP> fract;

    vec<VOICESPERCHIP> sample;

    shapeRAW = accumulateShape(lfo);
    speedRAW = accumulateSpeed(lfo);
    amount = accumulateAmount(lfo);

    if (lfo.dFreqSnap == 0) {
        speed =
            linlogMapping.mapValue(speedRAW * (layerA.lfoImperfection * layerA.feel.aImperfection.valueMapped + 1.0f)) *
            100.0f;
    }
    else {

        for (int32_t voice = 0; voice < VOICESPERCHIP; voice++) {
            uint32_t snapper = std::round(speedRAW[voice] * lfo.dFreq.max);
            speed[voice] = layerA.bpm * multTime[snapper] / 60.0f;
            speedRAW[voice] = (float)snapper / (float)lfo.dFreq.max;
        }
    }

    shape = shapeRAW * 7;

    for (int32_t voice = 0; voice < VOICESPERCHIP; voice++)
        if (newPhase[voice] == false) {
            phase[voice] += speed[voice] * SECONDSPERCVRENDER;
            newPhase[voice] = phase[voice] > 1.0f;
            phase[voice] -= std::floor(phase[voice]);
            if (alignLFOs)
                break;
        }

    fract = shape - floor(shape);

    for (int32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (shape[voice] < 1) {
            sample[voice] = faster_lerp_f32(calcSin(phase[voice]), calcInvRamp(phase[voice]), fract[voice]);
        }
        else if (shape[voice] < 3) {
            sample[voice] = calcRampTriangle(phase[voice], shape[voice]);
        }

        else {
            if (newPhase[voice]) {
                prevRandom[voice] = currentRandom[voice];
                currentRandom[voice] = calcRandom();
            }

            if (shape[voice] < 4) {
                sample[voice] = faster_lerp_f32(calcRamp(phase[voice]),
                                                faster_lerp_f32(prevRandom[voice], currentRandom[voice], phase[voice]),
                                                fract[voice]);
            }

            else if (shape[voice] < 5) {
                float rndLerp = std::clamp(phase[voice] / (1.0f - fract[voice]), 0.0f, 1.0f);
                sample[voice] = faster_lerp_f32(prevRandom[voice], currentRandom[voice], rndLerp);
            }

            else if (shape[voice] < 6) {
                sample[voice] =
                    faster_lerp_f32(currentRandom[voice], calcSquare(phase[voice], shape[voice]), fract[voice]);
            }

            else {
                sample[voice] = calcSquare(phase[voice], shape[voice]);
            }
        }

        if (alignLFOs)
            break;
    }

    for (int32_t voice = 0; voice < VOICESPERCHIP; voice++)
        newPhase[voice] = false;

    if (layerA.chipID == 0) { // chip A
        lfo.alignFloatBuffer = sample[0];
        lfo.alignPhaseBuffer = phase[0];
    }
    else if (alignLFOs) { // chip B
        sample[0] = lfo.alignFloatBuffer;
        phase[0] = lfo.alignPhaseBuffer;
    }

    // check if all voices should output the same LFO
    for (int32_t otherVoice = 1; otherVoice < VOICESPERCHIP; otherVoice++) {
        sample[otherVoice] = sample[0] * alignLFOs + sample[otherVoice] * !alignLFOs;
        phase[otherVoice] = phase[0] * alignLFOs + phase[otherVoice] * !alignLFOs;
        lfo.newPhase[otherVoice] = newPhase[0] * alignLFOs + newPhase[otherVoice] * !alignLFOs;
    }

    for (int32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        sampleRAW[voice] = sample[voice];
        sample[voice] = sample[voice] * amount[voice];
    }

    lfo.out = sample;
}

#endif
