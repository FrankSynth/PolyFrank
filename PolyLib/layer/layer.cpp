#include "layer.hpp"

// std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> Layer::sendCreatePatchInOut = nullptr;
// std::function<uint8_t(uint8_t, uint8_t, uint8_t)> Layer::sendDeletePatchInOut = nullptr;
// std::function<uint8_t(uint8_t)> Layer::sendDeleteAllPatches = nullptr;

bool Layer::sendData = false;

void Layer::initID() {
    ID modID;
    ID outputID;
    ID inputID;

    for (BaseModule *m : modules) { // for all modules
        m->id = modID.getNewId();
        m->layerId = this->id;
        ID settID;
        for (Analog *i : m->getPotis()) { // for all Knobs
            i->id = settID.getNewId();
            i->moduleId = m->id;
            i->layerId = this->id;
        }
        for (Digital *i : m->getSwitches()) { // for all Knobs
            i->id = settID.getNewId();
            i->moduleId = m->id;
            i->layerId = this->id;
        }

        // collect Inputs
        for (Input *i : m->getInputs()) { // for all Output
            inputs.push_back(i);
            i->id = inputID.getNewId();
            i->moduleId = m->id;
            i->layerId = this->id;
        }
        // collect Outputs
        for (Output *o : m->getOutputs()) { // for all Output
            outputs.push_back(o);
            o->id = outputID.getNewId();
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

    if (sendData) {
        sendCreatePatchInOut(id, patchesInOut.back().sourceOut->id, patchesInOut.back().targetIn->id, amount);
    }
}

void Layer::addPatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    addPatchInOut(*outputs[outputId], *inputs[inputId]);
}

void Layer::updatePatchInOut(PatchElementInOut &patch, float amount) {
    patch.setAmount(amount);
    if (sendData)
        sendUpdatePatchInOut(id, patch.sourceOut->id, patch.targetIn->id, amount);
}

void Layer::updatePatchInOut(uint8_t outputId, uint8_t inputId, float amount) {
    for (PatchElementInOut *p : outputs[outputId]->getPatchesInOut()) {
        if (p->targetIn == inputs[inputId]) {
            p->setAmount(amount);
            if (sendData)
                sendUpdatePatchInOut(id, p->sourceOut->id, p->targetIn->id, amount);
            return;
        }
    }
}

void Layer::removePatchInOut(PatchElementInOut &patch) {
    if (sendData) {
        sendDeletePatchInOut(id, patch.sourceOut->id, patch.targetIn->id);
    }

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

    if (sendData) {
        sendCreatePatchOutOut(id, patchesOutOut.back().sourceOut->id, patchesOutOut.back().targetOut->id, amount,
                              offset);
    }
}

void Layer::addPatchOutOut(uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
    addPatchOutOut(*outputs[outputOutId], *outputs[outputInId]);
}

void Layer::updatePatchOutOut(PatchElementOutOut &patch, float amount, float offset) {
    patch.setAmount(amount);
    patch.setOffset(offset);
    if (sendData)
        sendUpdatePatchOutOut(id, patch.sourceOut->id, patch.targetOut->id, amount, offset);
}

void Layer::updatePatchOutOut(uint8_t outputOutId, uint8_t outputInId, float amount, float offset) {
    for (PatchElementOutOut *p : outputs[outputOutId]->getPatchesOutOut()) {
        if (p->targetOut == outputs[outputInId]) {
            p->setAmount(amount);
            p->setOffset(offset);
            if (sendData)
                sendUpdatePatchOutOut(id, p->sourceOut->id, p->targetOut->id, amount, offset);
            return;
        }
    }
}

void Layer::removePatchOutOut(PatchElementOutOut &patch) {
    if (sendData) {
        sendDeletePatchOutOut(id, patch.sourceOut->id, patch.targetOut->id);
    }

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

    if (sendData) {
        sendDeleteAllPatches(id);
    }
}