#ifdef POLYRENDER

#include "renderImperfection.hpp"

extern Layer layerA;

inline float calcRandom() {
    uint32_t randomNumber;

    randomNumber = std::rand();
    randomNumber = randomNumber & 0x00FFFFFF;

    // map to -1, 1
    return ((float)randomNumber / 8388607.0f) - 1.0f;
}
void renderImperfection(Imperfection imperfect) {

    static float spread[VOICESPERCHIP] = {calcRandom(), calcRandom(), calcRandom(), calcRandom()}; // temp
    static float drift[VOICESPERCHIP] = {calcRandom(), calcRandom(), calcRandom(), calcRandom()};  // temp

    float *SpreadNextSample = imperfect.oSpread.nextSample;
    float *DriftNextSample = imperfect.oDrift.nextSample;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        SpreadNextSample[i] = spread[i] * layerA.imperfect.aSpread.valueMapped;
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        DriftNextSample[i] =
            1 * layerA.imperfect.aDrift
                    .valueMapped; // temporary , vll ein sehr langsamer sinus (30minuten oder so?)  schicken
}

#endif