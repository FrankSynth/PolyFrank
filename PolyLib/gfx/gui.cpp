#ifdef POLYCONTROL
#include "gui.hpp"

GUI ui;

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection

void Header_PanelBox::Draw(uint8_t active) {
    if (active) {
        drawRectangleFill(cWhite, x, y, width, heigth);
        drawString(pSource->name, cFont_Select, x + width / 2, y + (-font->size + heigth) / 2, font, CENTER);
    }
    else {
        drawRectangleFill(cClear, x, y, width, heigth);
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
    std::string mainName = main->name;

    if (mainName.empty()) {
        mainName = "-";
    }

    drawRectangleFill(cClear, x, y, width, heigth);

    drawString(mainName, cWhite, x + width / 2, y + (-fontSmall->size + heigth) / 2, fontSmall, CENTER);
}

//////PANELS/////
// PANEL Path Type

void GUIPanelPath::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelPath::Draw(uint8_t onlyLayer) {
    drawRectangleFill(cClear, panelAbsX, panelAbsY, panelWidth, panelHeight);
    uint16_t relX = 0;
    uint16_t relY = 0;

    uint16_t spacer = 20;

    //// Draw Layer Field////

    uint8_t layerID = allLayers[currentFocus.layer]->id; // get ID
    std::string text;
    if (layerID == 0) {
        text = "LAYER A"; // Layer name
    }
    else if (layerID == 1) {
        text = "LAYER B"; // Layer name
    }
    relX +=
        drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1, CENTER);
    relX += 1;

    if (onlyLayer) {
        return;
    }
    //// Draw Module Field////
    if (currentFocus.type >= FOCUSMODULE) {
        if (!allLayers[currentFocus.layer]->getModules().size()) { //  empty
            return;
        }
        std::string text = allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getName(); // Modul name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }

    //// Draw Focus Field////

    if (currentFocus.type == FOCUSOUTPUT) {
        if (!allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getOutputs().size()) { //  empty
            return;
        }
        std::string text = allLayers[currentFocus.layer]
                               ->getModules()[currentFocus.modul]
                               ->getOutputs()[currentFocus.id]
                               ->getName(); // Output name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
    else if (currentFocus.type == FOCUSINPUT) {
        if (!allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getInputs().size()) { //  empty
            return;
        }

        std::string text = allLayers[currentFocus.layer]
                               ->getModules()[currentFocus.modul]
                               ->getInputs()[currentFocus.id]
                               ->getName(); // Input name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
}

// PANEL Path Type

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

    for (int i = 0; i < panelCount; i++) {
        if (i == *activePanelID) {
            boxes[i].Draw(true);
        }
        else {
            boxes[i].Draw(false);
        }
    }

    for (unsigned int p = 1; p < panelCount; p++) {
        drawRectangleFill(cWhite, panelWidth * p + panelAbsX, 0 + panelAbsY, 1, panelHeight);
    }
}

void GUIFOOTER::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width / panelCount;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

    // create Header Boxes and linkt to action Handler;
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder1_CW, &actionHandler.encoder1_Push,
                                    panelWidth * 0 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder2_CW, &actionHandler.encoder2_Push,
                                    panelWidth * 1 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder3_CW, &actionHandler.encoder3_Push,
                                    panelWidth * 2 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
    boxes.push_back(Footer_PanelBox(&actionHandler.encoder4_CW, &actionHandler.encoder4_Push,
                                    panelWidth * 3 + panelAbsX, panelAbsY - panelHeight, panelWidth, panelHeight));
}
void GUIFOOTER::Draw() {

    // draw boxes

    for (Footer_PanelBox i : boxes) {
        i.Draw();
    }
    // draw Spacer
    for (unsigned int p = 1; p < panelCount; p++) {
        drawRectangleFill(cWhiteLight, panelWidth * p + panelAbsX, (int32_t)panelAbsY - panelHeight, 1, panelHeight);
    }
}

void GUISIDE::init(uint16_t width, uint16_t height, uint16_t y, uint16_t x) {
    panelWidth = width;
    panelHeight = height / panelCount;
    panelAbsY = y;
    panelAbsX = x;

    // boxes on the left
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft_1, panelAbsX, panelAbsY + panelHeight * 0, panelWidth, panelHeight));
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft_2, panelAbsX, panelAbsY + panelHeight * 1, panelWidth, panelHeight));
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft_3, panelAbsX, panelAbsY + panelHeight * 2, panelWidth, panelHeight));

    // boxes on the rigth
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight_1, LCDWIDTH - panelWidth, panelAbsY + panelHeight * 0,
                                  panelWidth, panelHeight));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight_2, LCDWIDTH - panelWidth, panelAbsY + panelHeight * 1,
                                  panelWidth, panelHeight));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight_3, LCDWIDTH - panelWidth, panelAbsY + panelHeight * 2,
                                  panelWidth, panelHeight));
}
void GUISIDE::Draw() {
    // draw boxes

    for (Side_PanelBox i : boxes) {
        i.Draw();
    }
}

