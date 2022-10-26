#ifdef POLYCONTROL

#include "guiPanelBase.hpp"
#include "datacore/dataHelperFunctions.hpp"

const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift12_FontInfo;

const GUI_FONTINFO *fontMedium = &GUI_FontBahnschrift24_FontInfo;
const GUI_FONTINFO *fontBig = &GUI_FontBahnschrift32_FontInfo;

const GUI_FONTINFO *fontConsole = &GUI_FontBahnschrift16_FontInfo;

void drawConsole(const CircularBuffer<char, 1024> &consoleBuffer, uint16_t rows, uint32_t x, uint32_t y, uint16_t w,
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

    const char *pData = consoleBuffer.m_head;

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

void drawDeviceManager(std::string *string, uint16_t rows, uint32_t x, uint32_t y, uint16_t w, uint16_t h) {

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

void drawPatchInOutElement(entryStruct *entry, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select) {
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

        // Draw Name
        if (select) {
            drawRectangleChampfered(cHighlight, x, y, nameWidth, h, 1);
            drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
        else {
            drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
            drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
    }
    else {
        text = allLayers[currentFocus.layer]->getModules()[data->targetIn->moduleId]->getShortName();
        text.append(" ");
        std::string text2 = "| ";
        text2.append(data->targetIn->getShortName());

        // Draw Name
        if (select) {
            drawRectangleChampfered(cHighlight, x, y, nameWidth, h, 1);
            drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
            drawString(text2, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, LEFT);
        }
        else {
            drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
            drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
            drawString(text2, cFont_Deselect, x + nameWidth / 2, y + (-fontMedium->size + h) / 2, fontMedium, LEFT);
        }
    }

    uint16_t valueBarWidth = w - nameWidth - 2 * spaceLeftRightBar;

    uint16_t valueBarHeigth = h - 2 * spaceTopBottomBar;

    uint16_t relX = nameWidth + 15;
    uint16_t relY = spaceTopBottomBar;

    int16_t valueBaroffsetCenter = 0;
    float amount;

    amount = data->getAmount();

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

void drawSettingElement(entryStruct *entry, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select,
                        uint32_t keyColor) {
    const GUI_FONTINFO *font = fontMedium;

    if (entry->setting == nullptr) {
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

    if (data->disable) {
        drawRectangleChampfered(cGreyLight, x, y, w, dataHeight / 2, 1);
        drawString(text, cBlack, x + nameWidth / 2, y + (-font->size + dataHeight) / 2 - fontShiftHeight, font, CENTER);
        text = "Disable";
        drawString(text, cBlack, x + w / 2, y + (-font->size + dataHeight) / 2 + fontShiftHeight, font,
                   CENTER); // center Text
        return;
    }

    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, dataHeight / 2, 1);
        drawString(text, cFont_Select, x + nameWidth / 2, y + (-font->size + dataHeight) / 2 - fontShiftHeight, font,
                   CENTER);
    }
    else {
        drawRectangleChampfered(cWhiteDark, x, y, w, dataHeight / 2, 1);
        drawString(text, cFont_Deselect, x + nameWidth / 2, y + (-font->size + dataHeight) / 2 - fontShiftHeight, font,
                   CENTER);
    }

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + w / 2, y + (-font->size + dataHeight) / 2 + fontShiftHeight, font,
               CENTER); // center Text

    uint16_t relY = h - valueBarHeigth;
    uint16_t valueBarWidth = w - 4;

    valueBarWidth = (float)valueBarWidth * ((float)data->value - data->min) / (float)(data->max - data->min);
    drawRectangleChampfered(keyColor, x + 2, relY + y, valueBarWidth, valueBarHeigth, 1);
}

void drawCustomDigitalElement(Digital *data, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    uint32_t nameWidth = w;
    uint32_t valueBarHeigth = 6;
    uint32_t dataHeight = h - valueBarHeigth;
    uint32_t fontShiftHeight = h / 4;

    // get text
    std::string text = data->getName();

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    drawRectangleChampfered(cHighlight, x, y, w, dataHeight / 2, 1);
    drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + dataHeight) / 2 - fontShiftHeight,
               fontMedium, CENTER);

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + w / 2, y + (-fontMedium->size + dataHeight) / 2 + fontShiftHeight, fontMedium,
               CENTER); // center Text

    uint32_t relY = h - valueBarHeigth;
    uint32_t valueBarWidth = 0;

    valueBarWidth = (float)w * ((float)data->valueMapped - data->min) / (float)(data->max - data->min);
    drawRectangleChampfered(cLayer, x, relY + y, valueBarWidth, valueBarHeigth, 1);
}

