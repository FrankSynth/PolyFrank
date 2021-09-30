#ifdef POLYCONTROL
#include "guiButtons.hpp"

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection

void Header_PanelBox::Draw(uint8_t active) {
    if (active) {
        drawRectangleFill(cHighlight, x, y, width, heigth);
        drawString(pSource->name, cFont_Select, x + width / 2, y + (-font->size + heigth) / 2, font, CENTER);
    }
    else {
        drawRectangleFill(cGreyLight, x, y, width, heigth);
        drawString(pSource->name, cFont_Deselect, x + width / 2, y + (-font->size + heigth) / 2, font, CENTER);
    }
}

void Footer_PanelBox::Draw() {
    std::string mainName = main->name;
    std::string secondName = secondary->name;

    if (mainName.empty()) {
        mainName = "-";
    }
    if (secondName.empty()) {
        secondName = "-";
    }

    drawRectangleFill(cClear, x, y, width, heigth);

    drawString(mainName, cWhite, x + width / 2, y - fontBig->size + heigth / 2 - fontShift, fontBig, CENTER);
    drawString(secondName, cWhite, x + width / 2, y + heigth / 2 + fontShift, fontSmall, CENTER);
}

void Side_PanelBox::Draw() {
    std::string mainName = actionHandle->handle.name;

    if (mainName.empty()) {
        return;
    }

    if (actionHandler.unlocked || actionHandle->unlock) {
        if (actionHandle->data != nullptr) {
            if (*(actionHandle->data)) {
                drawRectangleFill(cHighlight, x, y, width, heigth);
                drawStringVertical(mainName, cFont_Select, x + width / 2, y + heigth / 2, fontSmall);
            }
            else {
                drawRectangleFill(cGreyLight, x, y, width, heigth);
                drawStringVertical(mainName, cFont_Deselect, x + width / 2, y + heigth / 2, fontSmall);
            }
        }
        else if (actionHandle->state == PRESSED) {
            drawRectangleFill(cHighlight, x, y, width, heigth);

            drawStringVertical(mainName, cFont_Select, x + width / 2, y + heigth / 2, fontSmall);
        }

        else {
            drawRectangleFill(cGreyLight, x, y, width, heigth);

            drawStringVertical(mainName, cFont_Deselect, x + width / 2, y + heigth / 2, fontSmall);
        }
    }
    else {
        if (actionHandle->data != nullptr) {
            if (*(actionHandle->data)) {
                drawRectangleFill(cWarning, x, y, width, heigth);
                drawStringVertical(mainName, cBlack, x + width / 2, y + heigth / 2, fontSmall);
            }
            else {
                drawRectangleFill(cGreyLight, x, y, width, heigth);
                drawStringVertical(mainName, cBlack, x + width / 2, y + heigth / 2, fontSmall);
            }
        }
        else if (actionHandle->state == PRESSED) {
            drawRectangleFill(cWarning, x, y, width, heigth);

            drawStringVertical(mainName, cBlack, x + width / 2, y + heigth / 2, fontSmall);
        }

        else {
            drawRectangleFill(cGreyLight, x, y, width, heigth);

            drawStringVertical(mainName, cBlack, x + width / 2, y + heigth / 2, fontSmall);
        }
    }
}

void GUIHeader::init(std::vector<GUIPanelBase *> *panels, uint8_t *activePanelID, uint16_t width, uint16_t height,
                     uint16_t x, uint16_t y) {
    panelCount = 4;
    panelWidth = width / 4;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->activePanelID = activePanelID;

    // create Header Boxes;
    for (unsigned int p = 0; p < panelCount; p++) {
        boxes.push_back(
            Header_PanelBox((*panels)[p], panelWidth * p + panelAbsX, 0 + panelAbsY, panelWidth, panelHeight));
    }
}
void GUIHeader::Draw() {

    for (int i = 0; i < panelCount; i++)
        boxes[i].Draw(i == *activePanelID);

    for (unsigned int p = 1; p < panelCount; p++) {
        drawRectangleFill(cWhite, panelWidth * p + panelAbsX, 0 + panelAbsY, 1, panelHeight);
    }
}

void GUIFooter::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width / panelCount;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

    // create Header Boxes and linkt to action Handler;
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[0].handle_CW, &actionHandler.encoder[0].handle_PUSH,
                                    panelWidth * 0 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[1].handle_CW, &actionHandler.encoder[1].handle_PUSH,
                                    panelWidth * 1 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[2].handle_CW, &actionHandler.encoder[2].handle_PUSH,
                                    panelWidth * 2 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[3].handle_CW, &actionHandler.encoder[3].handle_PUSH,
                                    panelWidth * 3 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
}
void GUIFooter::Draw() {

    // draw boxes

    for (Footer_PanelBox i : boxes) {
        i.Draw();
    }
    // draw Spacer
    for (unsigned int p = 1; p < panelCount; p++) {
        drawRectangleFill(cWhiteLight, panelWidth * p + panelAbsX, (int32_t)panelAbsY - panelHeight, 1, panelHeight);
    }
}

void GUISide::init(uint16_t width, uint16_t height, uint16_t y, uint16_t x) {
    panelWidth = width;
    panelHeight = height / panelCount;
    panelAbsY = y;
    panelAbsX = x;

    // boxes on the left
    boxes.push_back(Side_PanelBox(&actionHandler.buttonLeft[0], panelAbsX, panelAbsY + panelHeight * 0 + 2,
                                  panelWidth - 6, panelHeight - 4));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonLeft[1], panelAbsX, panelAbsY + panelHeight * 1 + 2,
                                  panelWidth - 6, panelHeight - 4));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonLeft[2], panelAbsX, panelAbsY + panelHeight * 2 + 2,
                                  panelWidth - 6, panelHeight - 4));

    // boxes on the rigth
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight[0], LCDWIDTH - panelWidth + 6,
                                  panelAbsY + panelHeight * 0 + 2, panelWidth - 6, panelHeight - 4));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight[1], LCDWIDTH - panelWidth + 6,
                                  panelAbsY + panelHeight * 1 + 2, panelWidth - 6, panelHeight - 4));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight[2], LCDWIDTH - panelWidth + 6,
                                  panelAbsY + panelHeight * 2 + 2, panelWidth - 6, panelHeight - 4));
}
void GUISide::Draw() {
    // draw boxes

    for (Side_PanelBox i : boxes) {
        i.Draw();
    }
}

#endif // ifdef POLYCONTROL
