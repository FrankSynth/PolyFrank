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
    uint16_t elementSpace = 3;
    uint16_t elementHeight = (height - (FOCUSPANELENTRYS - 2) * elementSpace) / FOCUSPANELENTRYS;

    // init Elements
    for (int i = 0; i < FOCUSPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }
}

void GUIPanelFocus::registerPanelSettings() {
    // register Scroll
    if (newFocusLocation.type != NOFOCUS) {
        actionHandler.registerActionEncoder1({std::bind(&Scroller::scroll, &(this->scroll), 1), "SCROLL"},
                                             {std::bind(&Scroller::scroll, &(this->scroll), -1), "SCROLL"},
                                             {std::bind(focusDown, newFocusLocation), "FOCUS"});
    }
    else {
        actionHandler.registerActionEncoder1({std::bind(&Scroller::scroll, &(this->scroll), 1), "SCROLL"},
                                             {std::bind(&Scroller::scroll, &(this->scroll), -1), "SCROLL"},
                                             {nullptr, ""});
    }

    // register Panel Seetings Left
    actionHandler.registerActionLeft({nullptr, ""}, {std::bind(focusUp), "UP"}, // focus Up
                                     {std::bind(nextLayer), "LAYER"});          // Layer Switch

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({nullptr, ""}, {std::bind(Todo), "RESET"}, {std::bind(Todo), "PATCH"});
}

