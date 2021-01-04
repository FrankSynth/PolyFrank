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

void drawSettingElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    if (entry->setting == nullptr) {
        println("nullpt!");
        return;
    }
    Setting *data = entry->setting;
    uint16_t nameWidth = w;
    uint16_t valueBarHeigth = 6;
    uint16_t dataHeight = h - valueBarHeigth;

    uint16_t fontShiftHeight = h / 4;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cWhite, x + 2, y, nameWidth - 4, dataHeight / 2, 1);
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-elementFont->size + dataHeight) / 2 - fontShiftHeight,
                   elementFont, CENTER);
    }
    else {
        drawRectangleChampfered(cWhiteLight, x + 2, y, nameWidth - 4, dataHeight / 2, 1);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-elementFont->size + dataHeight) / 2 - fontShiftHeight,
                   elementFont, CENTER);
    }

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + w / 2, y + (-elementFont->size + dataHeight) / 2 + fontShiftHeight,
               elementFont,
               CENTER); // center Text

    // valueBar
    // drawRectangleChampfered(cGreyLight, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);

    uint16_t relY = h - valueBarHeigth;
    uint16_t valueBarWidth = w - 8;

    valueBarWidth = (float)valueBarWidth * ((float)data->value - data->min) / (float)(data->max - data->min);
    drawRectangleChampfered(cWhite, x + 4, relY + y, valueBarWidth, valueBarHeigth, 1);
}

