#ifdef POLYCONTROL

#include "guiPanelFocus.hpp"

void GUIPanelFocus::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->pathVisible = pathVisible;

    // elements Sizes
    uint16_t elementWidth = width - SCROLLBARWIDTH - 2;
    uint16_t elementSpace = 1;
    uint16_t elementHeight = (height - (FOCUSPANELENTRYS - 2) * elementSpace) / FOCUSPANELENTRYS;

    // init Elements
    for (int i = 0; i < FOCUSPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }

    elementHeight = (height - (FOCUSPANELENTRYS - 2) * elementSpace - waveBuffer.height) / FOCUSPANELENTRYSWAVE;
    for (int i = 0; i < FOCUSPANELENTRYSWAVE; i++) {
        panelElementsWave[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i + waveBuffer.height,
                                  elementWidth, elementHeight);
    }

    name = "FOCUS";
}

void GUIPanelFocus::registerPanelSettings() {
    // register Scroll

    if (newPanelFocus.type != NOFOCUS) {

        actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, this->scroll, 1), "SCROLL"},
                                            {std::bind(&Scroller::scroll, this->scroll, -1), "SCROLL"},
                                            {std::bind(focusDown, newPanelFocus), "FOCUS"});
    }
    else {
        actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, this->scroll, 1), "SCROLL"},
                                            {std::bind(&Scroller::scroll, this->scroll, -1), "SCROLL"});
    }

    actionHandler.registerActionLeft(0);
    actionHandler.registerActionLeft(1, {std::bind(focusUp), "UP"});
    // register Panel Settings Left
    if (globalSettings.multiLayer.value == 1) {
        actionHandler.registerActionLeft(2, {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeft(2);
    }

    // register Panel Settings Right
    actionHandler.registerActionRight(0);
    actionHandler.registerActionRight(1);
    actionHandler.registerActionRight(2);
}

void GUIPanelFocus::Draw() {
    // register Panel Settings
    registerPanelSettings();

    // update entrys
    collectEntrys();

    // resetFocus
    newPanelFocus.type = NOFOCUS;

    if (currentFocus.type == FOCUSMODULE) {
        ModuleType type = allLayers[currentFocus.layer]->modules[currentFocus.modul]->moduleType;

        if (type == MODULE_OSC || type == MODULE_SUB || type == MODULE_LFO || type == MODULE_ADSR ||
            type == MODULE_PHASE || type == MODULE_WAVESHAPER) {
            entrys = FOCUSPANELENTRYSWAVE;
            scroll->maxEntrysVisible = entrys;

            registerModuleSettings(panelElementsWave);

            for (int i = 0; i < FOCUSPANELENTRYSWAVE; i++) {
                panelElementsWave[i].Draw();
            }
            drawWaveFromModule(allLayers[currentFocus.layer]->modules[currentFocus.modul],
                               LCDWIDTH / 2 - waveBuffer.width / 2, HEADERHEIGHT + FOCUSHEIGHT + SPACER + SPACER);
            drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY + waveBuffer.height, SCROLLBARWIDTH,
                          panelHeight - waveBuffer.height, scroll->offset, scroll->entrys, FOCUSPANELENTRYSWAVE);
        }
        else { // module without wave
            entrys = FOCUSPANELENTRYS;
            scroll->maxEntrysVisible = entrys;

            registerModuleSettings(panelElements);
            for (int i = 0; i < FOCUSPANELENTRYS; i++) {
                panelElements[i].Draw();
            }

            drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight,
                          scroll->offset, scroll->entrys, FOCUSPANELENTRYS);
        }
    }
    else {
        entrys = FOCUSPANELENTRYS;
        scroll->maxEntrysVisible = entrys;

        if (currentFocus.type == FOCUSOUTPUT) {
            registerModulePatchOut();
            for (int i = 0; i < FOCUSPANELENTRYS; i++) {
                panelElements[i].Draw();
            }
        }
        else if (currentFocus.type == FOCUSINPUT) {
            registerModulePatchIn();
            for (int i = 0; i < FOCUSPANELENTRYS; i++) {
                panelElements[i].Draw();
            }
        }
        else if (currentFocus.type == FOCUSLAYER) {
            registerLayerModules();
            for (int i = 0; i < FOCUSPANELENTRYS; i++) {
                panelElements[i].Draw();
            }
        }
        drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scroll->offset,
                      scroll->entrys, FOCUSPANELENTRYS);
    }
}
void GUIPanelFocus::collectEntrys() {
    // set scroller
    if (currentFocus.type == FOCUSLAYER) {
        scroll = &scrollLayer;
    }
    else {
        scroll = &scrollModule;
    }
    // check for location change
    if (oldFocus.modul != currentFocus.modul) {
        scroll->resetScroll();
        oldFocus = currentFocus;
    }

    if (currentFocus.type == FOCUSINPUT) {
        patch.clear();

        for (PatchElement *p : allLayers[currentFocus.layer]
                                   ->getModules()[currentFocus.modul]
                                   ->getInputs()[currentFocus.id]
                                   ->getPatchesInOut()) {
            patch.push_back(p);
        }

        scroll->entrys = patch.size();
    }
    else if (currentFocus.type == FOCUSOUTPUT) {
        patch.clear();

        for (PatchElement *p : allLayers[currentFocus.layer]
                                   ->getModules()[currentFocus.modul]
                                   ->getOutputs()[currentFocus.id]
                                   ->getPatchesInOut()) {
            patch.push_back(p);
        }
        scroll->entrys = patch.size();
    }

    else if (currentFocus.type == FOCUSMODULE) {
        analog.clear();
        digital.clear();

        for (Analog *a : allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getPotis()) {
            if (a->displayVis) { // filter Display Visibility
                analog.push_back(a);
            }
        }
        for (Digital *d : allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getSwitches()) {
            if (d->displayVis) { // filter Display Visibility
                digital.push_back(d);
            }
        }

        scroll->entrys =
            analog.size() + digital.size() / SwitchEntrysPerElement + (digital.size() % SwitchEntrysPerElement != 0);
    }
    else if (currentFocus.type == FOCUSLAYER) {
        module.clear();
        for (BaseModule *m : allLayers[currentFocus.layer]->getModules()) {
            if (m->displayVis) { // filter Display Visibility
                module.push_back(m);
            }
        }
        scroll->entrys = module.size();
    }
    scroll->checkScroll(); // check scoll position
}