void GUI::Init() { // add settings pointer
    // init Display
    GFX_Init();

    guiPanelFocus.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);
    guiPanelLive.init(CENTERWIDTH, CENTERHEIGHT - VOICEHEIGHT - SPACER, BOARDERWIDTH,
                      HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "LIVE", 0);
    guiPanel_1.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "PATCH", 1);
    guiPanel_2.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "PRESET", 2);
    guiPanel_3.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "CONFIG", 3);

    guiPanelVoice.init(CENTERWIDTH, VOICEHEIGHT, BOARDERWIDTH,
                       HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER + CENTERHEIGHT - VOICEHEIGHT, "CONFIG", 3);

    // add Panels to vector
    panels.push_back(&guiPanelLive);
    panels.push_back(&guiPanel_1);
    panels.push_back(&guiPanel_2);
    panels.push_back(&guiPanel_3);
    panels.push_back(&guiPanelFocus);

    // init Header
    guiHeader.init(&panels, &activePanelID, LCDWIDTH, HEADERHEIGHT);

    // init Footer
    guiFooter.init(LCDWIDTH, FOOTERHEIGHT);

    // init side
    guiSide.init(BOARDERWIDTH, CENTERHEIGHT, HEADERHEIGHT + SPACER);

    // init Footer
    guiFooter.init(LCDWIDTH, FOOTERHEIGHT);

    // init Path
    guiPath.init(CENTERWIDTH, FOCUSHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER);

    // init Error
    guiError.init(LCDWIDTH, LCDHEIGHT, 0, 0);

    // register Header action
    actionHandler.registerActionHeader(
        {std::bind(&GUI::setPanelActive, this, 0), "LIVEMODE"}, {std::bind(&GUI::setPanelActive, this, 1), "PATCH"},
        {std::bind(&GUI::setPanelActive, this, 2), "PRESET"}, {std::bind(&GUI::setPanelActive, this, 3), "CONFIG"});

    setPanelActive(3);

    // Set Focus for test
}

void GUI::Clear() {
    // clear

    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);
}

void GUI::Draw() {
    // setDisplayBrightness
    __HAL_TIM_SET_COMPARE(&htim13, TIM_CHANNEL_1,
                          globalSettings.dispBrightness.getValue() * 1000); // 6553* 1-10 -> 65530

    setRenderState(RENDER_PROGRESS);

    // clear
    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);

    // Error Occurred?
    if (!globalSettings.error.errorActive) {

        checkFocusChange();

        // Draw Panel
        if (activePanel != nullptr) {
            activePanel->Draw();

            if (activePanel == &guiPanelFocus) { // focus Panel draw complete Path

                guiPath.Draw();
            }

            else if (activePanel == &guiPanelLive) { // focus Panel draw complete Path

                guiPanelVoice.Draw();
            }
            else {
                // Path Visisble?
                if (activePanel->pathVisible) { // other Panel only show path to layer
                    // Draw Path
                    guiPath.Draw(1);
                }
            }
        }

        // Draw Header
        guiHeader.Draw();

        // Draw Footer
        guiFooter.Draw();
    }
    else {
        guiError.Draw();
    }

    // Draw Side
    guiSide.Draw();

    setRenderState(RENDER_WAIT);
}
void GUI::checkFocusChange() {

    if (newFocus.type != NOFOCUS) { // check new focus set and activate Focus Panel
        if (currentFocus.id != newFocus.id || currentFocus.modul != newFocus.modul ||
            currentFocus.layer != newFocus.layer || currentFocus.type != newFocus.type) { // something changed?
            oldActivePanelID = activePanelID;
            activePanel = panels[4];
            activePanelID = 4;
            currentFocus = newFocus;
            newFocus.type = NOFOCUS;
        }
        else { // nothing change -> same button pressed twice ->back to last Panel
            setPanelActive(4);
            newFocus.type = NOFOCUS;
        }
    }
}

// PanelSelect
void GUI::setPanelActive(uint8_t panelID) {
    // zurück zum letzen panel
    if (activePanelID == panelID) {
        activePanel = panels[oldActivePanelID];
        activePanelID = oldActivePanelID;
    }
    else {
        oldActivePanelID = activePanelID;
        activePanel = panels[panelID];
        activePanelID = panelID;
    }
}

#endif // ifdef POLYCONTROL
