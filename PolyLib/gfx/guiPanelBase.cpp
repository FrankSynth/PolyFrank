#ifdef POLYCONTROL

#include "guiPanelBase.hpp"

const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift12_FontInfo;

const GUI_FONTINFO *fontMedium = &GUI_FontBahnschrift24_FontInfo;
const GUI_FONTINFO *fontBig = &GUI_FontBahnschrift32_FontInfo;

const GUI_FONTINFO *fontConsole = &GUI_FontBahnschrift16_FontInfo;

void drawConsole(CircularBuffer<char, 1024> consoleBuffer, uint16_t rows, uint16_t x, uint16_t y, uint16_t w,
                 uint16_t h) {

    uint8_t characterHeigth = fontConsole->size;

    uint8_t lines = h / characterHeigth;

    renderTask task;
    task.mode = M2MTRANSPARENT_A4;   // Set DMA2D To copy M2M with Blending
    task.height = fontConsole->size; // Set Font Height
    task.color = cFont_Deselect;     // Set Font Height

    uint16_t line = 0;
    uint16_t row = 0;
    uint16_t rowWidth = w / rows;

    // For each Char
    uint16_t relX = 0;

    char *pData = consoleBuffer.m_head;

    char c;

    for (uint16_t i = 0; i < consoleBuffer.size(); i++) {
        if (pData == consoleBuffer.m_end) { // check if we reached end-> start from the beginning
            pData = consoleBuffer.m_buffer;
        }
        c = *pData;

        if (c == '\n') { // New Line
            line += 1;
        }
        else if (c == '\r') { // Carriage return
            relX = 0;
        }
        else if (c == '\f') { // Form feed
            row += 1;
            line = 0;
            relX = 0;
        }
        else {

            task.width = fontConsole->font[(uint8_t)c - 32].BytesPerLine * 2;  // Character Width
            task.pSource = (uint32_t)fontConsole->font[(uint8_t)c - 32].pData; // Pointer to Character

            // check line and row
            if ((relX + task.width) >= rowWidth) {
                relX = 0;
                line += 1;
            }
            if (line >= lines) {
                row += 1;
                line = 0;

                if (row >= rows)
                    return;
            }

            // set character position
            task.x = x + row * rowWidth + relX;
            task.y = y + line * characterHeigth;

            relX += task.width;

            addToRenderQueue(task); // Add Task to RenderQueue
        }
        pData += 1;
    }
}

void drawDeviceManager(std::string *string, uint16_t rows, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

    uint8_t characterHeigth = fontConsole->size;

    uint8_t lines = h / characterHeigth;

    renderTask task;
    task.mode = M2MTRANSPARENT_A4;   // Set DMA2D To copy M2M with Blending
    task.height = fontConsole->size; // Set Font Height
    task.color = cFont_Deselect;     // Set Font Height

    uint16_t line = 0;
    uint16_t row = 0;
    uint16_t rowWidth = w / rows;

    // For each Char
    uint16_t relX = 0;

    for (char &c : *string) {

        if (c == '\n') { // New Line
            line += 1;
        }
        else if (c == '\r') { // Carriage return
            relX = 0;
        }
        else if (c == '\f') { // Form feed
            row += 1;
            line = 0;
            relX = 0;
        }
        else {

            task.width = fontConsole->font[(uint8_t)c - 32].BytesPerLine * 2;  // Character Width
            task.pSource = (uint32_t)fontConsole->font[(uint8_t)c - 32].pData; // Pointer to Character

            // check line and row
            if ((relX + task.width) >= rowWidth) {
                relX = 0;
                line += 1;
            }
            if (line >= lines) {
                row += 1;
                line = 0;

                if (row >= rows)
                    return;
            }

            // set character position
            task.x = x + row * rowWidth + relX;
            task.y = y + line * characterHeigth;

            relX += task.width;

            addToRenderQueue(task); // Add Task to RenderQueue
        }
    }
}

void drawPatchInOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {
    // PatchElementOutOut *dataOutOut = nullptr;
    PatchElement *data = nullptr;
    // uint8_t outOutFlag = 0;

    data = (PatchElement *)entry->patch;
    // }
    uint16_t nameWidth = 200;

    uint16_t spaceLeftRightBar = 50;
    uint16_t spaceTopBottomBar = 4;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);
    std::string text;
    // get text
    if (entry->type == PATCHOUTPUT) {
        text = allLayers[currentFocus.layer]->getModules()[data->sourceOut->moduleId]->getShortName();
    }
    else {
        text = allLayers[currentFocus.layer]->getModules()[data->targetIn->moduleId]->getShortName();
    }
    // Draw Name
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, nameWidth, h, 1);
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
    // if (entry->type == PATCHOUTOUT) {

    //     amount = dataOutOut->getAmount();
    // }
    // else {
    amount = data->getAmount();
    // }

    if (amount < 0) {
        valueBaroffsetCenter = (valueBarWidth + (float)valueBarWidth * amount) / 2;
    }
    else {
        valueBaroffsetCenter = valueBarWidth / 2;
    }

    if (entry->patch->amount < 0) {
        text = "-";
    }

    else {
        text = "+";
    }

    text.append(std::to_string(abs(entry->patch->amount)));

    text.resize(5);

    drawString(text, cFont_Deselect, x + relX + valueBarWidth + 15, y + (-fontBig->size + h) / 2, fontBig, LEFT);

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
        return;
    }
    Setting *data = entry->setting;
    uint16_t nameWidth = w;
    uint16_t valueBarHeigth;
    if (hugeFont) {
        valueBarHeigth = 12;
    }
    else {
        valueBarHeigth = 6;
    }
    uint16_t dataHeight = h - valueBarHeigth;

    uint16_t fontShiftHeight = h / 4;

    // clear

    drawRectangleChampfered(cGrey, x + 2, y, w - 4, h, 1);

    // get text
    std::string text = data->getName();

    if (data->disable) {
        drawRectangleChampfered(cWhiteLight, x + 2, y, w - 4, dataHeight / 2, 1);
        drawString(text, cBlack, x + nameWidth / 2, y + (-selectedFont->size + dataHeight) / 2 - fontShiftHeight,
                   selectedFont, CENTER);
        text = "Disable";
        drawString(text, cBlack, x + w / 2, y + (-selectedFont->size + dataHeight) / 2 + fontShiftHeight, selectedFont,
                   CENTER); // center Text
        return;
    }

    if (select) {
        drawRectangleChampfered(cHighlight, x + 2, y, w - 4, dataHeight / 2, 1);
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-selectedFont->size + dataHeight) / 2 - fontShiftHeight,
                   selectedFont, CENTER);
    }
    else {
        drawRectangleChampfered(cWhiteMedium, x + 2, y, w - 4, dataHeight / 2, 1);
        drawString(text, cFont_Deselect, x + nameWidth / 2,
                   y + (-selectedFont->size + dataHeight) / 2 - fontShiftHeight, selectedFont, CENTER);
    }

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + w / 2, y + (-selectedFont->size + dataHeight) / 2 + fontShiftHeight,
               selectedFont,
               CENTER); // center Text

    if (hugeFont) {
        drawRectangleFill(cWhiteMedium, x + 1, y + 3, 1, h - 3);
        drawRectangleFill(cWhiteMedium, x + w - 2, y + 3, 1, h - 3);
    }

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

        // Draw Name
        if (select) {
            drawRectangleChampfered(cHighlight, x, y, w, h, 1);
            drawString(*name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
        else {
            drawRectangleFill(cWhite, x + w - 1, y + 2, 1, h - 4);
            drawString(*name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
    }
}

void drawBasePatchElement(BasePatch *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                          uint8_t patched, uint8_t showModuleName) {

    uint32_t colorFont;
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, h, 1);
        colorFont = cFont_Select;
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        colorFont = cFont_Deselect;
    }

    if (showModuleName) {

        uint8_t layerID = element->layerId;
        uint8_t moduleID = element->moduleId;

        drawString(allLayers[layerID]->getModules()[moduleID]->shortName, colorFont, x + 8,
                   y + (-fontMedium->size + h) / 2, fontMedium, LEFT);
        drawString(element->name, colorFont, x + w - 8, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }
    else {
        drawString(element->name, colorFont, x + 8, y + (-fontMedium->size + h) / 2, fontMedium, LEFT);
    }

    if (patched == 1) {
        drawRectangleChampfered(cPatch, x - 12, y, 10, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cPatch, x + w + 2, y, 10, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cPatch, x - 12, y, 10, h, 1);
        drawRectangleChampfered(cPatch, x + w + 2, y, 10, h, 1);
    }
}

void drawBasePatchElement(BasePatch *element, PatchElement *patch, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint8_t select, uint8_t patched, uint8_t showModuleName) {

    int valueBarHeigth = 6;

    uint32_t colorFont;
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, h - valueBarHeigth - 2, 1);
        colorFont = cFont_Select;
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        colorFont = cFont_Deselect;
    }

    if (showModuleName) {

        uint8_t layerID = element->layerId;
        uint8_t moduleID = element->moduleId;

        drawString(allLayers[layerID]->getModules()[moduleID]->shortName, colorFont, x + 8,
                   y + (-fontMedium->size + h) / 2, fontMedium, LEFT);
        drawString(element->name, colorFont, x + w - 8, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }
    else {
        drawString(element->name, colorFont, x + 8, y + (-fontMedium->size + h) / 2, fontMedium, LEFT);
    }

    if (patched == 1) {
        drawRectangleChampfered(cPatch, x - 12, y, 10, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cPatch, x + w + 2, y, 10, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cPatch, x - 12, y, 10, h, 1);
        drawRectangleChampfered(cPatch, x + w + 2, y, 10, h, 1);
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
        drawRectangleChampfered(cHighlight, x, y, w, h, 1);
        drawString(element->shortName, cFont_Select, x + w - 8, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }
    else {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);
        drawString(element->shortName, cFont_Deselect, x + w - 8, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }

    if (patched == 1) {
        drawRectangleChampfered(cPatch, x - 12, y, 10, h, 1);
    }
    else if (patched == 2) {
        drawRectangleChampfered(cPatch, x + w + 2, y, 10, h, 1);
    }
    else if (patched == 3) {
        drawRectangleChampfered(cPatch, x - 12, y, 10, h, 1);
        drawRectangleChampfered(cPatch, x + w + 2, y, 10, h, 1);
    }
}

void drawDigitalElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    // x += 2;
    // w -= 4;

    Digital *data = entry->digital;
    uint16_t nameWidth = w / 2;

    uint16_t heightBar = 8;
    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, nameWidth, h, 1);
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
    uint16_t nameWidth = 200;

    uint16_t spaceLeftRightBar = 15;
    uint16_t spaceTopBottomBar = 4;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cHighlight, x + 15, y, nameWidth - 15, h, 1);
        drawString(text, cFont_Select, x + nameWidth - 12, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }
    else {
        drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth - 12, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }

    uint16_t valueBarWidth = w - nameWidth - 2 * spaceLeftRightBar;
    uint16_t valueBarHeigth = h - 2 * spaceTopBottomBar;

    uint16_t relX = nameWidth + spaceLeftRightBar;
    uint16_t relY = spaceTopBottomBar;

    // valueBar
    // drawRectangleChampfered(cGreyLight, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);
    if (data->min < 0) { // Centered ValueBar -> expect symmetric range

        uint16_t centerX = relX + x + valueBarWidth / 2;
        drawRectangleChampfered(cWhite, centerX, relY + y + 1, 1, valueBarHeigth - 2, 1); // center

        if (data->value >= ((int32_t)(data->inputRange / 2) + data->minInputValue)) { // positive value
            valueBarWidth = (float)valueBarWidth * (data->value - data->minInputValue - data->inputRange / 2) /
                            (float)(data->inputRange);

            drawRectangleChampfered(cWhite, centerX, relY + y, valueBarWidth, valueBarHeigth, 1);
        }
        else { // negative value

            valueBarWidth = (float)valueBarWidth * ((data->inputRange / 2) - (data->value - data->minInputValue)) /
                            (float)(data->inputRange);

            drawRectangleChampfered(cWhite, centerX - valueBarWidth, relY + y, valueBarWidth, valueBarHeigth, 1);
        }
    }
    else {
        valueBarWidth = (float)valueBarWidth * (data->value - data->minInputValue) /
                        (float)(data->maxInputValue - data->minInputValue);

        drawRectangleChampfered(cWhite, relX + x, relY + y, valueBarWidth, valueBarHeigth, 1);
    }

    if (data->input != nullptr) {               // patchable?
        if (data->input->patchesInOut.size()) { // patched?
            drawRectangleChampfered(cPatch, x, y, 13, h, 1);
        }
        else {
            drawRectangleChampfered(cWhiteMedium, x, y, 13, h, 1);
        }
    }
}

void drawModuleElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    BaseModule *data = entry->modules;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, h, 1);
        drawString(text, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
    else {
        drawString(text, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
    }
}

void drawPresetElement(presetStruct *element, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select) {

    std::string name;

    if (element->usageState == PRESET_FREE) {
        return;
    }
    else {
        name = element->name;
    }

    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, h, 1);
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

    entryHeight = height;

    if (!visible) {
        drawRectangleFill(cClear, panelAbsX, panelAbsY, width, height);
        return;
    }

    if (select) {
        actionHandler.registerActionEncoder(0, entrys[0].functionCW, entrys[0].functionCCW, entrys[0].functionPush);
        actionHandler.registerActionEncoder(1, entrys[1].functionCW, entrys[1].functionCCW, entrys[1].functionPush);
        actionHandler.registerActionEncoder(2, entrys[2].functionCW, entrys[2].functionCCW, entrys[2].functionPush);
        actionHandler.registerActionEncoder(3, entrys[3].functionCW, entrys[3].functionCCW, entrys[3].functionPush);
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

            else if (entrys[x].type == PATCHOUTPUT || entrys[x].type == PATCHINPUT) {
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

        if (active) {

            if (patch == nullptr) {
                drawBasePatchElement(entry, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched,
                                     showModuleName);
            }
            else {

                if (select) {
                    actionHandler.registerActionEncoder(
                        3, {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 1), "AMOUNT"},
                        {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 0), "AMOUNT"},
                        {std::bind(&PatchElement::setAmount, patch, 0), "RESET"});
                }
                drawBasePatchElement(entry, patch, panelAbsX, panelAbsY, entryWidth, entryHeight, select, patched,
                                     showModuleName);
            }
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

    entryHeight = height;

    if (!visible) {
        drawRectangleFill(cClear, panelAbsX, panelAbsY, width, height);
        return;
    }

    if (select) {
        actionHandler.registerActionEncoder(0, entrys[0].functionCW, entrys[0].functionCCW, entrys[0].functionPush);
        actionHandler.registerActionEncoder(1, entrys[1].functionCW, entrys[1].functionCCW, entrys[1].functionPush);
        actionHandler.registerActionEncoder(2, entrys[2].functionCW, entrys[2].functionCCW, entrys[2].functionPush);
        actionHandler.registerActionEncoder(3, entrys[3].functionCW, entrys[3].functionCCW, entrys[3].functionPush);
    }
    entryWidth = width / (numberEntrys);

    for (int x = 0; x < numberEntrys; x++) {
        if (entrys[x].type != EMPTY) {
            drawSettingElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select, false);
        }

        relX += entryWidth;
        entrys[x].functionCW = {nullptr, ""};
        entrys[x].functionCCW = {nullptr, ""};
        entrys[x].functionPush = {nullptr, ""};
    }

    numberEntrys = 0;
    select = 0;
}