Analog *GUIPanelFocus::getAnalogEntry() {
    if (analogIndex < analog.size()) {
        return analog[analogIndex++];
    }

    return nullptr;
}

Digital *GUIPanelFocus::getDigitalEntry() {
    if (digitalIndex < digital.size()) {
        return digital[digitalIndex++];
    }
    return nullptr;
}

BaseModule *GUIPanelFocus::getModuleEntry() {
    if (moduleIndex < module.size()) {
        return module[moduleIndex++];
    }
    return nullptr;
}

PatchElement *GUIPanelFocus::getPatchEntry() {
    if (patchIndex < patch.size()) {
        return patch[patchIndex++];
    }
    return nullptr;
}

void GUIPanelFocus::registerModuleSettings(Data_PanelElement *dataPanelElements) {
    analogIndex = scroll->offset;

    if (analogIndex >= analog.size()) {
        digitalIndex = scroll->offset - analog.size();
    }
    else {
        digitalIndex = 0;
    }

    uint16_t elementIndex = 0;

    for (; elementIndex < entrys; elementIndex++) {

        Analog *a = getAnalogEntry();

        if (a == nullptr) {
            break;
        }
        else {

            dataPanelElements[elementIndex].addAnalogEntry(a);

            // register newFocus position for downMove
            if ((scroll->relPosition) == elementIndex) {
                if (a->input != nullptr) {
                    newPanelFocus.id = a->input->id;
                    newPanelFocus.modul = a->input->moduleId;
                    newPanelFocus.type = FOCUSINPUT;

                    actionHandler.registerActionRight(2, {std::bind(focusPatch, newPanelFocus), "IN"});
                }
            }
        }
    }

    Digital *d = nullptr;
    d = getDigitalEntry();
    for (; elementIndex < entrys; elementIndex++) {

        if (d == nullptr) // more digitalEntry available?
            break;

        for (int x = 0; x < SwitchEntrysPerElement; x++) { // start new Element

            if (d == nullptr) {                                  // no more entry available
                dataPanelElements[elementIndex].addEmptyEntry(); // empty entry
            }
            else {
                dataPanelElements[elementIndex].addDigitalEntry(d);
            }

            d = getDigitalEntry();
        }
    }

    dataPanelElements[scroll->relPosition].select = 1;
    if (allLayers[currentFocus.layer]->modules[currentFocus.modul]->outputs.size()) {
        actionHandler.registerActionRight(
            1, {std::bind(focusPatch, location(currentFocus.layer, currentFocus.modul, currentFocus.id, FOCUSOUTPUT)),
                "OUT"});
    }
}

void GUIPanelFocus::registerModulePatchIn() {

    patchIndex = scroll->offset;
    for (uint16_t elementIndex = 0; elementIndex < entrys; elementIndex++) {

        PatchElement *p = getPatchEntry();

        if (p == nullptr) {
            break;
        }
        else {

            panelElements[elementIndex].addPatchOutputEntry(p);
        }
    }

    panelElements[scroll->relPosition].select = 1;

    // register newFocus

    newPanelFocus.id = currentFocus.id;
    newPanelFocus.modul = currentFocus.modul;
    newPanelFocus.type = FOCUSINPUT;

    actionHandler.registerActionRight(2, {std::bind(focusPatch, newPanelFocus), "PATCH"});
}

void GUIPanelFocus::registerModulePatchOut() {
    patchIndex = scroll->offset;
    for (uint16_t elementIndex = 0; elementIndex < entrys; elementIndex++) {

        PatchElement *p = getPatchEntry();

        if (p == nullptr) {
            break;
        }
        else {

            panelElements[elementIndex].addPatchInputEntry(p);
        }
    }

    panelElements[scroll->relPosition].select = 1;

    // register newFocus

    newPanelFocus.id = currentFocus.id;
    newPanelFocus.modul = currentFocus.modul;
    newPanelFocus.type = FOCUSOUTPUT;

    actionHandler.registerActionRight(2, {std::bind(focusPatch, newPanelFocus), "PATCH"});
}

void GUIPanelFocus::registerLayerModules() {
    uint16_t elementIndex = 0;

    moduleIndex = scroll->offset;

    for (; elementIndex < entrys; elementIndex++) {

        BaseModule *m = getModuleEntry();

        if (m == nullptr) {
            break;
        }
        else {

            panelElements[elementIndex].addModuleEntry(m);

            if ((scroll->relPosition) == elementIndex) {
                newPanelFocus.modul = m->id;
                newPanelFocus.type = FOCUSMODULE;
            }
        }
    }

    panelElements[scroll->relPosition].select = 1;
}

#endif