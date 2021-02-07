#ifdef POLYCONTROL

#include "guiPanelBase.hpp"

const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift12_FontInfo;
const GUI_FONTINFO *fontMedium = &GUI_FontBahnschrift24_FontInfo;
const GUI_FONTINFO *fontBig = &GUI_FontBahnschrift32_FontInfo;

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
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
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
        drawString(text, cFont_Deselect, x + relX + valueBarWidth + 12, y + (-fontMedium->size + h) / 2, fontMedium,
                   LEFT);
    }
    else {
        text = "ADD";
        drawString(text, cFont_Deselect, x + relX + valueBarWidth + 15, y + (-fontMedium->size + h) / 2, fontMedium,
                   LEFT);
    }

    drawRectangleChampfered(cWhite, relX + x + valueBaroffsetCenter, relY + y, (float)valueBarWidth / 2 * abs(amount),
                            valueBarHeigth, 1);

    drawRectangleFill(cWhite, relX + x + valueBarWidth / 2 - 1, relY + y + 2, 1, valueBarHeigth - 4);
}

void drawSettingElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                        uint8_t hugeFont) {
    const GUI_FONTINFO *selectedFont;
    if (hugeFont) {
        selectedFont = fontBig;
    }
    else {
        selectedFont = fontMedium;
    }

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
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-selectedFont->size + dataHeight) / 2 - fontShiftHeight,
                   selectedFont, CENTER);
    }
    else {
        drawRectangleChampfered(cWhiteLight, x + 2, y, nameWidth - 4, dataHeight / 2, 1);
        drawString(text, cFont_Deselect, x + nameWidth / 2,
                   y + (-selectedFont->size + dataHeight) / 2 - fontShiftHeight, selectedFont, CENTER);
    }

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + w / 2, y + (-selectedFont->size + dataHeight) / 2 + fontShiftHeight,
               selectedFont,
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
            drawString(*name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
        else {
            drawRectangleFill(cWhite, x + w - 1, y + 2, 1, h - 4);
            drawString(*name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
    }
}

void drawBasePatchElement(BasePatch *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                          uint8_t patched) {

    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
        drawString(element->name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        drawString(element->name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }

    if (patched == 1) {
        drawRectangleChampfered(cWhite, x - 12, y, 10, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cWhite, x + w + 2, y, 10, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cWhite, x - 12, y, 10, h, 1);
        drawRectangleChampfered(cWhite, x + w + 2, y, 10, h, 1);
    }
}

void drawBasePatchElement(BasePatch *element, PatchElement *patch, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint8_t select, uint8_t patched) {

    int valueBarHeigth = 6;

    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h - valueBarHeigth - 1, 1);
        drawString(element->name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2 - valueBarHeigth / 2,
                   fontMedium, CENTER);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        drawString(element->name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2 - valueBarHeigth / 2,
                   fontMedium, CENTER);
    }

    if (patched == 1) {
        drawRectangleChampfered(cWhite, x - 12, y, 10, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cWhite, x + w + 2, y, 10, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cWhite, x - 12, y, 10, h, 1);
        drawRectangleChampfered(cWhite, x + w + 2, y, 10, h, 1);
    }

    float amount;
    int valueBaroffsetCenter;

    amount = patch->getAmount();

    if (amount < 0) {
        valueBaroffsetCenter = (w + (float)w * amount) / 2;
    }
    else {
        valueBaroffsetCenter = w / 2;
    }

    drawRectangleFill(cWhite, x + valueBaroffsetCenter, y + h - valueBarHeigth, (float)w / 2 * abs(amount),
                      valueBarHeigth);

    drawRectangleFill(cWhite, x + w / 2 - 1, y + h - valueBarHeigth, 1, valueBarHeigth);
}

void drawModuleElement(BaseModule *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                       uint8_t patched) {

    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
        drawString(element->name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);
        drawString(element->name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }

    if (patched == 1) {
        drawRectangleChampfered(cWhite, x - 12, y, 10, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cWhite, x + w + 2, y, 10, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cWhite, x - 12, y, 10, h, 1);
        drawRectangleChampfered(cWhite, x + w + 2, y, 10, h, 1);
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
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawRectangleChampfered(cWhiteMedium, x, y, nameWidth, h, 1);
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }

    uint16_t valueBarWidth = w - nameWidth - 4;
    uint16_t valueBarHeigth = heightBar;

    uint16_t relX = nameWidth;
    uint16_t relY = h - heightBar;

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + relX + valueBarWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium,
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
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
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
        drawString(text, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawString(text, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
}

void drawPresetElement(presetStruct *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    std::string name;

    if (element->usageState != PRESETBLOCKUSED) {

        name = "FREE";
    }
    else {
        name = element->name;
    }

    if (select) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
        drawString(name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        drawString(name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
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
    if (entry != nullptr) {

        if (!active) {
            return;
        }
        if (patch == nullptr) {
            drawBasePatchElement(entry, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched);
        }
        else {

            if (select) {
                actionHandler.registerActionEncoder4(
                    {std::bind(&PatchElementInOut::changeAmount, patch, 0.05), "AMOUNT"},
                    {std::bind(&PatchElementInOut::changeAmount, patch, -0.05), "AMOUNT"},
                    {std::bind(&PatchElementInOut::setAmount, patch, 0), "RESET"});

                // TODO reset geht nicht?
            }
            drawBasePatchElement(entry, patch, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched);
        }
    }
    // reset Marker
    select = 0;
    active = 0;
    patched = 0;
    patch = nullptr;
    // entry = nullptr;
}

void Module_PanelElement::Draw() {
    if (entry != nullptr) {

        if (!active) {
            return;
        }
        if (select) {
            // actionHandler.registerActionEncoder4(entrys[2].functionCW, entrys[2].functionCCW,
            // entrys[2].functionPush);
        }

        drawModuleElement(entry, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched);

        // reset Marker
        select = 0;
        active = 0;
        patched = 0;
        //    entry = nullptr;
    }
}

void Preset_PanelElement::Draw() {
    if (entry != nullptr) {

        if (!active) {
            return;
        }
        if (select) {
            // actionHandler.registerActionEncoder4(entrys[2].functionCW, entrys[2].functionCCW,
            // entrys[2].functionPush);
        }

        drawPresetElement(entry, panelAbsX, panelAbsY, entryWidth, entryHeight, select);

        // reset Marker
        select = 0;
        active = 0;
    }
}

void Live_PanelElement::Draw() {
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
    entryWidth = width / (numberEntrys);

    for (int x = 0; x < numberEntrys; x++) {
        if (entrys[x].type != EMPTY) {
            drawSettingElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select, true);
        }

        relX += entryWidth;
        entrys[x].functionCW = {nullptr, ""};
        entrys[x].functionCCW = {nullptr, ""};
        entrys[x].functionPush = {nullptr, ""};
    }

    numberEntrys = 0;
    select = 0;
}

#endif