const char *valueToNote(const byte &noteIn) {

    byte note;
    note = noteIn % 12;
    switch (note) {
        case 0:
        case 1: return "C";
        case 2:
        case 3: return "D";
        case 4: return "E";
        case 5:
        case 6: return "F";
        case 7:
        case 8: return "G";
        case 9:
        case 10: return "A";
        case 11: return "B";
    }
    return "";
}

const char *valueToOctave(const byte &noteIn) {

    byte octave;
    octave = noteIn / 12;

    switch (octave) {
        case 0: return "-2";
        case 1: return "-1";
        case 2: return "0";
        case 3: return "1";
        case 4: return "2";
        case 5: return "3";
        case 6: return "4";
        case 7: return "5";
        case 8: return "6";
        case 9: return "7";
        case 10: return "8";
        case 11: return "9";
        case 12: return "10";
    }
    return "";
}

const char *valueToSharp(const byte &noteIn) {
    byte note;
    note = noteIn % 12;

    if (note == 1 || note == 3 || note == 6 || note == 8 || note == 10) {
        return "#";
    }

    return "";
}

const char *tuningToChar(const byte &tuning) {

    switch (tuning) {
        case 1: return "C";
        case 2: return "C#";
        case 3: return "D";
        case 4: return "D#";
        case 5: return "E";
        case 6: return "F";
        case 7: return "F#";
        case 8: return "G";
        case 9: return "G#";
        case 10: return "A";
        case 11: return "A#";
        case 12: return "B";
        case 13: return "F";
        default: return "-";
    }
}

// Patchmatrix