void drawNameElement(std::string *name, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    // clear
    if (name != nullptr) {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        // get text

        // Draw Name
        if (select) {
            drawRectangleChampfered(cWhite, x, y, w, h, 1);
            drawString(*name, cFont_Select, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
        }
        else {
            drawRectangleFill(cWhite, x + w - 1, y + 2, 1, h - 4);
            drawString(*name, cFont_Deselect, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
        }
    }
}

void drawBasePatchElement(BasePatch *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                          uint8_t patched) {

    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
        drawString(element->name, cFont_Select, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        drawString(element->name, cFont_Deselect, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }

    if (patched == 1) {
        drawRectangleChampfered(cWhite, x - 7, y, 5, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cWhite, x + w + 2, y, 5, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cWhite, x - 7, y, 5, h, 1);
        drawRectangleChampfered(cWhite, x + w + 2, y, 5, h, 1);
    }
}

void drawModuleElement(BaseModule *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                       uint8_t patched) {

    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
        drawString(element->name, cFont_Select, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);
        drawString(element->name, cFont_Deselect, x + w / 2, y + (-elementFont->size + h) / 2, elementFont, CENTER);
    }

    if (patched == 1) {
        drawRectangleChampfered(cWhite, x - 7, y, 5, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cWhite, x + w + 2, y, 5, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cWhite, x - 7, y, 5, h, 1);
        drawRectangleChampfered(cWhite, x + w + 2, y, 5, h, 1);
    }
}

void drawDigitalElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    x += 2;
    w -= 4;

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
        drawRectangleChampfered(cWhiteMedium, x, y, nameWidth, h, 1);
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

void Data_PanelElement::setName(std::string *name) {
    this->panelElementName = name;
}

void Data_PanelElement::Draw() {
    uint16_t relX = 0;
    uint16_t relY = 0;

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
    // wir nehmen an das wenn das erste element ein Setting ist, alle elemente Settings sind
    if (entrys[0].type == SETTING) {
        entryWidth = width / (numberEntrys + 1);

        // Feld mit GruppenNamen
        drawNameElement(panelElementName, relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
        relX += entryWidth;

        for (int x = 0; x < numberEntrys; x++) {
            if (entrys[x].type == EMPTY) {
            }
            else {
                drawSettingElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
            }

            relX += entryWidth;
            entrys[x].functionCW = {nullptr, ""};
            entrys[x].functionCCW = {nullptr, ""};
            entrys[x].functionPush = {nullptr, ""};
        }
    }

    else {

        entryWidth = width / numberEntrys;
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
    }

    numberEntrys = 0;
    select = 0;
}

void Patch_PanelElement::Draw() {
    if (!active) {
        return;
    }

    if (select) {
        // actionHandler.registerActionEncoder4(entrys[2].functionCW, entrys[2].functionCCW,
        // entrys[2].functionPush);
    }

    drawBasePatchElement(entry, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched);

    // reset Marker
    select = 0;
    active = 0;
    patched = 0;
}

void Module_PanelElement::Draw() {
    if (!active) {
        return;
    }
    if (select) {
        // actionHandler.registerActionEncoder4(entrys[2].functionCW, entrys[2].functionCCW, entrys[2].functionPush);
    }

    drawModuleElement(entry, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched);

    // reset Marker
    select = 0;
    active = 0;
    patched = 0;
}

void GUIPanelData::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {

    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

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
                PatchElementInOut *patchElement = allLayers[focus.layer]
                                                      ->getModules()[focus.modul]
                                                      ->getInputs()[focus.id]
                                                      ->getPatchesInOut()[dataIndex];

                panelElements[i].addPatchOutputEntry(
                    patchElement, {std::bind(&PatchElementInOut::changeAmount, patchElement, 0.02), "AMOUNT"},
                    {std::bind(&PatchElementInOut::changeAmount, patchElement, -0.02), "AMOUNT"},
                    {std::bind(&PatchElementInOut::setAmount, patchElement, 0), "RESET"});
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
        if (elementIndex >= DATAPANELENTRYS) {
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

void GUIPanelConfig::registerGlobalSettings() {
    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;
    uint8_t exit = false;

    size = globalSettings.__globSettingsMIDI.settings.size();

    while (true) {
        if (elementIndex >= CONFIGPANELENTRYS) {
            break;
        }
        panelElements[elementIndex].setName(&globalSettings.__globSettingsMIDI.category);

        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = globalSettings.__globSettingsMIDI.settings[dataIndex];

                panelElements[elementIndex].addSettingsEntry(
                    settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                    {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
                    {std::bind(&Setting::resetValue, settingsElement), "RESET"});
                dataIndex++;
            }
            else {
                panelElements[elementIndex].addEmptyEntry(); // empty entry
                exit = true;
            }
        }
        elementIndex++;
        if (exit)
            break;
    }
    size = globalSettings.__globSettingsSystem.settings.size();
    dataIndex = 0;
    exit = false;

    while (true) {
        if (elementIndex >= CONFIGPANELENTRYS) {
            break;
        }
        panelElements[elementIndex].setName(&globalSettings.__globSettingsSystem.category);

        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = globalSettings.__globSettingsSystem.settings[dataIndex];

                panelElements[elementIndex].addSettingsEntry(
                    settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                    {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
                    {std::bind(&Setting::resetValue, settingsElement), "RESET"});
                dataIndex++;
            }
            else {
                panelElements[elementIndex].addEmptyEntry(); // empty entry
                exit = true;
            }
        }
        elementIndex++;
        if (exit)
            break;
    }
    size = globalSettings.__globSettingsDisplay.settings.size();
    dataIndex = 0;
    exit = false;

    while (true) {
        if (elementIndex >= CONFIGPANELENTRYS) {
            break;
        }
        panelElements[elementIndex].setName(&globalSettings.__globSettingsDisplay.category);

        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = globalSettings.__globSettingsDisplay.settings[dataIndex];

                panelElements[elementIndex].addSettingsEntry(
                    settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                    {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
                    {std::bind(&Setting::resetValue, settingsElement), "RESET"});
                dataIndex++;
            }
            else {
                panelElements[elementIndex].addEmptyEntry(); // empty entry
                exit = true;
            }
        }
        elementIndex++;
        if (exit)
            break;
    }

    panelElements[scroll - scrollOffset].select = 1;
}

void GUIPanelConfig::changeScroll(int16_t change) {

    if (scroll + change != 0) {
        scroll = testInt(scroll + change, 0, entrys - 1);
    }
    else
        scroll = 0;

    if (scroll >= entrys) {
        scroll = 0;
    }

    if (entrys <= CONFIGPANELENTRYS) {
        scrollOffset = 0;
    }
    else if (scroll == entrys - 1) {
        scrollOffset = entrys - CONFIGPANELENTRYS;
    }
    else if (scroll == 0) {
        scrollOffset = 0;
    }

    else if (scroll >= (CONFIGPANELENTRYS + scrollOffset - 1)) {
        scrollOffset++;
    }
    else if (scroll < (scrollOffset + 1)) {
        scrollOffset--;
    }
}

void GUIPanelConfig::checkScroll() {
    changeScroll(0);
}

void GUIPanelConfig::resetScroll() {
    scroll = 0;
    scrollOffset = 0;
}

uint16_t GUIPanelConfig::updateEntrys() {
    entrys = 0;
    entrys += ceil((float)globalSettings.__globSettingsSystem.settings.size() / EntrysPerElement);

    entrys += ceil((float)globalSettings.__globSettingsDisplay.settings.size() / EntrysPerElement);
    entrys += ceil((float)globalSettings.__globSettingsMIDI.settings.size() / EntrysPerElement);

    return entrys;
}

void GUIPanelConfig::Draw() {

    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    // check Scroll position
    checkScroll();

    registerGlobalSettings();

    for (int i = 0; i < CONFIGPANELENTRYS; i++) {
        panelElements[i].Draw();
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scrollOffset, entrys,
                  CONFIGPANELENTRYS);
}

void GUIPanelConfig::registerPanelSettings() {

    actionHandler.registerActionEncoder1({std::bind(&GUIPanelConfig::changeScroll, this, 1), "SCROLL"},
                                         {std::bind(&GUIPanelConfig::changeScroll, this, -1), "SCROLL"}, {nullptr, ""});

    // register Panel Seetings Left
    actionHandler.registerActionLeft({nullptr, "RESET"}, // RESET
                                     {nullptr, ""}, {nullptr, ""});

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({nullptr, "SAVE"}, // SAVE
                                      {nullptr, ""}, {nullptr, ""});
}

void GUIPanelConfig::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id) {

    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;

    // elements Sizes
    uint16_t elementWidth = width - SCROLLBARWIDTH - 2;
    uint16_t elementSpace = 3;
    uint16_t elementHeight = (height - (CONFIGPANELENTRYS - 2) * elementSpace) / CONFIGPANELENTRYS;

    // init Elements
    for (int i = 0; i < CONFIGPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }
}

////////////////

void GUIPanelPatch::registerElements() {
    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    dataIndex = scrollModuleOffset;

    // register Module Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrysModule) {

            panelElementsModule[elementIndex].addEntry(allLayers[focus.layer]->getModules()[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }

        elementIndex++;
    }
    panelElementsModule[scrollModule - scrollModuleOffset].select = 1;

    elementIndex = 0;
    dataIndex = scrollSourceOffset;
    // register Source Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrysSource) {

            panelElementsSource[elementIndex].addEntry((BasePatch *)allLayers[focus.layer]->outputs[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }

        elementIndex++;
    }
    panelElementsSource[scrollSource - scrollSourceOffset].select = 1;

    elementIndex = 0;
    dataIndex = scrollTargetOffset;
    // register Target Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrysTarget) {

            panelElementsTarget[elementIndex].addEntry(
                (BasePatch *)allLayers[focus.layer]->getModules()[scrollModule]->getInputs()[dataIndex]);
            dataIndex++;
        }
        else {
            break;
        }

        elementIndex++;
    }
    panelElementsTarget[scrollTarget - scrollTargetOffset].select = 1;

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
                for (PatchElementInOut *p :
                     panelElementsSource[scrollSource - scrollSourceOffset].entry->getPatchesInOut()) {

                    if (panelElementsModule[i].entry->id == p->targetIn->moduleId) {
                        panelElementsModule[i].patched = 1;
                    }
                }
            }
        }
        for (int i = 0; i < maxEntrys; i++) {

            if (panelElementsModule[scrollModule - scrollModuleOffset].patched) {
                if (i < entrysTarget) {

                    for (PatchElementInOut *p : panelElementsTarget[i].entry->getPatchesInOut()) {

                        if (panelElementsModule[scrollModule - scrollModuleOffset].entry->id == p->targetIn->moduleId) {
                            panelElementsModule[scrollModule - scrollModuleOffset].patched = 3;
                            panelElementsTarget[i].patched = 1;
                        }
                    }
                }
            }
        }
    }
    else {
        // Liegt auf dem target ein Output?  -> makierung rechts

        // für jeden Eintrag
        for (int i = 0; i < maxEntrys; i++) {

            if (panelElementsTarget[i].entry->getPatchesInOut().size()) {
                panelElementsTarget[i].patched = 2;
            }
            // Ist die Source auf den aktuellen target connected -> makierung links
            for (PatchElementInOut *p :
                 panelElementsTarget[scrollTarget - scrollTargetOffset].entry->getPatchesInOut()) {

                if (panelElementsSource[i].entry->idGlobal == p->sourceOut->idGlobal) {
                    panelElementsSource[i].patched = 1;
                }
            }
        }
    }
}

