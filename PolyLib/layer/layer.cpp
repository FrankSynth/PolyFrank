#include "layer.hpp"

// std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> Layer::sendCreatePatchInOut = nullptr;
// std::function<uint8_t(uint8_t, uint8_t, uint8_t)> Layer::sendDeletePatchInOut = nullptr;
// std::function<uint8_t(uint8_t)> Layer::sendDeleteAllPatches = nullptr;

void Layer::initID() {
    ID modID;
    ID outputID;
    ID inputID;

    for (BaseModule *m : modules) { // for all modules
        m->id = modID.getNewId();
        m->layerId = this->id;
        ID digitalID;
        ID analogID;

        ID inputLocalID;
        ID outputLocalID;

        for (Analog *i : m->getPotis()) { // for all Knobs
            i->id = analogID.getNewId();
            i->moduleId = m->id;
            i->layerId = this->id;
        }
        for (Digital *i : m->getSwitches()) { // for all Knobs
            i->id = digitalID.getNewId();
            i->moduleId = m->id;
            i->layerId = this->id;
        }

        // collect Inputs
        for (Input *i : m->getInputs()) { // for all Output
            inputs.push_back(i);
            i->id = inputLocalID.getNewId();
            i->idGlobal = inputID.getNewId();
            i->moduleId = m->id;
            i->layerId = this->id;
        }
        // collect Outputs
        for (Output *o : m->getOutputs()) { // for all Output
            outputs.push_back(o);
            o->id = outputLocalID.getNewId();
            o->idGlobal = outputID.getNewId();
            o->moduleId = m->id;
            o->layerId = this->id;
        }
    }
}

void Layer::resetLayer() {

    for (BaseModule *m : modules) { // for all modules

        for (Analog *i : m->getPotis()) { // for all Knobs
            i->resetValue();
        }
        for (Digital *i : m->getSwitches()) { // for all Knobs
            i->resetValue();
        }
        // Layer specific settings, not part of modules
    }
    clearPatches();

#ifdef POLYRENDER
    allGatesOff();
    envA.resetAllADSRs();
    envF.resetAllADSRs();
    lfoA.resetAllPhases();
    lfoB.resetAllPhases();
#endif
}

void Layer::addPatchInOut(Output &sourceOut, Input &targetIn, float amount) {
    for (PatchElement *p : sourceOut.getPatchesInOut()) {
        if (p->targetIn == &targetIn)
            return;
    }

    patchesInOut.push_back(PatchElement(sourceOut, targetIn, id));
    sourceOut.addPatchInOut(patchesInOut.back());
    targetIn.addPatchInOut(patchesInOut.back());
    patchesInOut.back().setAmount(0.2);

#ifdef POLYCONTROL
    sendCreatePatchInOut(id, patchesInOut.back().sourceOut->idGlobal, patchesInOut.back().targetIn->idGlobal, amount);
#endif
}

void Layer::addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount) {
    addPatchInOut(*(outputs[outputId]), *(inputs[inputId]), amount);
}

void Layer::updatePatchInOutWithoutMapping(PatchElement &patch, float amount) {
    patch.setAmountWithoutMapping(amount);
}

void Layer::updatePatchInOutByIdWithoutMapping(uint8_t outputId, uint8_t inputId, float amount) {
    for (PatchElement *p : outputs[outputId]->getPatchesInOut()) {
        if (p->targetIn == inputs[inputId]) {
            this->updatePatchInOutWithoutMapping(*p, amount);
            return;
        }
    }
}

void Layer::removePatchInOut(PatchElement &patch) {
#ifdef POLYCONTROL
    sendDeletePatchInOut(id, patch.sourceOut->idGlobal, patch.targetIn->idGlobal);
#endif

    patch.sourceOut->removePatchInOut(patch); // remove sourceOut entry
    patch.targetIn->removePatchInOut(patch);  // remove targetIn entry

    patch.remove = true;                                             // set remove flag
    patchesInOut.remove_if([](PatchElement n) { return n.remove; }); // find element and remove from list
}

void Layer::removePatchInOutById(uint8_t outputId, uint8_t inputId) {
    for (PatchElement *p : outputs[outputId]->getPatchesInOut()) {
        if (p->targetIn == inputs[inputId]) {
            removePatchInOut(*p);
            return;
        }
    }
}

// OutOut

// void Layer::addPatchOutOut(Output &sourceOut, Output &targetOut, float amount, float offset) {
//     for (PatchElementOutOut *p : sourceOut.getPatchesOutOut()) {
//         if (p->targetOut == &targetOut)
//             return;
//     }

//     patchesOutOut.push_back(PatchElementOutOut(sourceOut, targetOut, id, amount, offset));
//     sourceOut.addPatchOutOut(patchesOutOut.back());
//     targetOut.addPatchOutOut(patchesOutOut.back());

// #ifdef POLYCONTROL
//     sendCreatePatchOutOut(id, patchesOutOut.back().sourceOut->idGlobal, patchesOutOut.back().targetOut->idGlobal,
//                           amount, offset);
// #endif
// }

