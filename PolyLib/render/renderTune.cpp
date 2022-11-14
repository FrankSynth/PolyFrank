#ifdef POLYRENDER

#include "renderTune.hpp"

extern Layer layerA;

void renderTune(Tune &tune) {

    // TODO log fuer blend noeting, wie im panning?

    // vec<VOICESPERCHIP> tune = accumulateLevel(steiner);

    tune.tuneS[0] = tune.getAnalog()[0 + layerA.chipID * 4]->valueMapped;
    tune.tuneS[1] = tune.getAnalog()[1 + layerA.chipID * 4]->valueMapped;
    tune.tuneS[2] = tune.getAnalog()[2 + layerA.chipID * 4]->valueMapped;
    tune.tuneS[3] = tune.getAnalog()[3 + layerA.chipID * 4]->valueMapped;

    tune.tuneL[0] = tune.getAnalog()[8 + 0 + layerA.chipID * 4]->valueMapped;
    tune.tuneL[1] = tune.getAnalog()[8 + 1 + layerA.chipID * 4]->valueMapped;
    tune.tuneL[2] = tune.getAnalog()[8 + 2 + layerA.chipID * 4]->valueMapped;
    tune.tuneL[3] = tune.getAnalog()[8 + 3 + layerA.chipID * 4]->valueMapped;
}

#endif