void GUIPanelPatch::changeScroll(int16_t changeModul, int16_t changeTarget, int16_t changeSource) {

    if (scrollTarget + changeTarget != 0) {
        scrollTarget = testInt(scrollTarget + changeTarget, 0, entrysTarget - 1);
    }
    else
        scrollTarget = 0;

    if (scrollTarget >= entrysTarget) {
        scrollTarget = 0;
    }

    if (entrysTarget <= maxEntrys) {
        scrollTargetOffset = 0;
    }
    else if (scrollTarget == entrysTarget - 1) {
        scrollTargetOffset = entrysTarget - maxEntrys;
    }
    else if (scrollTarget == 0) {
        scrollTargetOffset = 0;
    }

    else if (scrollTarget >= (maxEntrys + scrollTargetOffset - 1)) {
        scrollTargetOffset++;
    }
    else if (scrollTarget < (scrollTargetOffset + 1)) {
        scrollTargetOffset--;
    }

    ///
    if (scrollSource + changeSource != 0) {
        scrollSource = testInt(scrollSource + changeSource, 0, entrysSource - 1);
    }
    else
        scrollSource = 0;

    if (scrollSource >= entrysSource) {
        scrollSource = 0;
    }

    if (entrysSource <= maxEntrys) {
        scrollSourceOffset = 0;
    }
    else if (scrollSource == entrysSource - 1) {
        scrollSourceOffset = entrysSource - maxEntrys;
    }
    else if (scrollSource == 0) {
        scrollSourceOffset = 0;
    }

    else if (scrollSource >= (maxEntrys + scrollSourceOffset - 1)) {
        scrollSourceOffset++;
    }
    else if (scrollSource < (scrollSourceOffset + 1)) {
        scrollSourceOffset--;
    }

    ///
    if (scrollModule + changeModul != 0) {
        scrollModule = testInt(scrollModule + changeModul, 0, entrysModule - 1);
    }
    else
        scrollModule = 0;

    if (scrollModule >= entrysModule) {
        scrollModule = 0;
    }

    if (entrysModule <= maxEntrys) {
        scrollModuleOffset = 0;
    }
    else if (scrollModule == entrysModule - 1) {
        scrollModuleOffset = entrysModule - maxEntrys;
    }
    else if (scrollModule == 0) {
        scrollModuleOffset = 0;
    }

    else if (scrollModule >= (maxEntrys + scrollModuleOffset - 1)) {
        scrollModuleOffset++;
    }
    else if (scrollModule < (scrollModuleOffset + 1)) {
        scrollModuleOffset--;
    }
}