void MatrixPatch_PanelElement::Draw() {

    if (!visible) {
        return;
    }

    // create empty patch
    // uint32_t color = 0;

    if (entry != nullptr) {

        uint8_t barWidth = 3;
        uint8_t barHeight = 4;

        if (select) {
            drawRectangleFill(cHighlight, panelAbsX, panelAbsY, width, height);
        }
        else {
            drawRectangleFill(cGreyLight, panelAbsX, panelAbsY, width, height);
        }

        uint8_t color[4];
        float patchAmount = entry->amount;

        *(uint32_t *)color = cWhite;

        color[3] *= abs(patchAmount);

        drawRectangleFill(*(uint32_t *)color, panelAbsX + 2, panelAbsY + 2, width - 4 - barWidth * VOICESPERCHIP - 2,
                          height - 4);

        for (uint8_t i = 0; i < VOICESPERCHIP; i++) {
            float amount = patchAmount * entry->sourceOut->currentSample[i];
            amount = testFloat(amount, -1, 1);
            int16_t offsetY = ((int16_t)(height - barHeight) / 2) * (-amount) + (height - barHeight) / 2;
            drawRectangleFill(cWhite, panelAbsX + width - barWidth * (VOICESPERCHIP - i) - 2, panelAbsY + offsetY,
                              barWidth, barHeight);
        }
    }
    else {

        if (select) {
            drawRectangleFill(cHighlight, panelAbsX, panelAbsY, width, height);
            drawRectangleFill(cGreyLight, panelAbsX + 1, panelAbsY + 1, width - 2, height - 2);
        }
        else {
            drawRectangleFill(cWhiteLight, panelAbsX, panelAbsY, width, height);
        }
    }

    select = 0;
    visible = 0;
    entry = nullptr;
}

void MatrixPatch_PanelElement::addEntry(PatchElement *entry) {

    this->entry = entry;
    visible = 1;
}

void MatrixIn_PanelElement::Draw() {

    uint8_t barWidth = 3;
    uint8_t barHeight = 4;

    if (!visible) {
        return;
    }
    if (entry != nullptr) {

        if (select) {
            drawRectangleFill(cHighlight, panelAbsX, panelAbsY, width, height);

            drawString(entry->input->shortName, cFont_Select, panelAbsX + 2,
                       panelAbsY + (height - fontMedium->size) / 2, fontMedium, LEFT);
        }
        else {
            drawRectangleFill(cGreyLight, panelAbsX, panelAbsY, width, height);
            drawString(entry->input->shortName, cFont_Deselect, panelAbsX + 2,
                       panelAbsY + (height - fontMedium->size) / 2, fontMedium, LEFT);
        }
        if (entry->input->renderBuffer != nullptr) {
            // uint8_t color[4];

            // entry->

            for (uint8_t i = 0; i < VOICESPERCHIP; i++) {
                float amount = entry->input->renderBuffer->currentSample[i];

                amount -= entry->min;
                amount /= entry->max;

                amount = testFloat(amount, 0, 1);

                int16_t offsetY = (int16_t)(height - barHeight) * (1 - amount);
                drawRectangleFill(cWhite, panelAbsX + width - barWidth * (VOICESPERCHIP - i) - 2, panelAbsY + offsetY,
                                  barWidth, barHeight);
            }
        }
    }
    select = 0;
    visible = 0;
    entry = nullptr;
}

void MatrixIn_PanelElement::addEntry(Analog *entry) {

    this->entry = entry;
    visible = 1;
}

void MatrixOut_PanelElement::Draw() {
    uint8_t barWidth = 3; // oder so
    uint8_t barHeight = 4;

    // uint8_t color[4];

    if (!visible) {
        return;
    }
    if (entry != nullptr) {

        const char *name;

        if (entry->moduleId == allLayers[entry->layerId]->midi.id) {
            name = entry->shortName;
        }
        else {
            name = allLayers[entry->layerId]->getModules()[entry->moduleId]->shortName;
        }

        if (select) {
            drawRectangleFill(cHighlight, panelAbsX, panelAbsY, width, height);
            drawString(name, cFont_Select, panelAbsX + 5, panelAbsY + (height - fontMedium->size) / 2, fontMedium,
                       LEFT);
        }
        else {
            drawRectangleFill(cGreyLight, panelAbsX, panelAbsY, width, height);
            drawString(name, cFont_Deselect, panelAbsX + 5, panelAbsY + (height - fontMedium->size) / 2, fontMedium,
                       LEFT);
        }

        float amount;

        for (uint8_t i = 0; i < VOICESPERCHIP; i++) {

            amount = testFloat(entry->currentSample[i], -1, 1);
            int16_t offsetY = ((int16_t)(height - barHeight) / 2) * (-amount) + (height - barHeight) / 2;
            drawRectangleFill(cWhite, panelAbsX + width - barWidth * (VOICESPERCHIP - i) - 2, panelAbsY + offsetY,
                              barWidth, barHeight);
        }
    }
    select = 0;
    visible = 0;
    entry = nullptr;
}

