#ifdef POLYCONTROL

#include "guiPanelPatch.hpp"

void GUIPanelPatch::registerElements() {
    uint16_t dataIndex = 0;

    dataIndex = scrollModule.offset;

    // register Module Elements
    for (uint32_t elementIndex = 0; elementIndex < maxEntrys; elementIndex++) {
        if (dataIndex < entrysModule) {

            panelElementsModule[elementIndex].addEntry(allModules[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }
    }
    panelElementsModule[scrollModule.relPosition].select = 1;

    dataIndex = scrollSource.offset;
    // register Source Elements

    for (uint32_t elementIndex = 0; elementIndex < maxEntrys; elementIndex++) {
        if (dataIndex < entrysSource) {

            panelElementsSource[elementIndex].addEntry(allOutputs[dataIndex], 1);
            dataIndex++;
        }
        else {
            break;
        }
    }
    panelElementsSource[scrollSource.relPosition].select = 1;

    dataIndex = scrollTarget.offset;
    // register Target Elements

    for (uint32_t elementIndex = 0; elementIndex < maxEntrys; elementIndex++) {
        if (dataIndex < entrysTarget) {

            panelElementsTarget[elementIndex].addEntry(allInputs[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }
    }
    panelElementsTarget[scrollTarget.relPosition].select = 1;

    // register PatchMarker
    if (!flipView) {

        // für jeden Eintrag
        for (int i = 0; i < maxEntrys; i++) {

            if (i < entrysSource) {
                if (panelElementsSource[i].entry->getPatchesInOut().size()) {
                    panelElementsSource[i].patched = 2;
                }
            }

            if (i < entrysModule) {
                for (PatchElement *p : panelElementsSource[scrollSource.relPosition].entry->getPatchesInOut()) {

                    if (panelElementsModule[i].entry->id == p->targetIn->moduleId) {
                        panelElementsModule[i].patched = 1;
                    }
                }
            }
        }
        for (int i = 0; i < maxEntrys; i++) {

            if (i < entrysTarget) {

                for (PatchElement *p : panelElementsTarget[i].entry->getPatchesInOut()) {

                    if (panelElementsSource[scrollSource.relPosition].entry->idGlobal == p->sourceOut->idGlobal) {
                        panelElementsTarget[i].patched = 1;

                        if (panelElementsModule[scrollModule.relPosition].entry->id == p->targetIn->moduleId) {
                            panelElementsModule[scrollModule.relPosition].patched = 3;
                        }
                        panelElementsTarget[i].addPatchEntry(p); // register patchElement
                    }
                }
            }
        }
    }
    else {
        // Liegt auf dem target ein Output?  -> makierung rechts

        // für jeden Eintrag
        for (int i = 0; i < maxEntrys; i++) {
            if (i < entrysTarget) {

                if (panelElementsTarget[i].entry->getPatchesInOut().size()) {
                    panelElementsTarget[i].patched = 2;
                }

                // Ist die Source auf den aktuellen target connected -> makierung links
            }
        }
        if (entrysTarget != 0) {
            for (PatchElement *p : panelElementsTarget[scrollTarget.relPosition].entry->getPatchesInOut()) {
                for (int i = 0; i < maxEntrys; i++) {
                    if (i < entrysSource) {
                        if (panelElementsSource[i].entry->idGlobal == p->sourceOut->idGlobal) {
                            panelElementsSource[i].patched = 1;
                            panelElementsSource[i].addPatchEntry(p); // register patchElement
                        }
                    }
                }
            }
        }
    }
}

void GUIPanelPatch::collectModules() {

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

void GUIPanelPatch::collectInputs() {

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
    entrysTarget = allInputs.size();
    scrollTarget.entrys = entrysTarget;
    scrollTarget.checkScroll();
}
void GUIPanelPatch::collectOutputs() {

    allOutputs.clear();

    for (Output *output : allLayers[currentFocus.layer]->outputs) { // Collect Module if inputs available
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
    entrysSource = allOutputs.size();
    scrollSource.entrys = entrysSource;
    scrollSource.checkScroll();
}

void GUIPanelPatch::updateEntrys() {

    collectModules();
    collectInputs();
    collectOutputs();
}

void GUIPanelPatch::activate() {

    collectModules();

    if (currentFocus.type == FOCUSINPUT || currentFocus.type == FOCUSMODULE) {

        for (uint32_t i = 0; i < allModules.size(); i++) {
            if (allModules[i]->id == currentFocus.modul) {
                scrollModule.setScroll(i);
                break;
            }
        }
    }

    if (currentFocus.type == FOCUSINPUT) {
        collectInputs();
        for (uint32_t i = 0; i < allInputs.size(); i++) {
            if (allInputs[i]->id == currentFocus.id) {
                scrollTarget.setScroll(i);
                break;
            }
        }
    }

    if (currentFocus.type == FOCUSOUTPUT) {

        if (allLayers[currentFocus.layer]->modules[currentFocus.modul]->outputs.size()) {
            uint16_t searchID = allLayers[currentFocus.layer]->modules[currentFocus.modul]->outputs[0]->idGlobal;

            collectOutputs();
            for (uint32_t i = 0; i < allOutputs.size(); i++) {
                if (allOutputs[i]->idGlobal == searchID) {
                    scrollSource.setScroll(i);
                    break;
                }
            }
        }
        else {
            scrollSource.setScroll(0);
        }
    }
}

// Change Focus with PatchPanel Scolling
void GUIPanelPatch::scrollModulePosition(int16_t scroll) {
    scrollModule.scroll(scroll);
    currentFocus.modul = allModules[scrollModule.position]->id;
    updateEntrys();
    currentFocus.id = allInputs[scrollTarget.position]->id;
    currentFocus.type = FOCUSINPUT;
}
void GUIPanelPatch::scrollTargetPosition(int16_t scroll) {
    scrollTarget.scroll(scroll);
    currentFocus.modul = allModules[scrollModule.position]->id;
    updateEntrys();
    currentFocus.id = allInputs[scrollTarget.position]->id;
    currentFocus.type = FOCUSINPUT;
}
void GUIPanelPatch::scrollSourcePosition(int16_t scroll) {
    scrollSource.scroll(scroll);
    currentFocus.modul = allOutputs[scrollSource.position]->moduleId;
    currentFocus.id = allOutputs[scrollSource.position]->id;
    currentFocus.type = FOCUSOUTPUT;
}

void GUIPanelPatch::Draw() {
    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    registerElements();

    for (int i = 0; i < maxEntrys; i++) {
        if (!flipView) {
            panelElementsModule[i].panelAbsX = absXPositions[1];
            panelElementsTarget[i].panelAbsX = absXPositions[2];
            panelElementsSource[i].panelAbsX = absXPositions[0];
        }
        else {
            panelElementsModule[i].panelAbsX = absXPositions[0];
            panelElementsTarget[i].panelAbsX = absXPositions[1];
            panelElementsSource[i].panelAbsX = absXPositions[2];
        }

        panelElementsSource[i].Draw();
        panelElementsTarget[i].Draw();
        panelElementsModule[i].Draw();
    }

    // drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight,
    // scrollOffset, entrys,
    //               CONFIGPANELENTRYS);
}

void GUIPanelPatch::registerPanelSettings() {
    if (!flipView) {

        actionHandler.registerActionEncoder(1, {std::bind(&GUIPanelPatch::scrollModulePosition, this, 1), "MODULES"},
                                            {std::bind(&GUIPanelPatch::scrollModulePosition, this, -1), "MODULES"},
                                            {std::bind(&GUIPanelPatch::setFocus, this, FOCUSMODULE), "FOCUS"});
        actionHandler.registerActionEncoder(2, {std::bind(&GUIPanelPatch::scrollTargetPosition, this, 1), "INPUT"},
                                            {std::bind(&GUIPanelPatch::scrollTargetPosition, this, -1), "INPUT"},
                                            {std::bind(&GUIPanelPatch::setFocus, this, FOCUSINPUT), "FOCUS"});
        actionHandler.registerActionEncoder(0, {std::bind(&GUIPanelPatch::scrollSourcePosition, this, 1), "OUTPUT"},
                                            {std::bind(&GUIPanelPatch::scrollSourcePosition, this, -1), "OUTPUT"},
                                            {std::bind(&GUIPanelPatch::setFocus, this, FOCUSOUTPUT), "FOCUS"});
    }
    else {
        actionHandler.registerActionEncoder(0, {std::bind(&GUIPanelPatch::scrollModulePosition, this, 1), "MODULES"},
                                            {std::bind(&GUIPanelPatch::scrollModulePosition, this, -1), "MODULES"},
                                            {std::bind(&GUIPanelPatch::setFocus, this, FOCUSMODULE), "FOCUS"});
        actionHandler.registerActionEncoder(1, {std::bind(&GUIPanelPatch::scrollTargetPosition, this, 1), "INPUT"},
                                            {std::bind(&GUIPanelPatch::scrollTargetPosition, this, -1), "INPUT"},
                                            {std::bind(&GUIPanelPatch::setFocus, this, FOCUSINPUT), "FOCUS"});
        actionHandler.registerActionEncoder(2, {std::bind(&GUIPanelPatch::scrollSourcePosition, this, 1), "OUTPUT"},
                                            {std::bind(&GUIPanelPatch::scrollSourcePosition, this, -1), "OUTPUT"},
                                            {std::bind(&GUIPanelPatch::setFocus, this, FOCUSOUTPUT), "FOCUS"});
    }
    // register Panel Seetings Left

    if (globalSettings.multiLayer.value == 1) {
        actionHandler.registerActionLeft(2, {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeftData(0, {std::bind(&GUIPanelPatch::toggleFlipView, this), "FLIP"},
                                             &(this->flipView));
        actionHandler.registerActionLeftData(1, {std::bind(&GUIPanelPatch::toggleFilterdView, this), "FILTER"},
                                             &(this->filteredView));
        actionHandler.registerActionLeft(2);
    }

    // register Panel Seetings Right
    actionHandler.registerActionRight(0, {std::bind(&GUIPanelPatch::addCurrentPatch, this), "ADD"});
    actionHandler.registerActionRight(1, {std::bind(&GUIPanelPatch::removeCurrentPatch, this), "REMOVE"}, 1);
    actionHandler.registerActionRight(2, {std::bind(&GUIPanelPatch::clearPatches, this), "CLEAR"}, 1);

    // clear Encoder 4
    actionHandler.registerActionEncoder(3, {nullptr, ""}, {nullptr, ""}, {nullptr, ""}); // clear action
}

void GUIPanelPatch::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y, std::string name, uint8_t id,
                         uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;
    this->pathVisible = pathVisible;

    uint16_t elementSpaceX = 35;
    // elements Sizes
    uint16_t elementWidth = (width - 2 * elementSpaceX) / 3;
    uint16_t elementSpaceY = 3;
    uint16_t elementHeight = (height - (maxEntrys - 2) * elementSpaceY) / maxEntrys;

    absXPositions[0] = x;
    absXPositions[1] = x + elementWidth + elementSpaceX;
    absXPositions[2] = x + 2 * (elementWidth + elementSpaceX);

    // init Elements
    for (int i = 0; i < maxEntrys; i++) {

        panelElementsSource[i].init(0, panelAbsY + (elementHeight + elementSpaceY) * i, elementWidth, elementHeight);
        panelElementsTarget[i].init(0, panelAbsY + (elementHeight + elementSpaceY) * i, elementWidth, elementHeight);
        panelElementsModule[i].init(0, panelAbsY + (elementHeight + elementSpaceY) * i, elementWidth, elementHeight);
    }
}

void GUIPanelPatch::addCurrentPatch() {
    allLayers[currentFocus.layer]->addPatchInOutById(panelElementsSource[scrollSource.relPosition].entry->idGlobal,
                                                     panelElementsTarget[scrollTarget.relPosition].entry->idGlobal);
}

void GUIPanelPatch::removeCurrentPatch() {
    allLayers[currentFocus.layer]->removePatchInOutById(panelElementsSource[scrollSource.relPosition].entry->idGlobal,
                                                        panelElementsTarget[scrollTarget.relPosition].entry->idGlobal);
}

void GUIPanelPatch::clearPatches() {
    allLayers[currentFocus.layer]->clearPatches();
}

#endif