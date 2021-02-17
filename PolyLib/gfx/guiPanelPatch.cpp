#ifdef POLYCONTROL

#include "guiPanelPatch.hpp"

void GUIPanelPatch::registerElements() {
    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    dataIndex = scrollModule.offset;

    // register Module Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrysModule) {

            panelElementsModule[elementIndex].addEntry(allLayers[currentFocus.layer]->getModules()[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }

        elementIndex++;
    }
    panelElementsModule[scrollModule.relPosition].select = 1;

    elementIndex = 0;
    dataIndex = scrollSource.offset;
    // register Source Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrysSource) {

            panelElementsSource[elementIndex].addEntry((BasePatch *)allLayers[currentFocus.layer]->outputs[dataIndex],
                                                       1);
            dataIndex++;
        }
        else {
            break;
        }

        elementIndex++;
    }
    panelElementsSource[scrollSource.relPosition].select = 1;

    elementIndex = 0;
    dataIndex = scrollTarget.offset;
    // register Target Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrysTarget) {

            panelElementsTarget[elementIndex].addEntry((BasePatch *)allLayers[currentFocus.layer]
                                                           ->getModules()[scrollModule.position]
                                                           ->getInputs()[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }

        elementIndex++;
    }
    panelElementsTarget[scrollTarget.relPosition].select = 1;

    // register PatchMarker
    if (flipView) {

        // für jeden Eintrag
        for (int i = 0; i < maxEntrys; i++) {

            if (i < entrysSource) {
                if (panelElementsSource[i].entry->getPatchesInOut().size()) {
                    panelElementsSource[i].patched = 2;
                }
            }

            if (i < entrysModule) {
                for (PatchElementInOut *p : panelElementsSource[scrollSource.relPosition].entry->getPatchesInOut()) {

                    if (panelElementsModule[i].entry->id == p->targetIn->moduleId) {
                        panelElementsModule[i].patched = 1;
                    }
                }
            }
        }
        for (int i = 0; i < maxEntrys; i++) {

            if (i < entrysTarget) {

                for (PatchElementInOut *p : panelElementsTarget[i].entry->getPatchesInOut()) {

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
            for (PatchElementInOut *p : panelElementsTarget[scrollTarget.relPosition].entry->getPatchesInOut()) {
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

void GUIPanelPatch::updateEntrys() {
    entrysModule = allLayers[currentFocus.layer]->getModules().size();
    entrysTarget = allLayers[currentFocus.layer]->getModules()[scrollModule.position]->getInputs().size();
    entrysSource = allLayers[currentFocus.layer]->outputs.size();
    // check ScrollPosition
    scrollModule.entrys = entrysModule;
    scrollTarget.entrys = entrysTarget;
    scrollSource.entrys = entrysSource;

    // check Scroll position
    scrollSource.checkScroll();
    scrollTarget.checkScroll();
    scrollModule.checkScroll();
}

void GUIPanelPatch::Draw() {
    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    registerElements();

    for (int i = 0; i < maxEntrys; i++) {
        if (flipView) {
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
    if (flipView) {

        actionHandler.registerActionEncoder2({std::bind(&Scroller::scroll, &(this->scrollModule), 1), "MODULES"},
                                             {std::bind(&Scroller::scroll, &(this->scrollModule), -1), "MODULES"},
                                             {std::bind(&GUIPanelPatch::setFocus, this, FOCUSMODULE), "FOCUS"});
        actionHandler.registerActionEncoder3({std::bind(&Scroller::scroll, &(this->scrollTarget), 1), "INPUT"},
                                             {std::bind(&Scroller::scroll, &(this->scrollTarget), -1), "INPUT"},
                                             {std::bind(&GUIPanelPatch::setFocus, this, FOCUSINPUT), "FOCUS"});
        actionHandler.registerActionEncoder1({std::bind(&Scroller::scroll, &(this->scrollSource), 1), "OUTPUT"},
                                             {std::bind(&Scroller::scroll, &(this->scrollSource), -1), "OUTPUT"},
                                             {std::bind(&GUIPanelPatch::setFocus, this, FOCUSOUTPUT), "FOCUS"});
    }
    else {
        actionHandler.registerActionEncoder1({std::bind(&Scroller::scroll, &(this->scrollModule), 1), "MODULES"},
                                             {std::bind(&Scroller::scroll, &(this->scrollModule), -1), "MODULES"},
                                             {std::bind(&GUIPanelPatch::setFocus, this, FOCUSMODULE), "FOCUS"});
        actionHandler.registerActionEncoder2({std::bind(&Scroller::scroll, &(this->scrollTarget), 1), "INPUT"},
                                             {std::bind(&Scroller::scroll, &(this->scrollTarget), -1), "INPUT"},
                                             {std::bind(&GUIPanelPatch::setFocus, this, FOCUSINPUT), "FOCUS"});
        actionHandler.registerActionEncoder3({std::bind(&Scroller::scroll, &(this->scrollSource), 1), "OUTPUT"},
                                             {std::bind(&Scroller::scroll, &(this->scrollSource), -1), "OUTPUT"},
                                             {std::bind(&GUIPanelPatch::setFocus, this, FOCUSOUTPUT), "FOCUS"});
    }
    // register Panel Seetings Left

    if (globalSettings.multiLayer.value == 1) {
        actionHandler.registerActionLeft({std::bind(&GUIPanelPatch::toggleFlipView, this), "FLIP"}, // RESET
                                         {std::bind(&GUIPanelPatch::clearPatches, this), "CLEAR"},
                                         {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeft({std::bind(&GUIPanelPatch::toggleFlipView, this), "FLIP"}, // RESET
                                         {std::bind(&GUIPanelPatch::clearPatches, this), "CLEAR"}, {nullptr, ""});
    }

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({std::bind(&GUIPanelPatch::addCurrentPatch, this), "ADD"}, // SAVE
                                      {std::bind(&GUIPanelPatch::removeCurrentPatch, this), "REMOVE"}, {nullptr, ""});

    // clear Encoder 4
    actionHandler.registerActionEncoder4({nullptr, ""}, {nullptr, ""}, {nullptr, ""}); // clear action
}

void GUIPanelPatch::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id,
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