void MatrixOut_PanelElement::addEntry(Output *entry) {

    this->entry = entry;
    visible = 1;
}

void MatrixModule_PanelElement::Draw() {
    if (!visible) {
        return;
    }
    if (entry != nullptr) {
        const char *name = entry->shortName;

        if (select) {
            drawRectangleFill(cHighlight, panelAbsX, panelAbsY, width, height);
            drawString(name, cFont_Select, panelAbsX + width / 2, panelAbsY + (height - fontMedium->size) / 2,
                       fontMedium, CENTER);
        }
        else {
            drawRectangleFill(cGreyLight, panelAbsX, panelAbsY, width, height);
            drawString(name, cFont_Deselect, panelAbsX + width / 2, panelAbsY + (height - fontMedium->size) / 2,
                       fontMedium, CENTER);
        }
    }
    select = 0;
    visible = 0;
    entry = nullptr;
}

void MatrixModule_PanelElement::addEntry(BaseModule *entry) {

    this->entry = entry;
    visible = 1;
}

void drawWaveFromModule(BaseModule *module, uint16_t x, uint16_t y) {

    // empty waveBuffer
    uint32_t data = 0x00000000;

    fastMemset(&data, (uint32_t *)*(waveBuffer.buffer), (waveBuffer.width * waveBuffer.height) / 2);

    // drwaWavebuffer

    if (module->id == allLayers[currentFocus.layer]->oscA.id) {
        drawWave(allLayers[currentFocus.layer]->renderedAudioWavesOscA, 100, 2);
    }
    else if (module->id == allLayers[currentFocus.layer]->oscB.id) {
        drawWave(allLayers[currentFocus.layer]->renderedAudioWavesOscB, 100, 2);
    }
    else if (module->id == allLayers[currentFocus.layer]->sub.id) {
        drawWave(allLayers[currentFocus.layer]->renderedAudioWavesSub, 100, 2);
    }
    else if (module->moduleType == MODULE_LFO) {
        int8_t wave[100];

        calculateLFOWave((LFO *)module, wave, 100);
        drawWave(wave, 100, 2);
    }
    else if (module->moduleType == MODULE_ADSR) {
        vec<2> wave[100];

        calculateADSRWave((ADSR *)module, wave, 100);
        drawVecWave(wave, 100);
    }

    SCB_CleanDCache_by_Addr((uint32_t *)waveBuffer.buffer, waveBuffer.height * waveBuffer.width * 2);

    copyWaveBuffer(waveBuffer, x, y);
}

void drawWave(int8_t *renderedWave, uint16_t samples, uint32_t repeats) {

    uint16_t wavecolor = 0xFFFF;
    uint16_t gridcolor = 0x3FFF;
    uint16_t framecolor = 0xAFFF;

    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;

    // Grid

    uint8_t grid = 12;

    for (uint16_t i = 1; i < grid; i++) {
        drawLine(waveBuffer, i * ((float)waveBuffer.width / grid), 0, i * ((float)waveBuffer.width / grid),
                 waveBuffer.height - 1, gridcolor);
    }
    drawLine(waveBuffer, 0, waveBuffer.height / 2, waveBuffer.width - 1, waveBuffer.height / 2, framecolor);

    // Frame
    drawLine(waveBuffer, 0, 0, waveBuffer.width - 1, 0, framecolor);
    drawLine(waveBuffer, 0, waveBuffer.height - 1, waveBuffer.width - 1, waveBuffer.height - 1, framecolor);
    drawLine(waveBuffer, 0, 0, 0, waveBuffer.height - 1, framecolor);
    drawLine(waveBuffer, waveBuffer.width - 1, 0, waveBuffer.width - 1, waveBuffer.height - 1, framecolor);

    for (uint16_t i = 0; i < 100 * repeats - 1; i++) {
        x1 = ((float)waveBuffer.width / (100.f * repeats - 1)) * i;
        x2 = ((float)waveBuffer.width / (100.f * repeats - 1)) * (i + 1);
        y1 = -renderedWave[i % 100];
        y2 = -renderedWave[(i + 1) % 100];

        y1 = (float)(y1 + 128) * (waveBuffer.height / 255.f);
        y2 = (float)(y2 + 128) * (waveBuffer.height / 255.f);

        y1 = testInt(y1, 1, waveBuffer.height - 2);
        y2 = testInt(y2, 1, waveBuffer.height - 2);

        x1 = testInt(x1, 1, waveBuffer.width - 1);
        x2 = testInt(x2, 1, waveBuffer.width - 1);
        draw3Line(waveBuffer, x1, y1, x2, y2, wavecolor);
    }
}

