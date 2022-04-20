#ifdef POLYCONTROL

#include "GUIPanelPatchMatrix.hpp"

void GUIPanelPatchMatrix::registerElements() {

    uint16_t dataIndex = 0;

    Output *out;
    Input *in;

    for (uint16_t x = 0; x < MATRIXCOLUMN; x++) {
        if ((x + scrollOut.offset) < allOutputs.size()) {
            panelElementsOut[x].addEntry(allOutputs[x + scrollOut.offset]);
        }
    }

    for (uint16_t y = 0; y < MATRIXROWS; y++) {
        if ((y + scrollIn.offset) < allInputs.size()) {
            in = allInputs[y + scrollIn.offset];
            panelElementsIn[y].addEntry(in);
            for (uint16_t x = 0; x < MATRIXCOLUMN; x++) {
                if ((x + scrollOut.offset) < allOutputs.size()) {
                    out = allOutputs[x + scrollOut.offset];
                    panelElementsPatch[x][y].addEntry(nullptr);
                    for (uint16_t i = 0; i < in->getPatchesInOut().size(); i++) {
                        if (in->getPatchesInOut()[i]->sourceOut->idGlobal == out->idGlobal) {
                            panelElementsPatch[x][y].addEntry(in->getPatchesInOut()[i]);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (uint16_t y = 0; y < MATRIXROWS; y++) {
        if ((y + scrollModule.offset) < allModules.size()) {
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

    for (BaseModule *module : allLayers[currentFocus.layer]->getModules()) { // Collect Module if inputs available
        uint8_t check = 0;
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
        for (Input *input : allModules[scrollModule.position]->inputs) {
            if (input->visible) {
                if (filteredView) {
                    if (input->patchesInOut.size())
                        allInputs.push_back(input);
                }
                else {
                    allInputs.push_back(input);
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

    if (midiView) {

        for (Output *output : allLayers[currentFocus.layer]->midi.outputs) { // Collect Module if inputs available
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
    else {
        for (Output *output : allLayers[currentFocus.layer]->outputs) { // Collect Module if inputs available
            if (output->moduleId != allLayers[currentFocus.layer]->midi.id){
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

    collectModules();

    if (currentFocus.type == FOCUSINPUT || currentFocus.type == FOCUSMODULE) {

        for (uint16_t i = 0; i < allModules.size(); i++) {
            if (allModules[i]->id == currentFocus.modul) {
                scrollModule.setScroll(i);
                break;
            }
        }
    }

    if (currentFocus.type == FOCUSINPUT) {
        collectInputs();
        for (uint16_t i = 0; i < allInputs.size(); i++) {
            if (allInputs[i]->id == currentFocus.id) {
                scrollIn.setScroll(i);
                break;
            }
        }
    }

    if (currentFocus.type == FOCUSOUTPUT) {

        if (allLayers[currentFocus.layer]->modules[currentFocus.modul]->outputs.size()) {
            uint16_t searchID = allLayers[currentFocus.layer]->modules[currentFocus.modul]->outputs[0]->idGlobal;

            collectOutputs();
            for (uint16_t i = 0; i < allOutputs.size(); i++) {
                if (allOutputs[i]->idGlobal == searchID) {
                    scrollOut.setScroll(i);
                    break;
                }
            }
        }
        else {
            scrollOut.setScroll(0);
        }
    }
}

// Change Focus with PatchPanel Scolling
void GUIPanelPatchMatrix::scrollModulePosition(int16_t scroll) {
    scrollModule.scroll(scroll);
    currentFocus.modul = allModules[scrollModule.position]->id;
    collectModules();
    collectInputs();

    currentFocus.id = allInputs[scrollIn.position]->id;
    currentFocus.type = FOCUSINPUT;
}
void GUIPanelPatchMatrix::scrollInPosition(int16_t scroll) {
    scrollIn.scroll(scroll);
    currentFocus.modul = allModules[scrollModule.position]->id;
    collectModules();
    collectInputs();
    currentFocus.id = allInputs[scrollIn.position]->id;
    currentFocus.type = FOCUSINPUT;
}
void GUIPanelPatchMatrix::scrollOutPosition(int16_t scroll) {
    scrollOut.scroll(scroll);
    currentFocus.modul = allOutputs[scrollOut.position]->moduleId;
    currentFocus.id = allOutputs[scrollOut.position]->id;
    currentFocus.type = FOCUSOUTPUT;
}

void GUIPanelPatchMatrix::Draw() {
    // register Panel Seetings
    registerPanelSettings();

    // update number ob entrys
    collectModules();
    collectInputs();
    collectOutputs();

    registerElements();

    for (int x = 0; x < MATRIXCOLUMN; x++) {
        panelElementsOut[x].Draw();
    }
    for (int y = 0; y < MATRIXROWS; y++) {
        panelElementsIn[y].Draw();
        panelElementsModule[y].Draw();

        for (int x = 0; x < MATRIXCOLUMN; x++) {
            panelElementsPatch[x][y].Draw();
        }
    }
}
// drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight,
// scrollOffset, entrys,
//               CONFIGPANELENTRYS);

void GUIPanelPatchMatrix::registerPanelSettings() {

    actionHandler.registerActionEncoder(0, {std::bind(&GUIPanelPatchMatrix::scrollModulePosition, this, 1), "MODULES"},
                                        {std::bind(&GUIPanelPatchMatrix::scrollModulePosition, this, -1), "MODULES"},
                                        {std::bind(&GUIPanelPatchMatrix::setFocus, this, FOCUSMODULE), "FOCUS"});
    actionHandler.registerActionEncoder(1, {std::bind(&GUIPanelPatchMatrix::scrollInPosition, this, 1), "INPUT"},
                                        {std::bind(&GUIPanelPatchMatrix::scrollInPosition, this, -1), "INPUT"},
                                        {std::bind(&GUIPanelPatchMatrix::setFocus, this, FOCUSINPUT), "FOCUS"});
    actionHandler.registerActionEncoder(2, {std::bind(&GUIPanelPatchMatrix::scrollOutPosition, this, 1), "OUTPUT"},
                                        {std::bind(&GUIPanelPatchMatrix::scrollOutPosition, this, -1), "OUTPUT"},
                                        {std::bind(&GUIPanelPatchMatrix::setFocus, this, FOCUSOUTPUT), "FOCUS"});

    actionHandler.registerActionEncoder(3);

    if (globalSettings.multiLayer.value == 1) {
        actionHandler.registerActionLeft(2, {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeft(2);
    }

    actionHandler.registerActionLeftData(0, {std::bind(&GUIPanelPatchMatrix::toggleMidiView, this), "MIDI"},
                                         &(this->midiView));
    actionHandler.registerActionLeftData(1, {std::bind(&GUIPanelPatchMatrix::toggleFilterdView, this), "FILTER"},
                                         &(this->filteredView));

    // register Panel Settings Right
    actionHandler.registerActionRight(0, {std::bind(&GUIPanelPatchMatrix::addCurrentPatch, this), "ADD"});
    actionHandler.registerActionRight(1, {std::bind(&GUIPanelPatchMatrix::removeCurrentPatch, this), "REMOVE"});
    actionHandler.registerActionRight(2, {std::bind(&GUIPanelPatchMatrix::clearPatches, this), "CLEAR"}, 1);
}

void GUIPanelPatchMatrix::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id,
                               uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;
    this->pathVisible = pathVisible;

    uint16_t moduleWidth = 100;
    uint16_t inElementWidth = 100;
    uint16_t outElementHeight = 30;

    uint16_t patchElementWidth = (width - inElementWidth - moduleWidth) / MATRIXCOLUMN;
    uint16_t rowHeigth = (height - outElementHeight) / MATRIXROWS;

    for (int i = 0; i < MATRIXCOLUMN; i++) {
        panelElementsOut[i].init(inElementWidth + moduleWidth + panelAbsX + patchElementWidth * i, panelAbsY,
                                 patchElementWidth - 1, outElementHeight - 1);
    }
    for (int j = 0; j < MATRIXROWS; j++) {
        panelElementsIn[j].init(panelAbsX + moduleWidth, outElementHeight + panelAbsY + rowHeigth * j,
                                inElementWidth - 1, rowHeigth - 1);

        panelElementsModule[j].init(panelAbsX, outElementHeight + panelAbsY + rowHeigth * j, moduleWidth - 1,
                                    rowHeigth - 1);

        for (int k = 0; k < MATRIXCOLUMN; k++) {
            panelElementsPatch[k][j].init(inElementWidth + moduleWidth + panelAbsX + patchElementWidth * k,
                                          outElementHeight + panelAbsY + rowHeigth * j, patchElementWidth - 1,
                                          rowHeigth - 1);
        }
    }
}

void GUIPanelPatchMatrix::addCurrentPatch() {
    allLayers[currentFocus.layer]->addPatchInOutById(panelElementsOut[scrollOut.relPosition].entry->idGlobal,
                                                     panelElementsIn[scrollIn.relPosition].entry->idGlobal);
}

void GUIPanelPatchMatrix::removeCurrentPatch() {
    //
    allLayers[currentFocus.layer]->removePatchInOutById(panelElementsOut[scrollOut.relPosition].entry->idGlobal,
                                                        panelElementsIn[scrollIn.relPosition].entry->idGlobal);
}

void GUIPanelPatchMatrix::clearPatches() {
    allLayers[currentFocus.layer]->clearPatches();
}

#endif