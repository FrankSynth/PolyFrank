#ifdef POLYRENDER

#include "renderSub.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateShape() {
    return clamp(layerA.sub.iShape + layerA.sub.aShape, layerA.sub.aShape.min, layerA.sub.aShape.max);
}

void renderSub() {
    layerA.sub.shape = accumulateShape();
    layerA.sub.phaseLength = 0.5f * !layerA.sub.dOctaveSwitch + 0.25f * layerA.sub.dOctaveSwitch;

    layerA.sub.subWavetable =
        clamp(round((layerA.sub.oscANote - (layerA.sub.dOctaveSwitch + 1)) * ((float)SUBWAVETABLES / 10.0f)), 0.0f,
              (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor
}

#endif