void drawVecWave(vec<2> *renderedWave, uint16_t samples) {

    uint16_t wavecolor = 0xFFFF;
    uint16_t gridcolor = 0x3FFF;
    uint16_t framecolor = 0xAFFF;

    vec<2> p1;
    vec<2> p2;

    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
    // Grid

    uint8_t grid = 12;

    for (uint16_t i = 1; i < grid; i++) {
        drawLine(waveBuffer, i * ((float)waveBuffer.width / grid), 0, i * ((float)waveBuffer.width / grid),
                 waveBuffer.height - 1, gridcolor);
    }
    drawLine(waveBuffer, 0, waveBuffer.height / 2, waveBuffer.width - 1, waveBuffer.height / 2, framecolor);

    // Frame
    drawLine(waveBuffer, 0, 0, waveBuffer.width - 1, 0, framecolor);
    drawLine(waveBuffer, 0, waveBuffer.height - 1, waveBuffer.width - 1, waveBuffer.height - 1, framecolor);
    drawLine(waveBuffer, 0, 0, 0, waveBuffer.height - 1, framecolor);
    drawLine(waveBuffer, waveBuffer.width - 1, 0, waveBuffer.width - 1, waveBuffer.height - 1, framecolor);

    for (uint16_t i = 0; i < (samples - 1); i++) {
        // load Wave
        p1 = renderedWave[i];
        p2 = renderedWave[i + 1];

        // Flip Wave
        p1[1] = 1 - p1[1];
        p2[1] = 1 - p2[1];

        // Scale Wave
        p1[0] *= (waveBuffer.width - 2);
        p2[0] *= (waveBuffer.width - 2);

        p1[0] += 1;
        p2[0] += 1;

        p1[1] *= waveBuffer.height - 2;
        p2[1] *= waveBuffer.height - 2;

        // Check range
        x1 = testInt((int32_t)p1[0], 1, waveBuffer.width - 1);
        y1 = testInt((int32_t)p1[1], 1, waveBuffer.height - 2);
        x2 = testInt((int32_t)p2[0], 1, waveBuffer.width - 1);
        y2 = testInt((int32_t)p2[1], 1, waveBuffer.height - 2);

        draw3Line(waveBuffer, x1, y1, x2, y2, wavecolor);
    }
}

#include "render/renderLFO.cpp"

void calculateLFOWave(LFO *module, int8_t *waveBuffer, uint16_t samples) {

    const float random[10] = {-.8, 0.6, 0, 1, -0.3, 0.2, 0, -0.9, 0.5};

    float phase = 0;

    float shape = testFloat(module->iShape.renderBuffer->currentSample[0] + module->aShape.valueMapped,
                            module->aShape.min, module->aShape.max - 0.001);

    float index;

    for (uint16_t i = 0; i < samples; i++) {
        phase = (1.f / samples) * i;

        phase = phase - floor(phase);

        float fract = shape - std::floor(shape);

        if (shape < 1) {
            index = fast_lerp_f32(calcSin(phase), calcRamp(phase), fract);
        }
        else if (shape < 2) {
            index = fast_lerp_f32(calcRamp(phase), calcTriangle(phase), fract);
        }
        else if (shape < 3) {
            index = fast_lerp_f32(calcTriangle(phase), calcInvRamp(phase), fract);
        }
        else if (shape < 4) {
            index = fast_lerp_f32(calcInvRamp(phase), calcSquare(phase, shape), fract);
        }
        else if (shape < 5)
            index = calcSquare(phase, shape);

        else {
            index = fast_lerp_f32(-1.0f, random[(uint16_t)((4.f / samples) * i)], fract);
        }

        waveBuffer[i] = index * 127;
    }
}