void GUIPanelPatch::checkScroll() {
    changeScroll(0, 0, 0);
}

void GUIPanelPatch::resetScroll() {
    // scroll = 0;
    // scrollOffset = 0;
}

void GUIPanelPatch::updateEntrys() {

    entrysModule = allLayers[focus.layer]->getModules().size();
    entrysTarget = allLayers[focus.layer]->getModules()[scrollModule]->getInputs().size();
    entrysSource = allLayers[focus.layer]->outputs.size();
}

void GUIPanelPatch::Draw() {

    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    // check Scroll position
    checkScroll();

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

    // drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scrollOffset,
    // entrys,
    //               CONFIGPANELENTRYS);
}

void GUIPanelPatch::registerPanelSettings() {

    if (flipView) {

        actionHandler.registerActionEncoder2({std::bind(&GUIPanelPatch::changeScroll, this, 1, 0, 0), "SCROLL"},
                                             {std::bind(&GUIPanelPatch::changeScroll, this, -1, 0, 0), "SCROLL"},
                                             {nullptr, ""});
        actionHandler.registerActionEncoder3({std::bind(&GUIPanelPatch::changeScroll, this, 0, 1, 0), "SCROLL"},
                                             {std::bind(&GUIPanelPatch::changeScroll, this, 0, -1, 0), "SCROLL"},
                                             {nullptr, ""});
        actionHandler.registerActionEncoder1({std::bind(&GUIPanelPatch::changeScroll, this, 0, 0, 1), "SCROLL"},
                                             {std::bind(&GUIPanelPatch::changeScroll, this, 0, 0, -1), "SCROLL"},
                                             {nullptr, ""});
    }
    else {
        actionHandler.registerActionEncoder1({std::bind(&GUIPanelPatch::changeScroll, this, 1, 0, 0), "SCROLL"},
                                             {std::bind(&GUIPanelPatch::changeScroll, this, -1, 0, 0), "SCROLL"},
                                             {nullptr, ""});
        actionHandler.registerActionEncoder2({std::bind(&GUIPanelPatch::changeScroll, this, 0, 1, 0), "SCROLL"},
                                             {std::bind(&GUIPanelPatch::changeScroll, this, 0, -1, 0), "SCROLL"},
                                             {nullptr, ""});
        actionHandler.registerActionEncoder3({std::bind(&GUIPanelPatch::changeScroll, this, 0, 0, 1), "SCROLL"},
                                             {std::bind(&GUIPanelPatch::changeScroll, this, 0, 0, -1), "SCROLL"},
                                             {nullptr, ""});
    }
    // register Panel Seetings Left
    actionHandler.registerActionLeft({std::bind(&GUIPanelPatch::toggleFlipView, this), "FLIP"}, // RESET
                                     {std::bind(&GUIPanelPatch::clearPatches, this), "CLEAR"},
                                     {std::bind(nextLayer), "LAYER"});

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({std::bind(&GUIPanelPatch::addCurrentPatch, this), "ADD"}, // SAVE
                                      {std::bind(&GUIPanelPatch::removeCurrentPatch, this), "REMOVE"}, {nullptr, ""});
}

void GUIPanelPatch::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;

    uint16_t elementSpaceX = 40;
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
    allLayers[focus.layer]->addPatchInOutById(panelElementsSource[scrollSource - scrollSourceOffset].entry->idGlobal,
                                              panelElementsTarget[scrollTarget - scrollTargetOffset].entry->idGlobal);
}

void GUIPanelPatch::removeCurrentPatch() {
    allLayers[focus.layer]->removePatchInOutById(
        panelElementsSource[scrollSource - scrollSourceOffset].entry->idGlobal,
        panelElementsTarget[scrollTarget - scrollTargetOffset].entry->idGlobal);
}

void GUIPanelPatch::clearPatches() {
    allLayers[focus.layer]->clearPatches();
}

#endif