void drawNameElement(std::string *name, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t select) {

    // clear
    if (name != nullptr) {
        drawRectangleChampfered(cGrey, x, y, w, h, 1);

        // Draw Name
        if (select) {
            drawRectangleChampfered(cHighlight, x, y, w, h, 1);
            drawString(*name, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
        else {
            // drawRectangleFill(cWhite, x + w - 1, y + 2, 1, h - 4);
            drawString(*name, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2, fontMedium, CENTER);
        }
    }
}

void drawBasePatchElement(BasePatch *element, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select,
                          uint8_t patched, uint8_t showModuleName) {

    uint32_t colorFont;
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, h, 1);
        colorFont = cFont_Select;
    }
    else {
        drawRectangleChampfered(cGreyDark, x, y, w, h, 1);

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

void drawBasePatchElement(BasePatch *element, PatchElement *patch, uint32_t x, uint32_t y, uint16_t w, uint16_t h,
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

void drawModuleElement(BaseModule *element, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select,
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

void drawDigitalElement(entryStruct *entry, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select) {

    // x += 2;
    // w -= 4;

    Digital *data = entry->digital;
    uint16_t nameWidth = w / 2;

    uint16_t spaceLeftRightBar = 15;
    uint16_t spaceTopBottomBar = 6;

    // clear

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    // get text
    std::string text = data->getName();

    // Draw Name
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, nameWidth, h, 1);
        drawString(text, cFont_Select, x + nameWidth - 12, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }
    else {
        drawString(text, cFont_Deselect, x + nameWidth - 12, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }

    uint16_t valueBarWidth = w / 2 - 2 * spaceLeftRightBar;
    uint16_t valueBarHeigth = h - 2 * spaceTopBottomBar;

    uint16_t relY = (h - valueBarHeigth) / 2;

    text = data->getValueAsString();

    valueBarWidth = (float)valueBarWidth * ((float)data->valueMapped - data->min) / (float)(data->max - data->min);

    drawRectangleChampfered(cLayer, x + nameWidth + spaceLeftRightBar, relY + y, valueBarWidth, valueBarHeigth, 1);

    drawString(text, cFont_Deselect, x + nameWidth + w / 4, y + (-fontMedium->size + h) / 2, fontMedium,
               CENTER); // center Text
}

void drawAnalogElement(entryStruct *entry, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select) {

    Analog *data = entry->analog;
    uint16_t nameWidth = w / 2;

    uint16_t spaceLeftRightBar = 15;
    uint16_t spaceTopBottomBar = 6;

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
        // drawRectangleFill(cWhite, x + nameWidth - 1, y + 2, 1, h - 4);
        drawString(text, cFont_Deselect, x + nameWidth - 12, y + (-fontMedium->size + h) / 2, fontMedium, RIGHT);
    }

    uint16_t valueBarWidth = w - nameWidth - 2 * spaceLeftRightBar;
    uint16_t valueBarHeigth = h - 2 * spaceTopBottomBar;

    uint16_t relX = nameWidth + spaceLeftRightBar;
    uint16_t relY = spaceTopBottomBar;

    // valueBar
    if (data->min < 0) { // Centered ValueBar -> expect symmetric range

        uint16_t centerX = relX + x + valueBarWidth / 2;
        drawRectangleFill(cWhite, centerX, relY + y, 1, valueBarHeigth); // center

        if (data->value >= ((int32_t)(data->inputRange / 2) + data->minInputValue)) { // positive value
            valueBarWidth = (float)valueBarWidth * (data->value - data->minInputValue - data->inputRange / 2) /
                            (float)(data->inputRange);

            drawRectangleFill(cLayer, centerX, relY + y, valueBarWidth, valueBarHeigth);
        }
        else { // negative value

            valueBarWidth = (float)valueBarWidth * ((data->inputRange / 2) - (data->value - data->minInputValue)) /
                            (float)(data->inputRange);

            drawRectangleFill(cLayer, centerX - valueBarWidth, relY + y, valueBarWidth, valueBarHeigth);
        }
    }
    else {
        valueBarWidth = (float)valueBarWidth * (data->value - data->minInputValue) /
                        (float)(data->maxInputValue - data->minInputValue);

        drawRectangleFill(cLayer, relX + x, relY + y, valueBarWidth, valueBarHeigth);
    }

    if (data->input != nullptr) {               // patchable?
        if (data->input->patchesInOut.size()) { // patched?
            drawRectangleFill(cWhite, x, y, 13, h);
        }
        else {
            drawRectangleFill(cWhiteDark, x, y, 13, h);
        }
    }

    if (data->input != nullptr && data->input->renderBuffer != nullptr) { // draw real value

        uint32_t marker = 2;

        valueBarWidth = w - nameWidth - 2 * spaceLeftRightBar;
        uint32_t valuePos = 0;

        for (uint32_t i = 0; i < NUMBERVOICES; i++) {
            float value = data->input->renderBuffer->currentSample[i];
            value = testFloat(value, data->min, data->max);

            if (data->min < 0) { // Centered ValueBar -> expect symmetric range
                uint16_t centerX = relX + x + valueBarWidth / 2;

                if (value >= ((int32_t)((data->max - data->min) / 2) + data->min)) { // positive value

                    valuePos = centerX + ((float)valueBarWidth * (value - data->min - (data->max - data->min) / 2) /
                                          (float)((data->max - data->min)));
                }
                else { // negative value

                    valuePos = centerX - ((float)valueBarWidth * (((data->max - data->min) / 2) - (value - data->min)) /
                                          (float)((data->max - data->min)));
                }
            }
            else {
                valuePos = (float)valueBarWidth * (value - data->min) / (float)(data->max - data->min) + relX + x;
            }

            drawRectangleFill(cHighlight, valuePos, relY + y, marker, valueBarHeigth);
        }
    }
}

void drawSmallAnalogElement(Analog *data, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select,
                            uint8_t modulename) {

    if (data == nullptr)
        return;
    uint16_t heightBar = 10;

    // clear
    drawRectangleChampfered(cGrey, x, y, w, h, 1);
    std::string text;

    // get text
    if (modulename == true) {
        text = allLayers[data->layerId]->modules[data->moduleId]->getShortName();
    }
    else {
        text = data->getName();
    }

    // Draw Name
    if (select) {
        drawRectangleChampfered(cHighlight, x, y, w, h - heightBar - 2, 1);
        drawString(text, cFont_Select, x + w / 2, y + (-fontMedium->size + h) / 2 - 6, fontMedium, CENTER);
    }
    else {
        drawRectangleChampfered(cWhiteDark, x, y, w, h - heightBar - 2, 1);
        drawString(text, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h) / 2 - 6, fontMedium, CENTER);
    }

    uint16_t valueBarWidth = w;
    uint16_t valueBarHeigth = heightBar;
    uint16_t relY = h - heightBar;
    valueBarWidth = (float)valueBarWidth * ((float)data->valueMapped - data->min) / (float)(data->max - data->min);

    drawRectangleChampfered(cLayer, x, relY + y, valueBarWidth, valueBarHeigth, 1);
}

void drawModuleElement(entryStruct *entry, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select) {

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

void drawPresetElement(presetStruct *element, uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t select) {

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
        // drawRectangleFill(cClear, panelAbsX, panelAbsY, width, height);
        return;
    }

    // wir nehmen an das wenn das erste element ein Setting ist, alle elemente Settings sind
    if (entrys[0].type == SETTING) {

        if (select) {
            actionHandler.registerActionEncoder(0, entrys[0].functionCW, entrys[0].functionCCW, entrys[0].functionPush);
            actionHandler.registerActionEncoder(1, entrys[1].functionCW, entrys[1].functionCCW, entrys[1].functionPush);
            actionHandler.registerActionEncoder(2, entrys[2].functionCW, entrys[2].functionCCW, entrys[2].functionPush);
            actionHandler.registerActionEncoder(3, entrys[3].functionCW, entrys[3].functionCCW, entrys[3].functionPush);
            actionHandler.registerActionEncoder(5);
        }

        uint32_t nameFieldWidth = 148;

        entryWidth = (width - nameFieldWidth + 2) / numberEntrys;

        // Feld mit GruppenNamen
        drawNameElement(panelElementName, relX + panelAbsX, relY + panelAbsY, nameFieldWidth - 2, entryHeight, select);
        relX += nameFieldWidth;

        for (int x = 0; x < numberEntrys; x++) {
            if (entrys[x].type == EMPTY) {
            }
            else {
                drawSettingElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth - 2, entryHeight, select);
            }

            relX += entryWidth;
            entrys[x].functionCW = {nullptr, ""};
            entrys[x].functionCCW = {nullptr, ""};
            entrys[x].functionPush = {nullptr, ""};
        }
    }

    else {
        if (select) {
            actionHandler.registerActionEncoder(5, entrys[0].functionCW, entrys[0].functionCCW,
                                                entrys[0].functionPush); // amount
        }
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
                        5, {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 1), "AMOUNT"},
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
        // drawRectangleFill(cClear, panelAbsX, panelAbsY, width, height);
        return;
    }

    if (select) {
        actionHandler.registerActionEncoder(0, entrys[0].functionCW, entrys[0].functionCCW, entrys[0].functionPush);
        actionHandler.registerActionEncoder(1, entrys[1].functionCW, entrys[1].functionCCW, entrys[1].functionPush);
        actionHandler.registerActionEncoder(2, entrys[2].functionCW, entrys[2].functionCCW, entrys[2].functionPush);
        actionHandler.registerActionEncoder(3, entrys[3].functionCW, entrys[3].functionCCW, entrys[3].functionPush);
    }
    entryWidth = (width + 2) / (numberEntrys);

    for (int x = 0; x < numberEntrys; x++) {
        if (entrys[x].type != EMPTY) {
            drawSettingElement(&entrys[x], relX + panelAbsX, relY + panelAbsY, entryWidth - 2, entryHeight, select,
                               keyColor);
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

    if (select) {
        drawRectangleFill(cHighlight, panelAbsX, panelAbsY, width, height);
        drawRectangleFill(cGrey, panelAbsX + 1, panelAbsY + 1, width - 2, height - 2);
    }
    else {
        drawRectangleFill(cGreyLight, panelAbsX, panelAbsY, width, height);
        drawRectangleFill(cGrey, panelAbsX + 1, panelAbsY + 1, width - 2, height - 2);
    }

    if (entry != nullptr) {

        uint32_t blocksize = (width - 2) / 4;

        uint8_t barHeigth = 8;
        // uint8_t barHeight = 4;

        // uint8_t color[4];
        float patchAmount = entry->amount;

        if (patchAmount < 0) {
            drawRectangleFill(cLayer, panelAbsX + width / 2 - (width / 2 - 3) * -patchAmount,
                              panelAbsY + height - 3 - barHeigth, (width / 2 - 3) * -patchAmount, barHeigth);
        }
        else {

            drawRectangleFill(cLayer, panelAbsX + width / 2, panelAbsY + height - 3 - barHeigth,
                              (width / 2 - 3) * patchAmount, barHeigth);
        }

        for (uint8_t y = 0; y < VOICESPERCHIP / 4; y++) {
            for (uint8_t x = 0; x < VOICESPERCHIP / 2; x++) {

                uint32_t index = x + y * 4;

                float amount = patchAmount * entry->sourceOut->currentSample[index];
                amount = abs(testFloat(amount, -1, 1));

                drawRectangleCentered(cHighlight, (blocksize * amount) / 2,
                                      blocksize / 2 + panelAbsX + x * blocksize + 2,
                                      blocksize / 2 + panelAbsY + y * blocksize + 2);
            }
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
                amount /= entry->max - entry->min;

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
            drawString(name, cFont_Select, panelAbsX + 3, panelAbsY + (height - fontMedium->size) / 2, fontMedium,
                       LEFT);
        }
        else {
            drawRectangleFill(cGreyLight, panelAbsX, panelAbsY, width, height);
            drawString(name, cFont_Deselect, panelAbsX + 3, panelAbsY + (height - fontMedium->size) / 2, fontMedium,
                       LEFT);
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

void registerDigitalElement(uint32_t index, Digital *data) {

    if (data == nullptr) {
        actionHandler.registerActionEncoder(index); // empty entry
        return;
    }

    actionHandler.registerActionEncoder(index, {std::bind(&Digital::nextValue, data), ""},
                                        {std::bind(&Digital::previousValue, data), ""},
                                        {std::bind(&Digital::resetValue, data), ""});
}

void drawCustomControls(BaseModule *module, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {

    uint32_t elementWidth = (w + 2) / 4;

    switch (module->moduleType) {
        case MODULE_OSC_A: {
            // register
            registerDigitalElement(0, &((OSC_A *)module)->dSample0);
            registerDigitalElement(1, &((OSC_A *)module)->dSample1);
            registerDigitalElement(2, &((OSC_A *)module)->dSample2);
            registerDigitalElement(3, &((OSC_A *)module)->dSample3);

            // draw
            drawCustomDigitalElement(&((OSC_A *)module)->dSample0, x, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((OSC_A *)module)->dSample1, x + elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((OSC_A *)module)->dSample2, x + 2 * elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((OSC_A *)module)->dSample3, x + 3 * elementWidth, y, elementWidth - 2, h);
            break;
        }

        case MODULE_OSC_B: {
            // register
            registerDigitalElement(0, &((OSC_B *)module)->dSample0);
            registerDigitalElement(1, &((OSC_B *)module)->dSample1);
            registerDigitalElement(2, &((OSC_B *)module)->dSample2);
            registerDigitalElement(3, &((OSC_B *)module)->dSample3);

            // draw
            drawCustomDigitalElement(&((OSC_B *)module)->dSample0, x, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((OSC_B *)module)->dSample1, x + elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((OSC_B *)module)->dSample2, x + 2 * elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((OSC_B *)module)->dSample3, x + 3 * elementWidth, y, elementWidth - 2, h);
            break;
        }

        case MODULE_MIX: {
            // register
            registerDigitalElement(0, &((Mixer *)module)->dOSCADestSwitch);
            registerDigitalElement(1, &((Mixer *)module)->dOSCBDestSwitch);
            registerDigitalElement(2, &((Mixer *)module)->dSUBDestSwitch);
            registerDigitalElement(3, &((Mixer *)module)->dNOISEDestSwitch);

            // draw
            drawCustomDigitalElement(&((Mixer *)module)->dOSCADestSwitch, x, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((Mixer *)module)->dOSCBDestSwitch, x + elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((Mixer *)module)->dSUBDestSwitch, x + 2 * elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((Mixer *)module)->dNOISEDestSwitch, x + 3 * elementWidth, y, elementWidth - 2,
                                     h);
            break;
        }

        case MODULE_ADSR: {
            // register
            registerDigitalElement(0, &((ADSR *)module)->dLoop);
            registerDigitalElement(1, &((ADSR *)module)->dLatch);
            registerDigitalElement(2, &((ADSR *)module)->dReset);
            registerDigitalElement(3, &((ADSR *)module)->dGateTrigger);

            // draw
            drawCustomDigitalElement(&((ADSR *)module)->dLoop, x, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((ADSR *)module)->dLatch, x + elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((ADSR *)module)->dReset, x + 2 * elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((ADSR *)module)->dClockTrigger, x + 3 * elementWidth, y, elementWidth - 2, h);

            break;
        }

        case MODULE_LFO: {
            // register
            registerDigitalElement(0, &((LFO *)module)->dFreqSnap);
            registerDigitalElement(1, &((LFO *)module)->dGateTrigger);
            registerDigitalElement(2, &((LFO *)module)->dClockTrigger);

            // draw
            drawCustomDigitalElement(&((LFO *)module)->dFreqSnap, x, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((LFO *)module)->dGateTrigger, x + elementWidth, y, elementWidth - 2, h);
            drawCustomDigitalElement(&((LFO *)module)->dClockTrigger, x + 2 * elementWidth, y, elementWidth - 2, h);

            if (liveData.livemodeClockSource.value) {
                registerDigitalElement(3, &((LFO *)module)->dClockStep);
                drawCustomDigitalElement(&((LFO *)module)->dClockStep, x + 3 * elementWidth, y, elementWidth - 2, h);
            }
            else {
                registerDigitalElement(3, &((LFO *)module)->dEXTDiv);
                drawCustomDigitalElement(&((LFO *)module)->dEXTDiv, x + 3 * elementWidth, y, elementWidth - 2, h);
            }
            break;
        }

        default: break;
    }
}

void drawWaveFromModule(WaveBuffer &buffer, BaseModule *module, uint32_t x, uint32_t y) {

    // empty buffer
    uint32_t data = 0x00000000;

    fastMemset(&data, (uint32_t *)*(buffer.buffer), (buffer.width * buffer.height) / 2);

    if (module->id == allLayers[module->layerId]->oscA.id) {

        drawWave(buffer, allLayers[module->layerId]->renderedAudioWavesOscA, 100, 2, c4444wavecolor, true);

        drawWaveTable(buffer, wavetables[((OSC_A *)module)->dSample0.valueMapped]->data, 0, buffer.width / 4,
                      (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawWaveTable(buffer, wavetables[((OSC_A *)module)->dSample1.valueMapped]->data, buffer.width / 4,
                      buffer.width / 4, (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawWaveTable(buffer, wavetables[((OSC_A *)module)->dSample2.valueMapped]->data, 2 * buffer.width / 4,
                      buffer.width / 4, (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawWaveTable(buffer, wavetables[((OSC_A *)module)->dSample3.valueMapped]->data, 3 * buffer.width / 4,
                      buffer.width / 4, (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawFrame(buffer, c4444framecolor);

        drawLine(buffer, 0, (buffer.height / 3) * 2, buffer.width, (buffer.height / 3) * 2, c4444wavecolor);

        drawLine(buffer, buffer.width / 4, (buffer.height / 3) * 2, buffer.width / 4, buffer.height, c4444wavecolor);
        drawLine(buffer, 2 * buffer.width / 4, (buffer.height / 3) * 2, 2 * buffer.width / 4, buffer.height,
                 c4444wavecolor);
        drawLine(buffer, 3 * buffer.width / 4, (buffer.height / 3) * 2, 3 * buffer.width / 4, buffer.height,
                 c4444wavecolor);
    }
    else if (module->id == allLayers[module->layerId]->oscB.id) {
        drawWave(buffer, allLayers[module->layerId]->renderedAudioWavesOscB, 100, 2, c4444wavecolor, true);
        drawWaveTable(buffer, wavetables[((OSC_B *)module)->dSample0.valueMapped]->data, 0, buffer.width / 4,
                      (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawWaveTable(buffer, wavetables[((OSC_B *)module)->dSample1.valueMapped]->data, buffer.width / 4,
                      buffer.width / 4, (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawWaveTable(buffer, wavetables[((OSC_B *)module)->dSample2.valueMapped]->data, 2 * buffer.width / 4,
                      buffer.width / 4, (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawWaveTable(buffer, wavetables[((OSC_B *)module)->dSample3.valueMapped]->data, 3 * buffer.width / 4,
                      buffer.width / 4, (buffer.height / 3) * 2, buffer.height / 3, 186, 1, c4444wavecolor);

        drawFrame(buffer, c4444framecolor);

        drawLine(buffer, 0, (buffer.height / 3) * 2, buffer.width, (buffer.height / 3) * 2, c4444wavecolor);

        drawLine(buffer, buffer.width / 4, (buffer.height / 3) * 2, buffer.width / 4, buffer.height, c4444wavecolor);
        drawLine(buffer, 2 * buffer.width / 4, (buffer.height / 3) * 2, 2 * buffer.width / 4, buffer.height,
                 c4444wavecolor);
        drawLine(buffer, 3 * buffer.width / 4, (buffer.height / 3) * 2, 3 * buffer.width / 4, buffer.height,
                 c4444wavecolor);
    }
    else if (module->id == allLayers[module->layerId]->sub.id) {
        drawWave(buffer, allLayers[module->layerId]->renderedAudioWavesSub, 100, 2, c4444wavecolor);
        drawFrame(buffer, c4444framecolor);
    }
    else if (module->moduleType == MODULE_LFO) {
        int8_t wave[100];
        calculateLFOWave((LFO *)module, wave, 100);

        drawWave(buffer, wave, 100, 2, c4444wavecolor);
        drawFrame(buffer, c4444framecolor);
    }
    else if (module->moduleType == MODULE_PHASE) {
        drawWave(buffer, allLayers[module->layerId]->renderedAudioWavesOscA, 100, 1, c4444wavecolorTrans);
        drawPhaseshaper(buffer, (Phaseshaper *)module);
        drawFrame(buffer, c4444framecolor);
    }
    else if (module->moduleType == MODULE_ADSR) {
        drawADSR(buffer, (ADSR *)module);
        drawFrame(buffer, c4444framecolor);
    }
    else if (module->moduleType == MODULE_WAVESHAPER) {
        drawWave(buffer, allLayers[module->layerId]->renderedAudioWavesOscA, 100, 2, c4444wavecolorTrans);
        drawWaveshaper(buffer, (Waveshaper *)module);
        drawFrame(buffer, c4444framecolor);
    }

    SCB_CleanDCache_by_Addr((uint32_t *)buffer.buffer, buffer.height * buffer.width * 2);

    copyWaveBuffer(buffer, x, y);
}

void drawGrid(WaveBuffer &buffer, uint16_t color) {

    uint8_t grid = 12;

    for (uint16_t i = 1; i < grid; i++) {
        drawLine(buffer, i * ((float)buffer.width / grid), 0, i * ((float)buffer.width / grid), buffer.height - 1,
                 color);
    }
    drawLine(buffer, 0, buffer.height / 2, buffer.width - 1, buffer.height / 2, color);
}
void drawFrame(WaveBuffer &buffer, uint16_t color) {

    drawLine(buffer, 0, 0, buffer.width - 1, 0, color);
    drawLine(buffer, 0, buffer.height - 1, buffer.width - 1, buffer.height - 1, color);
    drawLine(buffer, 0, 0, 0, buffer.height - 1, color);
    drawLine(buffer, buffer.width - 1, 0, buffer.width - 1, buffer.height - 1, color);
}
void drawWave(WaveBuffer &buffer, int8_t *renderedWave, uint16_t samples, uint32_t repeats, uint16_t color,
              uint32_t reduceHeight) {

    int16_t height = buffer.height;

    if (reduceHeight) {
        height = (buffer.height / 3) * 2;
    }

    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;

    for (uint16_t i = 0; i < 100 * repeats - 1; i++) {
        x1 = ((float)buffer.width / (100.f * repeats - 1)) * i;
        x2 = ((float)buffer.width / (100.f * repeats - 1)) * (i + 1);
        y1 = -renderedWave[i % 100];
        y2 = -renderedWave[(i + 1) % 100];

        y1 = (float)(y1 + 128) * (height / 255.f);
        y2 = (float)(y2 + 128) * (height / 255.f);

        y1 = testInt(y1, 1, height - 2);
        y2 = testInt(y2, 1, height - 2);

        x1 = testInt(x1, 1, buffer.width - 1);
        x2 = testInt(x2, 1, buffer.width - 1);
        drawLineThick(buffer, x1, y1, x2, y2, color);
    }
}

void drawWaveTable(WaveBuffer &buffer, const float *wavetable, uint16_t x_offset, uint16_t width, uint16_t y_offset,
                   uint16_t heigth, uint16_t samples, uint32_t repeats, uint16_t color) {

    int16_t x1;
    int16_t y1;

    int16_t x2;
    int16_t y2;
    uint32_t step = 4;

    for (uint16_t i = 0; i < samples * repeats - step; i += step) {
        x1 = ((float)width / ((float)samples * repeats - 1)) * i;
        y1 = (float)(-wavetable[i % samples] + 1.f) / 2.f * (float)(heigth);

        x2 = ((float)width / ((float)samples * repeats - 1)) * (i + step);
        y2 = (float)(-wavetable[(i + step) % samples] + 1.f) / 2.f * (float)(heigth);

        x1 = testInt(x1, 0, width - 1);
        y1 = testInt(y1, 0, heigth - 2);

        x2 = testInt(x2, 0, width - 1);
        y2 = testInt(y2, 0, heigth - 2);

        drawLineThick(buffer, x_offset + x1, y_offset + y1, x_offset + x2, y_offset + y2, color);
    }
}

void drawVecWave(WaveBuffer &buffer, vec<2> *renderedWave, uint16_t samples) {

    vec<2> p1;
    vec<2> p2;

    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
    // Grid

    uint8_t grid = 12;

    for (uint16_t i = 1; i < grid; i++) {
        drawLine(buffer, i * ((float)buffer.width / grid), 0, i * ((float)buffer.width / grid), buffer.height - 1,
                 c4444gridcolor);
    }
    drawLine(buffer, 0, buffer.height / 2, buffer.width - 1, buffer.height / 2, c4444framecolor);

    // Frame
    drawLine(buffer, 0, 0, buffer.width - 1, 0, c4444framecolor);
    drawLine(buffer, 0, buffer.height - 1, buffer.width - 1, buffer.height - 1, c4444framecolor);
    drawLine(buffer, 0, 0, 0, buffer.height - 1, c4444framecolor);
    drawLine(buffer, buffer.width - 1, 0, buffer.width - 1, buffer.height - 1, c4444framecolor);

    for (uint16_t i = 0; i < (samples - 1); i++) {
        // load Wave
        p1 = renderedWave[i];
        p2 = renderedWave[i + 1];

        // Check range
        x1 = testInt((int32_t)p1[0], 0, buffer.width - 1);
        y1 = testInt((int32_t)p1[1], 0, buffer.height - 1);
        x2 = testInt((int32_t)p2[0], 0, buffer.width - 1);
        y2 = testInt((int32_t)p2[1], 0, buffer.height - 1);

        drawLineWidth(buffer, x1, y1, x2, y2, 4, c4444wavecolor);
    }
}

#include "render/renderLFO.cpp"

void calculateLFOWave(LFO *module, int8_t *renderedWave, uint16_t samples) {

    const float random[10] = {-.8, 0.6, 0, 1, -0.3, 0.2, 0, -0.9, 0.5};

    float phase = 0;

    float shape = testFloat(module->shapeRAW[0], module->aShape.min, module->aShape.max - 0.001);

    shape *= 7;

    float index;

    for (uint16_t i = 0; i < samples; i++) {
        phase = (1.f / samples) * i;

        phase = phase - floor(phase);

        float fract = shape - std::floor(shape);

        if (shape < 1) {
            index = fast_lerp_f32(calcSin(phase), calcInvRamp(phase), fract);
        }
        // else if (shape < 2) {
        //     index = calcRampTriangle(phase, shape);
        // }
        else if (shape < 3) {
            index = calcRampTriangle(phase, shape);
        }
        else if (shape < 4) {

            float randPhasef = phase * 3.999f;
            uint16_t randPhase = randPhasef;
            float randPhaseFrac = randPhasef - std::floor(randPhasef);

            index = fast_lerp_f32(calcRamp(phase),

                                  fast_lerp_f32(

                                      random[changeIntLoop(randPhase, -1, 0, 3)],

                                      random[randPhase],

                                      randPhaseFrac),
                                  fract);
        }
        else if (shape < 5) {

            float randPhasef = phase * 3.999f;
            uint16_t randPhase = randPhasef;
            float randPhaseFrac = std::clamp((randPhasef - std::floor(randPhasef)) * 1.0f / (1.0f - fract), 0.0f, 1.0f);

            index = fast_lerp_f32(random[changeIntLoop(randPhase, -1, 0, 3)], random[randPhase], randPhaseFrac);
        }

        else if (shape < 6) {

            float randPhasef = phase * 3.999f;
            uint16_t randPhase = randPhasef;

            index = fast_lerp_f32(random[randPhase], calcSquare(phase, shape), fract);
        }

        else {
            index = calcSquare(phase, shape);
        }

        renderedWave[i] = index * 127;
    }
}

void calculateNoiseWave(Noise *module, int8_t *renderedWave, uint16_t samples) {
    static uint8_t init = false;
    static int8_t random[100];

    if (init == false) {
        init = true;
        for (size_t i = 0; i < 100; i++) {
            random[i] = (((float)rand() / (RAND_MAX / 2)) - 1) * 127;
        }
    }
    uint32_t sampleCrushCount = 0;
    int8_t sample = 0;
    int8_t newSample;

    for (uint16_t i = 0; i < samples; i++) {
        newSample = random[i];

        sampleCrushCount = sampleCrushCount * std::ceil((float)i / samples);
        sampleCrushCount = sampleCrushCount + 24;
        bool sampleCrushNow = sampleCrushCount > (module->samplecrusher[0] * 960);

        sampleCrushCount *= !sampleCrushNow;
        sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

        renderedWave[i] = sample;
    }
}

void drawADSR(WaveBuffer &buffer, ADSR *module) {

    float range = 4.2;

    float delay = fastMap(module->aDelay, module->aDelay.min, module->aDelay.max, 0.04, 1);
    float attack = fastMap(module->aAttack, module->aAttack.min, module->aAttack.max, 0.04, 1);
    float decay = fastMap(module->aDecay, module->aDecay.min, module->aDecay.max, 0.04, 1);
    float release = fastMap(module->aRelease, module->aRelease.min, module->aRelease.max, 0.04, 1);
    float sustain = range - attack - decay - release - delay;
    float sustainLevel = fastMap(module->aSustain, module->aSustain.min, module->aSustain.max, 0.00, 1);

    float shape = module->aShape;

    // Attack
    vec<2> P[6];
    P[0] = 0;
    P[1][0] = delay / range;
    P[1][1] = 0;

    P[2][0] = P[1][0] + (attack / range);
    P[2][1] = 1;

    P[3][0] = P[2][0] + decay / range;
    P[3][1] = sustainLevel;

    P[4] = P[3];
    P[4][0] += sustain / range;

    P[5][0] = P[4][0] + release / range;
    P[5][1] = 0;

    for (uint32_t i = 0; i < 6; i++) {
        // Flip Wave
        P[i][1] = 1 - P[i][1];

        // Scale Wave
        P[i][0] *= buffer.width - 1;
        P[i][1] *= buffer.height - 14;
        P[i][1] += 7;
    }

    // shape Attack
    vec<2> A1;
    vec<2> A2;
    vec<2> A;

    A1 = (P[1] + P[2]) / 2.f;
    A2[0] = P[1][0];
    A2[1] = P[2][1];

    A = fast_lerp_f32(A1, A2, shape);

    // shape Decay

    vec<2> D1;
    vec<2> D2;
    vec<2> D;

    D1 = (P[2] + P[3]) / 2.f;
    D2[0] = P[2][0];
    D2[1] = P[3][1];

    D = fast_lerp_f32(D1, D2, shape);

    // shape Release

    vec<2> R1;
    vec<2> R2;
    vec<2> R;

    R1 = (P[4] + P[5]) / 2.f;
    R2[0] = P[4][0];
    R2[1] = P[5][1];

    R = fast_lerp_f32(R1, R2, shape);

    // Draw Wave
    drawLineThick(buffer, P[0][0], P[0][1], P[1][0], P[1][1], c4444wavecolor);
    drawQuadBezier(buffer, P[1][0], P[1][1], A[0], A[1], P[2][0], P[2][1], c4444wavecolor);
    drawQuadBezier(buffer, P[2][0], P[2][1], D[0], D[1], P[3][0], P[3][1], c4444wavecolor);

    drawLineThick(buffer, P[3][0], P[3][1], P[4][0], P[4][1], c4444wavecolor);
    drawQuadBezier(buffer, P[4][0], P[4][1], R[0], R[1], P[5][0], P[5][1], c4444wavecolor);

    drawFilledCircle(buffer, P[1][0], P[1][1], c4444dot, 8);
    drawFilledCircle(buffer, P[2][0], P[2][1], c4444dot, 8);
    drawFilledCircle(buffer, P[3][0], P[3][1], c4444dot, 8);
    drawFilledCircle(buffer, P[4][0], P[4][1], c4444dot, 8);
}

void drawPhaseshaper(WaveBuffer &buffer, Phaseshaper *module) {
    vec<2> P1;
    vec<2> P2;
    vec<2> P3;
    vec<2> P4;

    // Invert/Scale
    P1[0] = 0;
    P1[1] = 7 + (1 - module->Point1Y[0]) * (buffer.height - 14);

    P2[0] = module->Point2X[0] * (buffer.width - 1);
    P2[1] = 7 + (1 - module->Point2Y[0]) * (buffer.height - 14);

    P3[0] = module->Point3X[0] * (buffer.width - 1);
    P3[1] = 7 + (1 - module->Point3Y[0]) * (buffer.height - 14);

    P4[0] = buffer.width - 1;
    P4[1] = 7 + (1 - module->Point4Y[0]) * (buffer.height - 14);

    drawLineWidth(buffer, P1[0], P1[1], P2[0], P2[1], 3, c4444wavecolor);
    drawLineWidth(buffer, P2[0], P2[1], P3[0], P3[1], 3, c4444wavecolor);
    drawLineWidth(buffer, P3[0], P3[1], P4[0], P4[1], 3, c4444wavecolor);

    // Dots
    drawFilledCircle(buffer, P2[0], P2[1], c4444dot, 8);
    drawFilledCircle(buffer, P3[0], P3[1], c4444dot, 8);
}

void drawWaveshaper(WaveBuffer &buffer, Waveshaper *module) {

    __disable_irq();

    if ((module->Point1X[0] < module->Point2X[0]) && (module->Point2X[0] < module->Point3X[0])) {

        // module->splineX[0].clear();
        // module->splineY[0].clear();

        // module->splineX[0].push_back(0.0f);
        module->splineX[0][1] = module->Point1X[0];
        module->splineX[0][2] = module->Point2X[0];
        module->splineX[0][3] = module->Point3X[0];
        // module->splineX[0].push_back(1.0f);

        // module->splineY[0].push_back(0.0f);
        module->splineY[0][1] = module->Point1Y[0];
        module->splineY[0][2] = module->Point2Y[0];
        module->splineY[0][3] = module->Point3Y[0];
        module->splineY[0][4] = module->Point4Y[0];
        __enable_irq();
        module->wavespline[0].set_points(module->splineX[0], module->splineY[0]);
    }

    __enable_irq();

    int xdots[4];
    int ydots[4];
    // Invert/Scale
    xdots[0] = module->splineX[0][1] * (buffer.width - 1);
    ydots[0] = 7 + (1 - module->Point1Y[0]) * (buffer.height - 14);

    xdots[1] = module->splineX[0][2] * (buffer.width - 1);
    ydots[1] = 7 + (1 - module->Point2Y[0]) * (buffer.height - 14);

    xdots[2] = module->splineX[0][3] * (buffer.width - 1);
    ydots[2] = 7 + (1 - module->Point3Y[0]) * (buffer.height - 14);

    xdots[3] = 1 * (buffer.width - 1);
    ydots[3] = 7 + (1 - module->Point4Y[0]) * (buffer.height - 14);

    // drawLineThick(buffer, 0, y[0], x[0], y[0], c4444wavecolor);

    int32_t previousYInt = 0;

    for (uint32_t i = 0; i < buffer.width; i++) {
        float y = module->wavespline[0]((float)i / (buffer.width - 1));
        int32_t yInt = 7 + (1.0f - y) * (buffer.height - 14);

        if (std::abs(yInt - previousYInt) < 4 || i == 0)
            drawPixelThick(buffer, i, yInt, c4444wavecolor);
        else
            drawLineThick(buffer, i - 1, previousYInt, i, yInt, c4444wavecolor);

        previousYInt = yInt;
    }

    // drawCubicSpline(buffer, 3, x, y, c4444wavecolor);

    // Dots
    drawFilledCircle(buffer, xdots[0], ydots[0], c4444dot, 8);
    drawFilledCircle(buffer, xdots[1], ydots[1], c4444dot, 8);
    drawFilledCircle(buffer, xdots[2], ydots[2], c4444dot, 8);
    drawFilledCircle(buffer, xdots[3], ydots[3], c4444dot, 8);
}

void Effect_PanelElement::Draw() {
    uint16_t relX = 0;
    uint16_t relY = 0;

    entryHeight = height;

    if (visible) {

        if (select) {
            for (uint8_t i = 0; i < 4; i++) {
                if (entrys[i] != nullptr) {
                    actionHandler.registerActionEncoder(
                        i, {std::bind(&Analog::changeValueWithEncoderAcceleration, entrys[i], 1), "AMOUNT"},
                        {std::bind(&Analog::changeValueWithEncoderAcceleration, entrys[i], 0), "AMOUNT"},
                        {std::bind(&Analog::resetValue, entrys[i]), "RESET"});
                }
                else {
                    actionHandler.registerActionEncoder(i); // clear encoder
                }
            }
        }

        entryWidth = width / numberEntrys;

        for (int i = 0; i < numberEntrys; i++) {
            if (entrys[i] == nullptr) {
            }
            else {
                drawSmallAnalogElement(entrys[i], relX + panelAbsX + 1, relY + panelAbsY, entryWidth - 2, entryHeight,
                                       select);
                entrys[i] = nullptr;
            }

            relX += entryWidth;
        }
    }

    numberEntrys = 0;
    select = 0;
}

void EffectAmount_PanelElement::Draw() {
    uint16_t relX = 0;
    uint16_t relY = 0;

    entryHeight = height;

    if (!visible) {
        // drawRectangleFill(cClear, panelAbsX, panelAbsY, width, height);
        return;
    }

    entryWidth = width / (numberEntrys + 1);

    // Feld mit GruppenNamen

    drawSmallAnalogElement(effect, relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select, true);
    relX += entryWidth;

    // drawNameElement(&name, relX + panelAbsX, relY + panelAbsY, entryWidth, entryHeight, select);
    // relX += entryWidth;

    if (select) {
        for (uint8_t i = 0; i < 4; i++) {
            if (entrys[i] != nullptr) {
                actionHandler.registerActionEncoder(
                    i, {std::bind(&Analog::changeValueWithEncoderAcceleration, entrys[i], 1), entrys[i]->getName()},
                    {std::bind(&Analog::changeValueWithEncoderAcceleration, entrys[i], 0), entrys[i]->getName()},
                    {std::bind(&Analog::resetValue, entrys[i]), "RESET"});
            }
            else {
                actionHandler.registerActionEncoder(i); // clear encoder
            }
        }
    }

    for (int i = 0; i < numberEntrys; i++) {
        if (entrys[i] == nullptr) {
        }
        else {
            drawSmallAnalogElement(entrys[i], relX + panelAbsX + 1, relY + panelAbsY, entryWidth - 2, entryHeight,
                                   select, moduleName[i]);
            entrys[i] = nullptr;
        }

        relX += entryWidth;
    }
    numberEntrys = 0;
    select = 0;
}

void drawQuickViewAnalog(Analog *data, uint32_t x, uint32_t y, uint16_t w, uint16_t h) {
    uint16_t spaceLeftRightBar = 15;
    uint16_t spaceTopBottomBar = 6;

    // clear
    drawRectangleChampfered(cGreyDark, x, y, w, h, 1);
    drawRectangleChampfered(cGreyLight, x, y, w, h / 2, 1);

    // get text
    std::string text = data->getName();

    // Draw Name

    drawString(text, cFont_Deselect, x + w / 2, y + (-fontMedium->size + h / 2) / 2, fontMedium, CENTER);

    uint16_t valueBarWidth = w - 2 * spaceLeftRightBar;
    uint16_t valueBarHeigth = h / 2 - 2 * spaceTopBottomBar;

    uint16_t relX = spaceLeftRightBar;
    uint16_t relY = h / 2 + spaceTopBottomBar;

    // valueBar
    if (data->min < 0) { // Centered ValueBar -> expect symmetric range

        uint16_t centerX = relX + x + valueBarWidth / 2;

        if (data->value >= ((int32_t)(data->inputRange / 2) + data->minInputValue)) { // positive value
            valueBarWidth = (float)valueBarWidth * (data->value - data->minInputValue - data->inputRange / 2) /
                            (float)(data->inputRange);

            drawRectangleFill(cLayer, centerX, relY + y, valueBarWidth, valueBarHeigth);
        }
        else { // negative value

            valueBarWidth = (float)valueBarWidth * ((data->inputRange / 2) - (data->value - data->minInputValue)) /
                            (float)(data->inputRange);

            drawRectangleFill(cLayer, centerX - valueBarWidth, relY + y, valueBarWidth, valueBarHeigth);
        }
    }
    else {
        valueBarWidth = (float)valueBarWidth * (data->value - data->minInputValue) /
                        (float)(data->maxInputValue - data->minInputValue);

        drawRectangleFill(cLayer, relX + x, relY + y, valueBarWidth, valueBarHeigth);
    }

    if (data->input != nullptr && data->input->renderBuffer != nullptr) { // draw real value

        uint32_t marker = 2;

        valueBarWidth = w - 2 * spaceLeftRightBar;
        uint32_t valuePos = 0;

        for (uint32_t i = 0; i < NUMBERVOICES; i++) {
            float value = data->input->renderBuffer->currentSample[i];
            value = testFloat(value, data->min, data->max);

            if (data->min < 0) { // Centered ValueBar -> expect symmetric range
                uint16_t centerX = relX + x + valueBarWidth / 2;

                if (value >= ((int32_t)((data->max - data->min) / 2) + data->min)) { // positive value

                    valuePos = centerX + ((float)valueBarWidth * (value - data->min - (data->max - data->min) / 2) /
                                          (float)((data->max - data->min)));
                }
                else { // negative value

                    valuePos = centerX - ((float)valueBarWidth * (((data->max - data->min) / 2) - (value - data->min)) /
                                          (float)((data->max - data->min)));
                }
            }
            else {
                valuePos = (float)valueBarWidth * (value - data->min) / (float)(data->max - data->min) + relX + x;
            }

            drawRectangleFill(cHighlight, valuePos, relY + y, marker, valueBarHeigth);
        }
    }
}

void drawQuickViewDigital(Digital *data, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    uint32_t nameWidth = w;
    uint32_t valueBarHeigth = 6;
    uint32_t dataHeight = h - valueBarHeigth;
    uint32_t fontShiftHeight = h / 4;

    // get text
    std::string text = data->getName();

    drawRectangleChampfered(cGrey, x, y, w, h, 1);

    drawRectangleChampfered(cLayer, x, y, w, dataHeight / 2, 1);
    drawString(text, cFont_Select, x + nameWidth / 2, y + (-fontMedium->size + dataHeight) / 2 - fontShiftHeight,
               fontMedium, CENTER);

    text = data->getValueAsString();

    drawString(text, cFont_Deselect, x + w / 2, y + (-fontMedium->size + dataHeight) / 2 + fontShiftHeight, fontMedium,
               CENTER); // center Text

    uint32_t relY = h - valueBarHeigth;
    uint32_t valueBarWidth = 0;

    valueBarWidth = (float)w * ((float)data->valueMapped - data->min) / (float)(data->max - data->min);
    drawRectangleChampfered(cLayer, x, relY + y, valueBarWidth, valueBarHeigth, 1);
}

#endif