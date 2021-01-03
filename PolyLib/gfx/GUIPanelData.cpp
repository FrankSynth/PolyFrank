#ifdef POLYCONTROL

#include "GUIPanelData.hpp"

const GUI_FONTINFO *elementFont = &GUI_FontBahnschrift24_FontInfo;

void drawPatchInOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {
    PatchElementOutOut *dataOutOut = nullptr;
    PatchElementInOut *data = nullptr;
    uint8_t outOutFlag = 0;

    if (entry->type == PATCHOUTOUT) {
        dataOutOut = (PatchElementOutOut *)entry->patch;
        outOutFlag = 1;
    }
    else {
        data = (PatchElementInOut *)entry->patch;
    }
    uint16_t nameWidth = 140;

    uint16_t spaceLeftRightBar = 50;
    uint16_t spaceTopBottomBar = 8;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);
    std::string text;
    // get text
    if (entry->type == PATCHOUTPUT) {
        text = allLayers[focus.layer]->getModules()[data->sourceOut->moduleId]->getName();
    }
    else if (entry->type == PATCHOUTOUT) {
        text = allLayers[focus.layer]->getModules()[dataOutOut->sourceOut->moduleId]->getName();
    }
    else {
        text = allLayers[focus.layer]->getModules()[data->targetIn->moduleId]->getName();
    }
    // Draw Name
    if (select) {
        drawRectangleChampfered(cWhite, x, y, nameWidth, h, 1);
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
    else {
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }

    uint16_t valueBarWidth = w - nameWidth - 2 * spaceLeftRightBar;

    uint16_t valueBarHeigth = h - 2 * spaceTopBottomBar;

    uint16_t relX = nameWidth + 15;
    uint16_t relY = spaceTopBottomBar;

    // valueBar
    // drawRectangleChampfered(cGreyLight, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);

    int16_t valueBaroffsetCenter = 0;
    float amount;
    if (entry->type == PATCHOUTOUT) {

        amount = dataOutOut->getAmount();
    }
    else {
        amount = data->getAmount();
    }

    if (amount < 0) {
        valueBaroffsetCenter = (valueBarWidth + (float)valueBarWidth * amount) / 2;
    }
    else {
        valueBaroffsetCenter = valueBarWidth / 2;
    }
    if (outOutFlag) {
        text = "MULT";
        drawString(text, cFont_Deselect, x + relX + valueBarWidth + 12, y + (-elementFont->size + h) / 2, elementFont,
                   LEFT);
    }
    else {
        text = "ADD";
        drawString(text, cFont_Deselect, x + relX + valueBarWidth + 15, y + (-elementFont->size + h) / 2, elementFont,
                   LEFT);
    }

    drawRectangleChampfered(cWhite, relX + x + valueBaroffsetCenter, relY + y, (float)valueBarWidth / 2 * abs(amount),
                            valueBarHeigth, 1);

    drawRectangleFill(cWhite, relX + x + valueBarWidth / 2 - 1, relY + y + 2, 1, valueBarHeigth - 4);
}

void drawDigitalElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    Digital *data = entry->digital;
    uint16_t nameWidth = w / 2;

    uint16_t heightBar = 8;
    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cWhite, x, y, nameWidth, h, 1);
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
    else {
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }

    uint16_t valueBarWidth = w - nameWidth - 4;
    uint16_t valueBarHeigth = heightBar;

    uint16_t relX = nameWidth;
    uint16_t relY = h - heightBar;

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + relX + valueBarWidth / 2, y + (-elementFont->size + h) / 2, elementFont,
               CENTER); // center Text

    // valueBar
    // drawRectangleChampfered(cGreyLight, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);

    valueBarWidth = (float)valueBarWidth * ((float)data->valueMapped - data->min) / (float)(data->max - data->min);

    drawRectangleChampfered(cWhite, relX + x + 2, relY + y, valueBarWidth, valueBarHeigth, 1);
}

void drawAnalogElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    Analog *data = entry->analog;
    uint16_t nameWidth = 140;

    uint16_t spaceLeftRightBar = 15;
    uint16_t spaceTopBottomBar = 8;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cWhite, x, y, nameWidth, h, 1);
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
    else {
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }

    uint16_t valueBarWidth = w - nameWidth - 2 * spaceLeftRightBar;
    uint16_t valueBarHeigth = h - 2 * spaceTopBottomBar;

    uint16_t relX = nameWidth + spaceLeftRightBar;
    uint16_t relY = spaceTopBottomBar;

    // valueBar
    // drawRectangleChampfered(cGreyLight, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);

    valueBarWidth = (float)valueBarWidth * data->valueMapped / (float)(data->max - data->min);

    drawRectangleChampfered(cWhite, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);
}

void drawModuleElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    BaseModule *data = entry->modules;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
        drawString(text, cFont_Select, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
    else {
        drawString(text, cFont_Deselect, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
}

void Data_PanelElement::Draw() {
    uint16_t relX = 0;
    uint16_t relY = 0;

    entryWidth = width / numberEntrys;
    entryHeight = heigth;

    if (!visible) {
        drawRectangleFill(cClear, panelAbsX, panelAbsY, width, heigth);

        return;
    }

    if (select) {
        actionHandler.registerActionEncoder2(entrys[0].functionCW, entrys[0].functionCCW, entrys[0].functionPush);
        actionHandler.registerActionEncoder3(entrys[1].functionCW, entrys[1].functionCCW, entrys[1].functionPush);
        actionHandler.registerActionEncoder4(entrys[2].functionCW, entrys[2].functionCCW, entrys[2].functionPush);
    }

    for (int x = 0; x < numberEntrys; x++) {
        if (entrys[x].type == EMPTY) {
        }

        else if (entrys[x].type == ANALOG) {
            drawAnalogElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        }
        else if (entrys[x].type == DIGITAL) {
            drawDigitalElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        }
        else if (entrys[x].type == PATCHINPUT) {
            drawPatchInOutElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        }
        else if (entrys[x].type == PATCHOUTPUT) {
            drawPatchInOutElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        }
        else if (entrys[x].type == PATCHOUTOUT) {
            drawPatchInOutElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        }
        else if (entrys[x].type == MODULE) {
            drawModuleElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        }

        entrys[x].functionCW = {nullptr, ""};
        entrys[x].functionCCW = {nullptr, ""};
        entrys[x].functionPush = {nullptr, ""};

        relX += entryWidth;
    }

    numberEntrys = 0;
    select = 0;
}

void GUIPanelData::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id) {

    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

    this->name = name;
    this->id = id;

    // elements Sizes
    uint16_t elementWidth = width - SCROLLBARWIDTH - 2;
    uint16_t elementSpace = 3;
    uint16_t elementHeight = (height - (DATAPANELENTRYS - 2) * elementSpace) / DATAPANELENTRYS;

    // init Elements
    for (int i = 0; i < DATAPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }
}

void GUIPanelData::registerPanelSettings() {

    // register Scroll
    if (newFocusLocation.type != NOFOCUS) {
        actionHandler.registerActionEncoder1({std::bind(&GUIPanelData::changeScroll, this, 1), "SCROLL"},
                                             {std::bind(&GUIPanelData::changeScroll, this, -1), "SCROLL"},
                                             {std::bind(focusDown, newFocusLocation), "FOCUS"});
    }
    else {
        actionHandler.registerActionEncoder1({std::bind(&GUIPanelData::changeScroll, this, 1), "SCROLL"},
                                             {std::bind(&GUIPanelData::changeScroll, this, -1), "SCROLL"},
                                             {nullptr, ""});
    }

    // register Panel Seetings Left
    actionHandler.registerActionLeft({nullptr, ""}, {std::bind(focusUp), "UP"}, // focus Up
                                     {std::bind(nextLayer), "LAYER"});          // Layer Switch

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({nullptr, ""}, {std::bind(Todo), "RESET"}, {std::bind(Todo), "PATCH"});
}

void GUIPanelData::Draw() {

    // register Panel Seetings
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    // check Scroll position
    checkScroll();

    // resetFocus
    newFocusLocation.type = NOFOCUS;

    if (focus.type == FOCUSMODULE) {
        registerModuleSettings();
        for (int i = 0; i < DATAPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSOUTPUT) {
        registerModulePatchOut();
        for (int i = 0; i < DATAPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSINPUT) {
        registerModulePatchIn();
        for (int i = 0; i < DATAPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSLAYER) {
        registerLayerModules();
        for (int i = 0; i < DATAPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }
    else if (focus.type == FOCUSCONFIG) {
        // registerConfigs();
        for (int i = 0; i < DATAPANELENTRYS; i++) {
            panelElements[i].Draw();
        }
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scrollOffset, entrys,
                  DATAPANELENTRYS);
}

uint16_t GUIPanelData::updateEntrys() {
    entrys = 0;

    if (focus.type == FOCUSINPUT) {
        entrys = 0; // BaseValue
        entrys += allLayers[focus.layer]
                      ->getModules()[focus.modul]
                      ->getInputs()[focus.id]
                      ->getPatchesInOut()
                      .size(); // all patches to this input

        // println("entrys Input", entrys);
        return entrys;
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

        return entrys;
    }
    else if (focus.type == FOCUSSETTING) {
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
        return entrys;
    }
    else if (focus.type == FOCUSLAYER) {
        return entrys = allLayers[focus.layer]->getModules().size(); // all Modules
    }

    return 0;
}

void GUIPanelData::registerModuleSettings() {

    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    size = allLayers[focus.layer]->getModules()[focus.modul]->getPotis().size();

    dataIndex = scrollOffset;

    while (true) {
        if (elementIndex >= DATAPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {
            Analog *analogElement = allLayers[focus.layer]->getModules()[focus.modul]->getPotis()[dataIndex];

            if (analogElement->displayVis) { // element Visible

                // register newFocusLocation
                if ((scroll - scrollOffset) == elementIndex) {
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
        if (elementIndex >= DATAPANELENTRYS) {
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

    panelElements[scroll - scrollOffset].select = 1;
}

void GUIPanelData::registerModulePatchIn() {
    // println("registerModulePatchIn");

    uint16_t size = allLayers[focus.layer]->getModules()[focus.modul]->getInputs()[focus.id]->getPatchesInOut().size();

    uint16_t dataIndex;

    while (true) {
        for (unsigned int i = 0; i < DATAPANELENTRYS; i++) {

            dataIndex = i + scrollOffset;
            if (dataIndex < size) {

                // println("register PatchElement");
                PatchElement *patchElement = allLayers[focus.layer]
                                                 ->getModules()[focus.modul]
                                                 ->getInputs()[focus.id]
                                                 ->getPatchesInOut()[dataIndex];

                panelElements[i].addPatchOutputEntry(
                    patchElement, {std::bind(&PatchElement::changeAmount, patchElement, 0.02), "AMOUNT"},
                    {std::bind(&PatchElement::changeAmount, patchElement, -0.02), "AMOUNT"},
                    {std::bind(&PatchElement::resetAmount, patchElement), "RESET"});
            }
            if (scroll == (i + scrollOffset)) {
                panelElements[i].select = 1;
            }
            else {
                panelElements[i].select = 0;
            }
        }

        break;
    }
}
void GUIPanelData::registerLayerModules() {

    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    size = allLayers[focus.layer]->getModules().size();

    dataIndex = scrollOffset;

    while (true) {
        if (elementIndex >= DATAPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {
            BaseModule *moduleElement = allLayers[focus.layer]->getModules()[dataIndex];

            panelElements[elementIndex].addModuleEntry(moduleElement, {nullptr, ""}, {nullptr, ""}, {nullptr, ""});

            if ((scroll - scrollOffset) == elementIndex) {
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
    panelElements[scroll - scrollOffset].select = 1;
}

void GUIPanelData::registerModulePatchOut() {

    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    size = allLayers[focus.layer]->getModules()[focus.modul]->getOutputs()[focus.id]->getPatchesInOut().size();

    dataIndex = scrollOffset;

    while (true) {
        if (elementIndex >= DATAPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {
            PatchElement *patchElement =
                allLayers[focus.layer]->getModules()[focus.modul]->getOutputs()[focus.id]->getPatchesInOut()[dataIndex];

            panelElements[elementIndex].addPatchInputEntry(
                patchElement, {std::bind(&PatchElement::changeAmount, patchElement, 0.02), "AMOUNT"},
                {std::bind(&PatchElement::changeAmount, patchElement, -0.02), "AMOUNT"},
                {std::bind(&PatchElement::resetAmount, patchElement), "RESET"});

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
        if (elementIndex >= DATAPANELENTRYS) {
            break;
        }

        if (dataIndex < size) {

            PatchElement *patchElement = allLayers[focus.layer]
                                             ->getModules()[focus.modul]
                                             ->getOutputs()[focus.id]
                                             ->getPatchesOutOut()[dataIndex];

            panelElements[elementIndex].addPatchOutOutEntry(
                patchElement, {std::bind(&PatchElement::changeAmount, patchElement, 0.02), "AMOUNT"},
                {std::bind(&PatchElement::changeAmount, patchElement, -0.02), "AMOUNT"},
                {std::bind(&PatchElement::resetAmount, patchElement), "RESET"});

            dataIndex++;
            elementIndex++;
        }
        else {
            break;
        }
    }
    panelElements[scroll - scrollOffset].select = 1;
}

void GUIPanelData::changeScroll(int16_t change) {

    if (scroll + change != 0) {
        scroll = testInt(scroll + change, 0, entrys - 1);
    }
    else
        scroll = 0;

    if (scroll >= entrys) {
        scroll = 0;
    }

    if (entrys <= DATAPANELENTRYS) {
        scrollOffset = 0;
    }
    else if (scroll == entrys - 1) {
        scrollOffset = entrys - DATAPANELENTRYS;
    }
    else if (scroll == 0) {
        scrollOffset = 0;
    }

    else if (scroll >= (DATAPANELENTRYS + scrollOffset - 1)) {
        scrollOffset++;
    }
    else if (scroll < (scrollOffset + 1)) {
        scrollOffset--;
    }
}

void GUIPanelData::checkScroll() {
    static location BackupForCompare = focus;

    if (focus.modul != BackupForCompare.modul || focus.id != BackupForCompare.id ||
        focus.type != BackupForCompare.type) {
        resetScroll();
        BackupForCompare = focus;
    }
    else {
        changeScroll(0);
    }
}

void GUIPanelData::resetScroll() {
    scroll = 0;
    scrollOffset = 0;
}

// Header
#endif