// void Layer::addPatchOutOutById(uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
//     addPatchOutOut(*outputs[outputOutId], *outputs[outputInId], amount);
// }

// void Layer::updatePatchOutOutWithoutMapping(PatchElementOutOut &patch, float amount, float offset) {
//     patch.setAmountWithoutMapping(amount);
//     patch.setOffsetWithoutMapping(amount);
// }

// void Layer::updatePatchOutOutByIdWithoutMapping(uint8_t outputOutId, uint8_t outputInId, float amount, float offset)
// {
//     for (PatchElementOutOut *p : outputs[outputOutId]->getPatchesOutOut()) {
//         if (p->targetOut == outputs[outputInId]) {
//             updatePatchOutOutWithoutMapping(*p, amount, offset);
//             return;
//         }
//     }
// }

// void Layer::removePatchOutOut(PatchElementOutOut &patch) {
// #ifdef POLYCONTROL
//     sendDeletePatchOutOut(id, patch.sourceOut->idGlobal, patch.targetOut->idGlobal);
// #endif

//     patch.sourceOut->removePatchOutOut(patch); // remove sourceOut entry
//     patch.targetOut->removePatchOutOut(patch); // remove targetIn entry

//     patch.remove = true;                                                    // set remove flag
//     patchesOutOut.remove_if([](PatchElementOutOut n) { return n.remove; }); // find element and remove from list
// }

// void Layer::removePatchOutOutById(uint8_t outputOutId, uint8_t outputInId) {
//     for (PatchElementOutOut *p : outputs[outputOutId]->getPatchesOutOut()) {
//         if (p->targetOut == outputs[outputInId]) {
//             removePatchOutOut(*p);
//             return;
//         }
//     }
// }

void Layer::clearPatches() {
    for (BaseModule *m : modules) {         // for all modules
        for (Output *o : m->getOutputs()) { // for all Output
            o->clearPatches();
        }
        for (Input *i : m->getInputs()) { // for all Output
            i->clearPatches();
        }
    }
    patchesInOut.clear();
    // patchesOutOut.clear();

#ifdef POLYCONTROL
    sendDeleteAllPatches(id);
#endif
}
#ifdef POLYCONTROL

void Layer::saveLayerToPreset(presetStruct *preset, std::string firstName, std::string secondName,
                              std::string thirdName) {

    collectLayerConfiguration();
    writePresetBlock(preset, firstName + " " + secondName + " " + thirdName);
}

void Layer::collectLayerConfiguration() {
    int32_t *buffer = (int32_t *)blockBuffer;
    uint32_t index = 0;

    for (BaseModule *m : modules) { //  all modules

        for (Analog *i : m->getPotis()) {
            buffer[index] = i->value;
            index++;
        }
        for (Digital *i : m->getSwitches()) {
            buffer[index] = i->valueMapped;
            index++;
        }
    }

    buffer[index] = patchesInOut.size();
    index++;

    patchSaveStruct *bufferPatch;
    bufferPatch = (patchSaveStruct *)&(buffer[index]);
    index = 0;
    patchSaveStruct patch;
    // save Patches
    for (PatchElement p : patchesInOut) {

        patch.sourceID = p.sourceOut->idGlobal;
        patch.targetID = p.targetIn->idGlobal;
        patch.amount = p.amount;

        bufferPatch[index] = patch;

        index++;
    }

    if ((uint32_t)((uint8_t *)&bufferPatch[index] - (uint8_t *)blockBuffer) > (PRESET_BLOCKSIZE)) {
        PolyError_Handler("ERROR | FATAL | LAYER -> SaveLayerToPreset -> BufferOverflow!");
    }
}
void Layer::loadLayerFromPreset(presetStruct *preset) {
    if (preset->usageState != PRESET_USED) {
        return;
    }

    int32_t *buffer = (int32_t *)readPreset(preset);

    uint32_t index = 0;
    for (BaseModule *m : modules) { //  all modules

        for (Analog *i : m->getPotis()) {
            i->setValue(buffer[index]);
            index++;
        }
        for (Digital *i : m->getSwitches()) {
            i->setValueWithoutMapping(buffer[index]);
            index++;
        }
    }

    // Layer specific settings, not part of modules
    // for (Setting *i : layerSettings.getSettings()) {
    //     i->setValue(buffer[index]);
    //     index++;
    // }

    // clear existing patches
    clearPatches();

    // read number of patches

    int32_t numberPatchesInOut = buffer[index];
    index++;

    patchSaveStruct *bufferPatch;
    bufferPatch = (patchSaveStruct *)&(buffer[index]);
    index = 0;
    patchSaveStruct patch;

    // load Patches
    for (int i = 0; i < numberPatchesInOut; i++) {

        patch = bufferPatch[index];
        addPatchInOutById(patch.sourceID, patch.targetID, patch.amount);
        index++;
    }

    // for (int i = 0; i < numberPatchesOutOut; i++) {

    //     patch = bufferPatch[index];
    //     addPatchOutOutById(patch.sourceID, patch.targetID, patch.amount);
    //     index++;
    // }
}

#endif
