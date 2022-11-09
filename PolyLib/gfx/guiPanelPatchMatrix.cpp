#ifdef POLYCONTROL

#include "GUIPanelPatchMatrix.hpp"

void GUIPanelPatchMatrix::registerElements() {

    Output *out;
    Analog *in;

    for (uint32_t x = 0; x < MATRIXCOLUMN; x++) {
        if (((int32_t)x + scrollOut.offset) < (int32_t)allOutputs.size()) {
            panelElementsOut[x].addEntry(allOutputs[x + scrollOut.offset]);
        }
    }

    for (uint32_t y = 0; y < PATCHMATRIXROWS; y++) {
        if (((int32_t)y + scrollIn.offset) < (int32_t)allInputs.size()) {
            in = allInputs[y + scrollIn.offset];
            panelElementsIn[y].addEntry(in);
            for (uint32_t x = 0; x < MATRIXCOLUMN; x++) {
                if (((int32_t)x + scrollOut.offset) < (int32_t)allOutputs.size()) {
                    out = allOutputs[x + scrollOut.offset];
                    panelElementsPatch[x][y].addEntry(nullptr);
                    for (uint32_t i = 0; i < in->input->getPatchesInOut().size(); i++) {
                        if (in->input->getPatchesInOut()[i]->sourceOut->idGlobal == out->idGlobal) {
                            panelElementsPatch[x][y].addEntry(in->input->getPatchesInOut()[i]);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (uint32_t y = 0; y < MATRIXROWS; y++) {
        if (((int32_t)y + scrollModule.offset) < (int32_t)allModules.size()) {
            panelElementsModule[y].addEntry(allModules[y + scrollModule.offset]);
        }
    }

    panelElementsOut[scrollOut.relPosition].select = 1;
    panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].select = 1;
    panelElementsIn[scrollIn.relPosition].select = 1;
    panelElementsModule[scrollModule.relPosition].select = 1;
}

void GUIPanelPatchMatrix::collectModules() {

    allModules.clear();

    for (BaseModule *module : allLayers[cachedFocus.layer]->getModules()) { // Collect Module if inputs available
        uint8_t check = 0;
        if (module->displayVis) {
            for (Input *input : module->inputs) {
                if (input->visible) {
                    if (filteredView) {
                        if (input->patchesInOut.size())
                            check = 1;
                    }
                    else {
                        check = 1;
                    }
                }
            }
        }
        if (check)
            allModules.push_back(module);
    }

    entrysModule = allModules.size();
    scrollModule.entrys = entrysModule;
    scrollModule.checkScroll();
}

void GUIPanelPatchMatrix::collectInputs() {

    allInputs.clear();

    if (allModules.size()) {
        for (Analog *analog : allModules[scrollModule.position]->knobs) {
            if (analog->input != nullptr) {
                if (analog->input->visible) {
                    if (filteredView) {
                        if (analog->input->patchesInOut.size())
                            allInputs.push_back(analog);
                    }
                    else {
                        allInputs.push_back(analog);
                    }
                }
            }
        }
    }
    entrysIn = allInputs.size();
    scrollIn.entrys = entrysIn;
    scrollIn.checkScroll();
}
void GUIPanelPatchMatrix::collectOutputs() {

    allOutputs.clear();

    if (viewMode == MIDIVIEW) {
        for (Output *output : allLayers[cachedFocus.layer]->outputs) { // Collect Module if inputs available
            ModuleType moduleType = allLayers[cachedFocus.layer]->modules[output->moduleId]->moduleType;
            if (moduleType == MODULE_MIDI) {
                if (output->visible) {
                    if (filteredView) {
                        if (output->patchesInOut.size())
                            allOutputs.push_back(output);
                    }
                    else {
                        allOutputs.push_back(output);
                    }
                }
            }
        }
    }
    else if (viewMode == ENVVIEW) {
        for (Output *output : allLayers[cachedFocus.layer]->outputs) { // Collect Module if inputs available
            ModuleType moduleType = allLayers[cachedFocus.layer]->modules[output->moduleId]->moduleType;
            if ((moduleType == MODULE_ADSR) | (moduleType == MODULE_LFO) | (moduleType == MODULE_FEEL)) {
                if (output->visible) {
                    if (filteredView) {
                        if (output->patchesInOut.size())
                            allOutputs.push_back(output);
                    }
                    else {
                        allOutputs.push_back(output);
                    }
                }
            }
        }
    }
    else if (viewMode == AUDIOVIEW) {
        for (Output *output : allLayers[cachedFocus.layer]->outputs) { // Collect Module if inputs available
            ModuleType moduleType = allLayers[cachedFocus.layer]->modules[output->moduleId]->moduleType;
            if ((moduleType == MODULE_OSC_A) | (moduleType == MODULE_OSC_B) | (moduleType == MODULE_NOISE)) {
                if (output->visible) {
                    if (filteredView) {
                        if (output->patchesInOut.size())
                            allOutputs.push_back(output);
                    }
                    else {
                        allOutputs.push_back(output);
                    }
                }
            }
        }
    }
    entrysOut = allOutputs.size();
    scrollOut.entrys = entrysOut;
    scrollOut.checkScroll();
}

void GUIPanelPatchMatrix::activate() {
    // find correc view
    ModuleType type = allLayers[cachedFocus.layer]->getModules()[cachedFocus.modul]->moduleType;

    if (cachedFocus.type == FOCUSOUTPUT) {
        if ((type == MODULE_ADSR) | (type == MODULE_LFO) | (type == MODULE_FEEL)) {
            setEnvView();
        }
        else if ((type == MODULE_OSC_A) | (type == MODULE_OSC_B) | (type == MODULE_SUB) | (type == MODULE_NOISE)) {
            setAudioView();
        }
        else if (type == MODULE_MIDI) {
            setMidiView();
        }
    }

    collectModules();

    if (cachedFocus.type == FOCUSINPUT || cachedFocus.type == FOCUSMODULE) {

        for (uint32_t i = 0; i < allModules.size(); i++) {
            if (allModules[i]->id == cachedFocus.modul) {
                scrollModule.setScroll(i);
                break;
            }
        }
        if (cachedFocus.type == FOCUSINPUT) {
            collectInputs();
            for (uint32_t i = 0; i < allInputs.size(); i++) {
                if (allInputs[i]->id == cachedFocus.id) {
                    scrollIn.setScroll(i);
                    break;
                }
            }

            if (selectedPatch != nullptr) {
                collectOutputs();
                for (uint32_t i = 0; i < allOutputs.size(); i++) {
                    if (allOutputs[i]->idGlobal == selectedPatch->sourceOut->idGlobal) {
                        scrollOut.setScroll(i);
                        break;
                    }
                }
            }
        }
    }

    if (cachedFocus.type == FOCUSOUTPUT) {

        if (allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->outputs.size()) {
            uint16_t searchID = allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->outputs[0]->idGlobal;

            collectOutputs();
            for (uint32_t i = 0; i < allOutputs.size(); i++) {
                if (allOutputs[i]->idGlobal == searchID) {
                    scrollOut.setScroll(i);
                    break;
                }
            }
            if (selectedPatch != nullptr) {

                for (uint32_t i = 0; i < allModules.size(); i++) {
                    if (allModules[i]->id == selectedPatch->targetIn->moduleId) {
                        scrollModule.setScroll(i);
                        break;
                    }
                }

                collectInputs();
                for (uint32_t i = 0; i < allInputs.size(); i++) {
                    if (allInputs[i]->input != nullptr) {
                        if (allInputs[i]->input->idGlobal == selectedPatch->targetIn->idGlobal) {
                            scrollIn.setScroll(i);
                            break;
                        }
                    }
                }
            }
        }
        else {
            scrollOut.setScroll(0);
        }
    }
}

// Change Focus with PatchPanel Scolling
void GUIPanelPatchMatrix::scrollModulePosition(int32_t scroll) {
    scrollModule.scroll(scroll);
    updateLocation = true;
}
void GUIPanelPatchMatrix::scrollInPosition(int32_t scroll) {
    scrollIn.scroll(scroll);
    updateLocation = true;
}
void GUIPanelPatchMatrix::scrollOutPosition(int32_t scroll) {
    scrollOut.scroll(scroll);
    updateLocation = true;
}

void GUIPanelPatchMatrix::Draw() {

    // update number ob entrys
    collectModules();
    collectInputs();
    collectOutputs();

    if (updateLocation) {
        updateLocation = false;

        currentFocus.modul = allModules[scrollModule.position]->id;

        if (allInputs.size()) {
            if (allInputs[scrollIn.position]->input != nullptr) {
                currentFocus.id = allInputs[scrollIn.position]->input->id;
            }
        }
        else {
            currentFocus.id = 0;
        }
        currentFocus.type = FOCUSINPUT;
    }

    registerElements();

    // register Panel Seetings
    registerPanelSettings();

    for (int x = 0; x < MATRIXCOLUMN; x++) {
        panelElementsOut[x].Draw();
    }
    for (int y = 0; y < MATRIXROWS; y++) {
        panelElementsModule[y].Draw();
    }

    drawRectangleFill(cWhite, panelAbsX + 94, panelAbsY + 35, 2, panelHeight - 35);
    for (int y = 0; y < PATCHMATRIXROWS; y++) {
        panelElementsIn[y].Draw();

        for (int x = 0; x < MATRIXCOLUMN; x++) {
            panelElementsPatch[x][y].Draw();
        }
    }
}

void GUIPanelPatchMatrix::registerPanelSettings() {

    actionHandler.registerActionEncoder(4, {std::bind(&GUIPanelPatchMatrix::scrollModulePosition, this, 1), "MODULE"},
                                        {std::bind(&GUIPanelPatchMatrix::scrollModulePosition, this, -1), "MODULE"},
                                        {std::bind(&GUIPanelPatchMatrix::setFocus, this, FOCUSMODULE), "FOCUS"});
    actionHandler.registerActionEncoder(0, {std::bind(&GUIPanelPatchMatrix::scrollInPosition, this, 1), "IN"},
                                        {std::bind(&GUIPanelPatchMatrix::scrollInPosition, this, -1), "IN"},
                                        {std::bind(&GUIPanelPatchMatrix::setFocus, this, FOCUSINPUT), "FOCUS"});
    actionHandler.registerActionEncoder(3, {std::bind(&GUIPanelPatchMatrix::scrollOutPosition, this, 1), "OUT"},
                                        {std::bind(&GUIPanelPatchMatrix::scrollOutPosition, this, -1), "OUT"},
                                        {std::bind(&GUIPanelPatchMatrix::setFocus, this, FOCUSOUTPUT), "FOCUS"});

    actionHandler.registerActionEncoder(2);
    actionHandler.registerActionEncoder(1);

    actionHandler.registerActionLeftData(0, {std::bind(&GUIPanelPatchMatrix::setEnvView, this), "ENV"}, &(this->isEnv));
    actionHandler.registerActionLeftData(1, {std::bind(&GUIPanelPatchMatrix::setMidiView, this), "MIDI"},
                                         &(this->isMidi));
    actionHandler.registerActionLeftData(2, {std::bind(&GUIPanelPatchMatrix::setAudioView, this), "AUDIO"},
                                         &(this->isAudio));

    actionHandler.registerActionRightData(0, {std::bind(&GUIPanelPatchMatrix::toggleFilterdView, this), "FILTER"},
                                          &(this->filteredView));

    if (panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry != nullptr) {
        actionHandler.registerActionRight(
            1, {std::bind(&Layer::removePatchInOutById, allLayers[cachedFocus.layer],
                          panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry->sourceOut->idGlobal,
                          panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry->targetIn->idGlobal),
                "RESET"});
    }
    else {
        actionHandler.registerActionRight(1);
    }
    actionHandler.registerActionRight(2, {std::bind(&GUIPanelPatchMatrix::clearPatches, this), "CLEAR"}, 1);

    if ((panelElementsOut[scrollOut.relPosition].entry != nullptr) &&
        (panelElementsOut[scrollOut.relPosition].entry != nullptr)) {
        if (panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry != nullptr) {

            actionHandler.registerActionEncoder(
                5,
                {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped,
                           panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry, 1),
                 "AMOUNT"},
                {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped,
                           panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry, 0),
                 "AMOUNT"},
                {std::bind(&Layer::removePatchInOutById, allLayers[cachedFocus.layer],
                           panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry->sourceOut->idGlobal,
                           panelElementsPatch[scrollOut.relPosition][scrollIn.relPosition].entry->targetIn->idGlobal),
                 "RESET"});
        }
        else {
            actionHandler.registerActionEncoder(
                5,
                {std::bind(&Layer::addPatchInOutById, allLayers[cachedFocus.layer],
                           panelElementsOut[scrollOut.relPosition].entry->idGlobal,
                           panelElementsIn[scrollIn.relPosition].entry->input->idGlobal, 0),
                 "AMOUNT"},
                {std::bind(&Layer::addPatchInOutById, allLayers[cachedFocus.layer],
                           panelElementsOut[scrollOut.relPosition].entry->idGlobal,
                           panelElementsIn[scrollIn.relPosition].entry->input->idGlobal, 0),
                 "AMOUNT"},
                {nullptr, "RESET"});
        }
    }
    else {
        actionHandler.registerActionEncoder(5);
    }
}

void GUIPanelPatchMatrix::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y, std::string name, uint8_t id,
                               uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;
    this->pathVisible = pathVisible;

    uint16_t moduleWidth = 100;
    uint16_t inElementWidth = 117;
    uint16_t outElementHeight = 35;

    uint16_t patchElementWidth = (width + 1 - inElementWidth - moduleWidth) / MATRIXCOLUMN;
    uint16_t rowHeigth = (height - outElementHeight) / PATCHMATRIXROWS;

    for (int i = 0; i < MATRIXCOLUMN; i++) {
        panelElementsOut[i].init(inElementWidth + moduleWidth + panelAbsX + patchElementWidth * i, panelAbsY,
                                 patchElementWidth - 1, outElementHeight - 1);
    }
    for (int j = 0; j < PATCHMATRIXROWS; j++) {
        panelElementsIn[j].init(panelAbsX + moduleWidth, outElementHeight + panelAbsY + rowHeigth * j,
                                inElementWidth - 1, rowHeigth - 1);

        for (int k = 0; k < MATRIXCOLUMN; k++) {
            panelElementsPatch[k][j].init(inElementWidth + moduleWidth + panelAbsX + patchElementWidth * k,
                                          outElementHeight + panelAbsY + rowHeigth * j, patchElementWidth - 1,
                                          rowHeigth - 1);
        }
    }

    rowHeigth = (height - outElementHeight) / MATRIXROWS;

    for (int j = 0; j < MATRIXROWS; j++) {

        panelElementsModule[j].init(panelAbsX, outElementHeight + panelAbsY + rowHeigth * j, moduleWidth - 6,
                                    rowHeigth - 1);
    }
}

void GUIPanelPatchMatrix::addCurrentPatch() {
    allLayers[cachedFocus.layer]->addPatchInOutById(panelElementsOut[scrollOut.relPosition].entry->idGlobal,
                                                    panelElementsIn[scrollIn.relPosition].entry->input->idGlobal);
}

void GUIPanelPatchMatrix::removeCurrentPatch() {
    //
    allLayers[cachedFocus.layer]->removePatchInOutById(panelElementsOut[scrollOut.relPosition].entry->idGlobal,
                                                       panelElementsIn[scrollIn.relPosition].entry->input->idGlobal);
}

void GUIPanelPatchMatrix::clearPatches() {
    allLayers[cachedFocus.layer]->clearPatches();
}

#endif