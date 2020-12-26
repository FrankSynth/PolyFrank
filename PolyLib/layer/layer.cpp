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

    // Layer specific settings, not part of modules
    layerSettings.id = modID.getNewId();
    ID settID;
    for (Setting *i : layerSettings.getSettings()) {
        i->id = settID.getNewId();
        i->moduleId = layerSettings.id;
        i->layerId = this->id;
    }
}

void Layer::addPatchInOut(Output &sourceOut, Input &targetIn, float amount) {
    for (PatchElementInOut *p : sourceOut.getPatchesInOut()) {
        if (p->targetIn == &targetIn)
            return;
    }

    patchesInOut.push_back(PatchElementInOut(sourceOut, targetIn, amount));
    sourceOut.addPatchInOut(patchesInOut.back());
    targetIn.addPatchInOut(patchesInOut.back());

#ifdef POLYCONTROL
    sendCreatePatchInOut(id, patchesInOut.back().sourceOut->idGlobal, patchesInOut.back().targetIn->idGlobal, amount);
#endif
}

void Layer::addPatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    addPatchInOut(*outputs[outputId], *inputs[inputId]);
}

void Layer::updatePatchInOut(PatchElementInOut &patch, float amount) {
    patch.setAmount(amount);
#ifdef POLYCONTROL
    sendUpdatePatchInOut(id, patch.sourceOut->idGlobal, patch.targetIn->idGlobal, amount);
#endif
}

void Layer::updatePatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    for (PatchElementInOut *p : outputs[outputId]->getPatchesInOut()) {
        if (p->targetIn == inputs[inputId]) {
            p->setAmount(amount);
#ifdef POLYCONTROL

            sendUpdatePatchInOut(id, p->sourceOut->idGlobal, p->targetIn->idGlobal, amount);
#endif
            return;
        }
    }
}

void Layer::removePatchInOut(PatchElementInOut &patch) {
#ifdef POLYCONTROL
    sendDeletePatchInOut(id, patch.sourceOut->idGlobal, patch.targetIn->idGlobal);
#endif

    patch.sourceOut->removePatchInOut(patch); // remove sourceOut entry
    patch.targetIn->removePatchInOut(patch);  // remove targetIn entry

    patch.remove = true;                                                  // set remove flag
    patchesInOut.remove_if([](PatchElementInOut n) { return n.remove; }); // find element and remove from list
}

void Layer::removePatchInOut(uint8_t outputId, uint8_t inputId) {
    for (PatchElementInOut *p : outputs[outputId]->getPatchesInOut()) {
        if (p->targetIn == inputs[inputId]) {
            removePatchInOut(*p);
            return;
        }
    }
}

// OutOut

void Layer::addPatchOutOut(Output &sourceOut, Output &targetOut, float amount, float offset) {
    for (PatchElementOutOut *p : sourceOut.getPatchesOutOut()) {
        if (p->targetOut == &targetOut)
            return;
    }

    patchesOutOut.push_back(PatchElementOutOut(sourceOut, targetOut, amount, offset));
    sourceOut.addPatchOutOut(patchesOutOut.back());
    targetOut.addPatchOutOut(patchesOutOut.back());

#ifdef POLYCONTROL
    sendCreatePatchOutOut(id, patchesOutOut.back().sourceOut->idGlobal, patchesOutOut.back().targetOut->idGlobal,
                          amount, offset);
#endif
}

void Layer::addPatchOutOut(uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
    addPatchOutOut(*outputs[outputOutId], *outputs[outputInId]);
}

void Layer::updatePatchOutOut(PatchElementOutOut &patch, float amount, float offset) {
    patch.setAmount(amount);
    patch.setOffset(offset);

#ifdef POLYCONTROL
    sendUpdatePatchOutOut(id, patch.sourceOut->idGlobal, patch.targetOut->idGlobal, amount, offset);
#endif
}

void Layer::updatePatchOutOut(uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
    for (PatchElementOutOut *p : outputs[outputOutId]->getPatchesOutOut()) {
        if (p->targetOut == outputs[outputInId]) {
            p->setAmount(amount);
            p->setOffset(offset);
#ifdef POLYCONTROL
            sendUpdatePatchOutOut(id, p->sourceOut->idGlobal, p->targetOut->idGlobal, amount, offset);
#endif
            return;
        }
    }
}

void Layer::removePatchOutOut(PatchElementOutOut &patch) {
#ifdef POLYCONTROL
    sendDeletePatchOutOut(id, patch.sourceOut->idGlobal, patch.targetOut->idGlobal);
#endif

    patch.sourceOut->removePatchOutOut(patch); // remove sourceOut entry
    patch.targetOut->removePatchOutOut(patch); // remove targetIn entry

    patch.remove = true;                                                    // set remove flag
    patchesOutOut.remove_if([](PatchElementOutOut n) { return n.remove; }); // find element and remove from list
}

void Layer::removePatchOutOut(uint8_t outputOutId, uint8_t outputInId) {
    for (PatchElementOutOut *p : outputs[outputOutId]->getPatchesOutOut()) {
        if (p->targetOut == outputs[outputInId]) {
            removePatchOutOut(*p);
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
    patchesOutOut.clear();

#ifdef POLYCONTROL
    sendDeleteAllPatches(id);
#endif
}