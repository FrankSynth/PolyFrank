#include "layer.hpp"

#ifdef POLYCONTROL
extern uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
extern uint8_t sendDeleteAllPatches(uint8_t layerId);
#endif

// uint32_t LayerRenBufferSw = 0;

void Layer::initID() {
    ID modID;
    ID outputID;
    ID inputID;

    for (BaseModule *m : modules) { // for all modules
        m->id = modID.getNewId();
        m->layerId = this->id;
        ID digitalID;
        ID analogID;
        ID renderBufferID;

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
        for (Input *i : m->getInputs()) { // for all Inputs
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

        for (RenderBuffer *r : m->getRenderBuffer()) { // for all Renderbuffer
            r->id = renderBufferID.getNewId();
            r->moduleId = m->id;
            r->layerId = this->id;
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

    envF.aShape.setValue(MAX_VALUE_12BIT);

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

#ifdef POLYCONTROL
    sendCreatePatchInOut(id, patchesInOut.back().sourceOut->idGlobal, patchesInOut.back().targetIn->idGlobal, 0);
#endif
    patchesInOut.back().setAmount(amount);
}

void Layer::addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount) {
    addPatchInOut(*(outputs[outputId]), *(inputs[inputId]), amount);
}

void Layer::updatePatchInOutWithoutMapping(PatchElement &patch, float amount) {
    patch.setAmountWithoutMapping(amount);
}

void Layer::updatePatchInOutByIdWithoutMapping(uint8_t outputId, uint8_t inputId, float amount) {
    if (outputId >= outputs.size()) {
        println("wrong outputID");
        return;
    }

    if (inputId >= inputs.size()) {
        println("wrong inputID");
        return;
    }

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

#ifdef POLYCONTROL
    sendDeleteAllPatches(id);

    // TODO temp patches
    addPatchInOut(envA.out, out.iVCA, 1.0f);
    addPatchInOut(envF.out, steiner.iCutoff, 1.0f);
    addPatchInOut(envF.out, ladder.iCutoff, 1.0f);
#endif
}

#ifdef POLYRENDER

/**
 * @brief load spreading buffer and imperfection base
 *
 */
void Layer::initLayer() {

    // set spread values dependend on chip ID
    float maxVal = chipID ? -1.0f : 1.0f;
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        spreadValues[i] = maxVal / ((float)i + 1.0f);

    // load imperfection buffers
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        lfoImperfection[i] = calcRandom() * LFOIMPERFECTIONWEIGHT;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        adsrImperfection[i] = calcRandom() * ADSRIMPERFECTIONWEIGHT + 1;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++) {
        for (uint16_t o = 0; o < OSCPERVOICE; o++) {
            for (uint16_t x = 0; x < NOTEIMPERFECTIONBUFFERSIZE; x++) {
                noteImperfection[o][i][x] = calcRandom() * NOTEIMPERFECTIONWEIGHT;
            }
        }
    }
}

#endif

#ifdef POLYCONTROL

void Layer::saveLayerToPreset(presetStruct *preset, std::string firstName, std::string secondName,
                              std::string thirdName) {

    collectLayerConfiguration();
    writePresetBlock(preset, firstName + " " + secondName + " " + thirdName);
}

void Layer::collectLayerConfiguration() {

    // println("collectConfiguration");
    int32_t *buffer = (int32_t *)blockBuffer;
    uint32_t index = 0;

    for (BaseModule *m : modules) { //  all modules

        for (Analog *i : m->getPotis()) {
            buffer[index] = i->value;
            index++;
            // println("value:  ", buffer[index]);
        }
        for (Digital *i : m->getSwitches()) {
            buffer[index] = i->valueMapped;
            index++;
            // println("value:  ", buffer[index]);
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