void GUIPanelFocus::Draw() {
    // register Panel Seetings
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    // resetFocus
    newFocusLocation.type = NOFOCUS;

    if (focus.type == FOCUSMODULE) {
        registerModuleSettings();
        for (int i = 0; i < FOCUSPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSOUTPUT) {
        registerModulePatchOut();
        for (int i = 0; i < FOCUSPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSINPUT) {
        registerModulePatchIn();
        for (int i = 0; i < FOCUSPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSLAYER) {
        registerLayerModules();
        for (int i = 0; i < FOCUSPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scroll.offset,
                  entrys, FOCUSPANELENTRYS);
}

void GUIPanelFocus::updateEntrys() {
    entrys = 0;

    if (focus.type == FOCUSINPUT) {
        entrys = 0; // BaseValue
        entrys += allLayers[focus.layer]
                      ->getModules()[focus.modul]
                      ->getInputs()[focus.id]
                      ->getPatchesInOut()
                      .size(); // all patches to this input

        // println("entrys Input", entrys);
    }
    else if (focus.type == FOCUSOUTPUT) {
        entrys = 0; // BaseValue
        entrys += allLayers[focus.layer]
                      ->getModules()[focus.modul]
                      ->getOutputs()[focus.id]
                      ->getPatchesInOut()
                      .size(); // all Input patches to this output

        entrys += allLayers[focus.layer]
                      ->getModules()[focus.modul]
                      ->getOutputs()[focus.id]
                      ->getPatchesOutOut()
                      .size(); // all Output patches to this output
        // println("entrys Output", entrys);
    }
    else if (focus.type == FOCUSMODULE) {

        for (Analog *a : allLayers[focus.layer]->getModules()[focus.modul]->getPotis()) {
            if (a->displayVis) { // filter Display Visibility
                entrys++;
            }
        }
        uint8_t sub = 0;
        for (Digital *d : allLayers[focus.layer]->getModules()[focus.modul]->getSwitches()) {

            if (d->displayVis) { // filter Display Visibility
                if (!(sub % 3)) {
                    entrys++;
                }
                sub++;
            }
        }
    }
    else if (focus.type == FOCUSLAYER) {
        entrys = allLayers[focus.layer]->getModules().size(); // all Modules
    }

    scroll.entrys = entrys;
    scroll.checkScroll();

    if (oldLocation.id != focus.id || oldLocation.modul != focus.modul || oldLocation.type != focus.type) {
        scroll.resetScroll();
        oldLocation = focus;
    }
}

void GUIPanelFocus::registerModuleSettings() {
    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    size = allLayers[focus.layer]->getModules()[focus.modul]->getPotis().size();

    dataIndex = scroll.offset;

    while (true) {
        if (elementIndex >= FOCUSPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {
            Analog *analogElement = allLayers[focus.layer]->getModules()[focus.modul]->getPotis()[dataIndex];

            if (analogElement->displayVis) { // element Visible

                // register newFocusLocation
                if ((scroll.position - scroll.offset) == elementIndex) {
                    if (analogElement->input != nullptr) {
                        newFocusLocation.id = analogElement->input->id;
                        newFocusLocation.modul = analogElement->input->moduleId;
                        newFocusLocation.type = FOCUSINPUT;
                    }
                }

                panelElements[elementIndex].addAnalogEntry(

                    analogElement, {std::bind(&Analog::changeValue, analogElement, 100), "AMOUNT"},
                    {std::bind(&Analog::changeValue, analogElement, -100), "AMOUNT"},
                    {std::bind(&Analog::resetValue, analogElement), "RESET"});

                dataIndex++;
                elementIndex++;
            }
            else {
                dataIndex++;
            }
        }
        else {
            break;
        }
    }

    size = allLayers[focus.layer]->getModules()[focus.modul]->getSwitches().size();
    dataIndex = 0;

    while (true) {
        if (elementIndex >= FOCUSPANELENTRYS) {
            break;
        }
        for (int x = 0; x < SwitchEntrysPerElement; x++) {

            if (dataIndex < size) {
                Digital *digitalElement = allLayers[focus.layer]->getModules()[focus.modul]->getSwitches()[dataIndex];

                panelElements[elementIndex].addDigitalEntry(
                    digitalElement, {std::bind(&Digital::nextValue, digitalElement), "NEXT"},
                    {std::bind(&Digital::previousValue, digitalElement), "NEXT"},
                    {std::bind(&Digital::resetValue, digitalElement), "RESET"});
                dataIndex++;
            }
            else {
                panelElements[elementIndex].addEmptyEntry(); // empty entry
            }
        }
        elementIndex++;
    }

    panelElements[scroll.position - scroll.offset].select = 1;
}

void GUIPanelFocus::registerModulePatchIn() {
    // println("registerModulePatchIn");

    uint16_t size = allLayers[focus.layer]->getModules()[focus.modul]->getInputs()[focus.id]->getPatchesInOut().size();

    uint16_t dataIndex;

    while (true) {
        for (unsigned int i = 0; i < FOCUSPANELENTRYS; i++) {

            dataIndex = i + scroll.offset;
            if (dataIndex < size) {

                // println("register PatchElement");
                PatchElementInOut *patchElement = allLayers[focus.layer]
                                                      ->getModules()[focus.modul]
                                                      ->getInputs()[focus.id]
                                                      ->getPatchesInOut()[dataIndex];

                panelElements[i].addPatchOutputEntry(
                    patchElement, {std::bind(&PatchElementInOut::changeAmount, patchElement, 0.02), "AMOUNT"},
                    {std::bind(&PatchElementInOut::changeAmount, patchElement, -0.02), "AMOUNT"},
                    {std::bind(&PatchElementInOut::setAmount, patchElement, 0), "RESET"});
            }
            if (scroll.position == (i + scroll.offset)) {
                panelElements[i].select = 1;
            }
            else {
                panelElements[i].select = 0;
            }
        }

        break;
    }
}
void GUIPanelFocus::registerLayerModules() {
    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    size = allLayers[focus.layer]->getModules().size();

    dataIndex = scroll.offset;

    while (true) {
        if (elementIndex >= FOCUSPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {
            BaseModule *moduleElement = allLayers[focus.layer]->getModules()[dataIndex];

            panelElements[elementIndex].addModuleEntry(moduleElement, {nullptr, ""}, {nullptr, ""}, {nullptr, ""});

            if ((scroll.position - scroll.offset) == elementIndex) {
                newFocusLocation.modul = moduleElement->id;
                newFocusLocation.type = FOCUSMODULE;
            }

            dataIndex++;
            elementIndex++;
        }
        else {
            break;
        }
    }
    panelElements[scroll.position - scroll.offset].select = 1;
}

void GUIPanelFocus::registerModulePatchOut() {
    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    size = allLayers[focus.layer]->getModules()[focus.modul]->getOutputs()[focus.id]->getPatchesInOut().size();

    dataIndex = scroll.offset;

    while (true) {
        if (elementIndex >= FOCUSPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {
            PatchElementInOut *patchElement =
                allLayers[focus.layer]->getModules()[focus.modul]->getOutputs()[focus.id]->getPatchesInOut()[dataIndex];

            panelElements[elementIndex].addPatchInputEntry(
                patchElement, {std::bind(&PatchElementInOut::changeAmount, patchElement, 0.02), "AMOUNT"},
                {std::bind(&PatchElementInOut::changeAmount, patchElement, -0.02), "AMOUNT"},
                {std::bind(&PatchElementInOut::setAmount, patchElement, 0), "RESET"});

            dataIndex++;
            elementIndex++;
        }
        else {
            break;
        }
    }

    dataIndex = 0;

    size = allLayers[focus.layer]->getModules()[focus.modul]->getOutputs()[focus.id]->getPatchesOutOut().size();

    while (true) {
        if (elementIndex >= FOCUSPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {

            PatchElementOutOut *patchElement = allLayers[focus.layer]
                                                   ->getModules()[focus.modul]
                                                   ->getOutputs()[focus.id]
                                                   ->getPatchesOutOut()[dataIndex];

            panelElements[elementIndex].addPatchOutOutEntry(
                patchElement, {std::bind(&PatchElementOutOut::changeAmount, patchElement, 0.02), "AMOUNT"},
                {std::bind(&PatchElementOutOut::changeAmount, patchElement, -0.02), "AMOUNT"},
                {std::bind(&PatchElementOutOut::setAmount, patchElement, 0), "RESET"});

            dataIndex++;
            elementIndex++;
        }
        else {
            break;
        }
    }
    panelElements[scroll.position - scroll.offset].select = 1;
}

#endif