void calculateADSRWave(ADSR *module, vec<2> *waveBuffer, uint32_t samples) {
    vec<2> L0;
    vec<2> L2;

    vec<2> A0;
    vec<2> A1;
    vec<2> A1B;
    vec<2> A2;

    vec<2> D0;
    vec<2> D1;
    vec<2> D1B;
    vec<2> D2;

    vec<2> S0;
    vec<2> S1;
    vec<2> S2;

    vec<2> R0;
    vec<2> R1;
    vec<2> R1B;
    vec<2> R2;

    float range = 4.2;

    float delay = fastMap(module->aDelay, module->aDelay.min, module->aDelay.max, 0.02, 1);
    float attack = fastMap(module->aAttack, module->aAttack.min, module->aAttack.max, 0.02, 1);
    float decay = fastMap(module->aDecay, module->aDecay.min, module->aDecay.max, 0.02, 1);
    float release = fastMap(module->aRelease, module->aRelease.min, module->aRelease.max, 0.02, 1);
    float sustain = range - attack - decay - release - delay;
    float sustainLevel = fastMap(module->aSustain, module->aSustain.min, module->aSustain.max, 0.00, 1);

    float shape = module->aShape;

    uint32_t delaySamples = (delay / range) * samples;
    uint32_t attackSamples = (attack / range) * samples;
    uint32_t decaySamples = (decay / range) * samples;
    uint32_t releaseSamples = (release / range) * samples;

    delaySamples = testInt(delaySamples, 2, samples);
    attackSamples = testInt(attackSamples, 2, samples);
    decaySamples = testInt(decaySamples, 2, samples);
    releaseSamples = testInt(releaseSamples, 2, samples);

    uint32_t sustainSamples = samples - attackSamples - decaySamples - releaseSamples - delaySamples;

    // println("samples : ", samples);
    // println("attackSamples : ", attackSamples);
    // println("decaySamples : ", decaySamples);
    // println("sustainSamples : ", sustainSamples);
    // println("releaseSamples : ", releaseSamples);
    // Delay
    L0 = 0;
    L2[0] = delay / range;
    L2[1] = 0;

    // Attack
    A0 = L2;
    A2[0] = A0[0] + (attack / range);
    A2[1] = 1;

    // shape Attack
    A1 = (A0 + A2) / 2.f;
    A1B[0] = A0[0];
    A1B[1] = A2[1];

    A1 = fast_lerp_f32(A1, A1B, shape);
    // Decay
    D0 = A2;

    D2[0] = D0[0] + decay / range;
    D2[1] = sustainLevel;
    D1 = (D0 + D2) / 2.f;
    D1B[0] = D0[0];
    D1B[1] = D2[1];

    D1 = fast_lerp_f32(D1, D1B, shape);

    // Sustain
    S0 = D2;

    S2 = S0;
    S2[0] += sustain / range;
    // S1 = (S0 + S2) / 2;

    // Release
    R0 = S2;

    R2[0] = R0[0] + release / range;
    R2[1] = 0;
    R1 = (R0 + R2) / 2.f;
    R1B[0] = R0[0];
    R1B[1] = R2[1];
    R1 = fast_lerp_f32(R1, R1B, shape);

    // set fix points
    waveBuffer[0] = L0;
    waveBuffer[delaySamples] = A0;
    waveBuffer[delaySamples + attackSamples] = D0;
    waveBuffer[delaySamples + attackSamples + decaySamples] = S0;
    waveBuffer[delaySamples + attackSamples + decaySamples + sustainSamples] = R0;
    waveBuffer[samples - 1] = R2;

    uint32_t index = 0;
    for (uint32_t i = 1; i < delaySamples; i++) {
        waveBuffer[i + index] = fast_lerp_f32(L0, L2, testFloat((float)i / (delaySamples), 0, 0.999));
    }
    index += delaySamples;

    for (uint32_t i = 1; i < attackSamples; i++) {
        waveBuffer[i + index] = bezier2D(A0, A1, A2, testFloat((float)i / (attackSamples), 0, 0.999));
    }
    index += attackSamples;
    for (uint32_t i = 1; i < decaySamples; i++) {
        waveBuffer[i + index] = bezier2D(D0, D1, D2, testFloat((float)i / (decaySamples), 0, 0.999));
    }
    index += decaySamples;

    for (uint32_t i = 1; i < sustainSamples; i++) {
        waveBuffer[i + index] = fast_lerp_f32(S0, S2, testFloat((float)i / (sustainSamples), 0, 0.999));
    }
    index += sustainSamples;

    for (uint32_t i = 1; i < (releaseSamples - 1); i++) {
        waveBuffer[i + index] = bezier2D(R0, R1, R2, testFloat((float)i / releaseSamples, 0, 0.999));
    }

    // for (uint32_t i = 0; i < samples; i++) {
    //     println(waveBuffer[i][0], "  ", waveBuffer[i][1]);
    // }
}

#endif