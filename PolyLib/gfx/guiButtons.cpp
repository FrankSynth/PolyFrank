#ifdef POLYCONTROL
#include "guiButtons.hpp"

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection

void Header_PanelBox::Draw(GUIPanelBase *panel, uint8_t active) {
    if (panel == nullptr)
        return;

    if (active) {
        drawRectangleFill(cHighlight, x, y, width, heigth);
        drawString(panel->name, cFont_Select, x + width / 2, y + (-font->size + heigth) / 2, font, CENTER);
    }
    else {
        // drawRectangleFill(cGreyLight, x, y, width, heigth);
        drawString(panel->name, cFont_Deselect, x + width / 2, y + (-font->size + heigth) / 2, font, CENTER);
    }
}

void Footer_PanelBox::Draw() {
    std::string mainName = main->name;
    drawString(mainName, cWhite, x + width / 2, y + (-fontBig->size + heigth) / 2, fontBig, CENTER);
}

void Side_PanelBox::Draw() {
    std::string mainName = actionHandle->handle.name;

    if (mainName.empty()) {
        return;
    }

    if (globalSettings.shift || actionHandle->unlock) {
        if (actionHandle->data != nullptr) {
            if (*(actionHandle->data)) {
                drawRectangleFill(cHighlight, x, y, width, heigth);
                drawStringVertical(mainName, cFont_Select, x + width / 2, y + heigth / 2, fontSmall);
            }
            else {
                // drawRectangleFill(cGreyLight, x, y, width, heigth);
                drawStringVertical(mainName, cWhite, x + width / 2, y + heigth / 2, fontSmall);
            }
        }
        else if (actionHandle->state == PRESSED) {
            drawRectangleFill(cHighlight, x, y, width, heigth);

            drawStringVertical(mainName, cFont_Select, x + width / 2, y + heigth / 2, fontSmall);
        }

        else {
            drawStringVertical(mainName, cWhite, x + width / 2, y + heigth / 2, fontSmall);
        }
    }
    else {
        if (actionHandle->data != nullptr) {
            if (*(actionHandle->data)) {
                drawRectangleFill(cWarning, x, y, width, heigth);
                drawStringVertical(mainName, cBlack, x + width / 2, y + heigth / 2, fontSmall);
            }
            else {
                // drawRectangleFill(cGreyLight, x, y, width, heigth);
                drawStringVertical(mainName, cWhite, x + width / 2, y + heigth / 2, fontSmall);
            }
        }
        else if (actionHandle->state == PRESSED) {
            drawRectangleFill(cWarning, x, y, width, heigth);

            drawStringVertical(mainName, cBlack, x + width / 2, y + heigth / 2, fontSmall);
        }

        else {

            drawStringVertical(mainName, cWhiteLight, x + width / 2, y + heigth / 2, fontSmall);
        }
    }
}

void GUIHeader::init(std::vector<GUIPanelBase *> *panels, uint8_t *activePanelID, uint32_t width, uint32_t height,
                     uint32_t x, uint32_t y) {
    panelCount = 4;
    panelWidth = (width + 2) / panelCount;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->activePanelID = activePanelID;
    this->panels = panels;

    // create Header Boxes;
    for (unsigned int p = 0; p < panelCount; p++) {
        boxes.push_back(Header_PanelBox(panelWidth * p + panelAbsX, 0 + panelAbsY, panelWidth - 2, panelHeight));
    }
}
void GUIHeader::Draw() {

    GUIPanelBase *entry;
    for (unsigned int i = 0; i < panelCount; i++) {
        if ((i + globalSettings.shift * panelCount) < (panels->size())) {
            entry = (*panels)[i + globalSettings.shift * panelCount];
            actionHandler.registerActionHeader(i,
                                               {std::bind(setPanelActive, i + globalSettings.shift * panelCount), ""});
        }
        else {
            entry = nullptr;
            actionHandler.registerActionHeader(i);
        }

        boxes[i].Draw(entry, (i + globalSettings.shift * panelCount) == *activePanelID);
    }
    for (unsigned int p = 1; p < panelCount; p++) {
        drawRectangleFill(cGreyLight, panelWidth * p + panelAbsX - 2, 0 + panelAbsY, 2, panelHeight);
    }
}

void GUIFooter::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = (width + 2) / panelCount;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

    // create Header Boxes and linkt to action Handler;
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[0].handle_CW, &actionHandler.encoder[0].handle_PUSH,
                                    panelWidth * 0 + panelAbsX, panelAbsY - panelHeight, panelWidth - 2, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[1].handle_CW, &actionHandler.encoder[1].handle_PUSH,
                                    panelWidth * 1 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[2].handle_CW, &actionHandler.encoder[2].handle_PUSH,
                                    panelWidth * 2 + panelAbsX, panelAbsY - panelHeight, panelWidth - 2, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder[3].handle_CW, &actionHandler.encoder[3].handle_PUSH,
                                    panelWidth * 3 + panelAbsX, panelAbsY - panelHeight, panelWidth - 2, panelHeight));
}
void GUIFooter::Draw() {
    // draw boxes

    for (Footer_PanelBox i : boxes) {
        i.Draw();
    }
    // draw Spacer
    for (unsigned int p = 1; p < panelCount; p++) {
        drawRectangleFill(cGreyLight, panelWidth * p + panelAbsX - 2, (int32_t)panelAbsY - panelHeight, 2, panelHeight);
    }
}

void GUISide::init(uint32_t width, uint32_t height, uint32_t y, uint32_t x) {
    panelWidth = width;
    panelHeight = (height) / panelCount;
    panelAbsY = y;
    panelAbsX = x;

    // boxes on the left
    boxes.push_back(Side_PanelBox(&actionHandler.buttonLeft[0], panelAbsX, panelAbsY, panelWidth, panelHeight));
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft[1], panelAbsX, panelAbsY + panelHeight, panelWidth, panelHeight));
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft[2], panelAbsX, panelAbsY + panelHeight * 2, panelWidth, panelHeight));

    // boxes on the rigth
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonRight[0], LCDWIDTH - panelWidth, panelAbsY, panelWidth, panelHeight));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight[1], LCDWIDTH - panelWidth, panelAbsY + panelHeight,
                                  panelWidth, panelHeight));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight[2], LCDWIDTH - panelWidth, panelAbsY + panelHeight * 2,
                                  panelWidth, panelHeight));
}
void GUISide::Draw() {
    // draw boxes

    for (Side_PanelBox i : boxes) {
        i.Draw();
    }
}

#endif // ifdef POLYCONTROL
