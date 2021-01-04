#ifdef POLYCONTROL
#include "gui.hpp"

GUI ui;

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection

void Header_PanelBox::Draw() {
    if (pSource->active) {
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

void GUIPanelPath::Draw() {
    drawRectangleFill(cClear, panelAbsX, panelAbsY, panelWidth, panelHeight);
    uint16_t relX = 0;
    uint16_t relY = 0;

    uint16_t spacer = 20;

    //// Draw Layer Field////

    uint8_t layerID = allLayers[focus.layer]->id;          // get ID
    std::string text = "LAYER " + std::to_string(layerID); // Layer name

    relX +=
        drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1, CENTER);
    relX += 1;

    //// Draw Module Field////
    if (focus.type >= FOCUSMODULE) {
        if (!allLayers[focus.layer]->getModules().size()) { //  empty
            return;
        }
        std::string text = allLayers[focus.layer]->getModules()[focus.modul]->getName(); // Modul name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }

    //// Draw Focus Field////

    if (focus.type == FOCUSOUTPUT) {
        if (!allLayers[focus.layer]->getModules()[focus.modul]->getOutputs().size()) { //  empty
            return;
        }
        std::string text =
            allLayers[focus.layer]->getModules()[focus.modul]->getOutputs()[focus.id]->getName(); // Output name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
    else if (focus.type == FOCUSINPUT) {
        if (!allLayers[focus.layer]->getModules()[focus.modul]->getInputs().size()) { //  empty
            return;
        }

        std::string text =
            allLayers[focus.layer]->getModules()[focus.modul]->getInputs()[focus.id]->getName(); // Input name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
}

// PANEL Path Type

void GUIHeader::init(std::vector<GUIPanelBase *> *panels, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelCount = 4;
    panelWidth = width / 4;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

    // create Header Boxes;
    for (unsigned int p = 0; p < panelCount; p++) {
        boxes.push_back(
            Header_PanelBox((*panels)[p], panelWidth * p + panelAbsX, 0 + panelAbsY, panelWidth, panelHeight));
    }
}
void GUIHeader::Draw() {

    for (Header_PanelBox i : boxes) {
        i.Draw();
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

    guiPanelData.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);
    guiPanel_3.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "CONFIG", 3);
    guiPanel_1.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "CONFIG", 3);

    // add Panels to vector
    panels.push_back(&guiPanel_0);
    panels.push_back(&guiPanel_1);
    panels.push_back(&guiPanel_2);
    panels.push_back(&guiPanel_3);
    panels.push_back(&guiPanelData);

    // init Header
    guiHeader.init(&panels, LCDWIDTH, HEADERHEIGHT);

    // init Footer
    guiFooter.init(LCDWIDTH, FOOTERHEIGHT);

    // init side
    guiSide.init(BOARDERWIDTH, CENTERHEIGHT, HEADERHEIGHT + SPACER);

    // init Footer
    guiFooter.init(LCDWIDTH, FOOTERHEIGHT);

    // init Path
    guiPath.init(CENTERWIDTH, FOCUSHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER);

    // register Header action
    actionHandler.registerActionHeader(
        {std::bind(&GUI::setPanel0Active, this), "LIVEMODE"}, {std::bind(&GUI::setPanel1Active, this), "PATCH"},
        {std::bind(&GUI::setPanel2Active, this), "PRESET"}, {std::bind(&GUI::setPanel3Active, this), "CONFIG"});

    setPanelFocusActive();

    // Set Focus for test
    setFocus({0, 0, 0, FOCUSMODULE});
}

void GUI::Clear() {
    // clear

    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);
}

void GUI::Draw() {

    // setDisplayBrigthness
    __HAL_TIM_SET_COMPARE(&htim13, TIM_CHANNEL_1,
                          globalSettings.dispBrigthness.getValue() * 1000); // 6553* 1-10 -> 65530

    setRenderState(RENDER_PROGRESS);

    // clear
    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);

    // Draw Header
    guiHeader.Draw();

    // Draw Path
    guiPath.Draw();

    // Draw Side
    guiSide.Draw();

    // Draw Footer
    guiFooter.Draw();

    // Draw Panel
    if (activePanel != nullptr) {
        activePanel->Draw();
    }

    setRenderState(RENDER_WAIT);
}
void GUI::setFocus(location newFocus) {

    focus = newFocus;

    setPanelFocusActive(); // activate Focus Panel
}

// PanelSelect
void GUI::setPanel0Active() {
    guiPanel_0.active = 1;
    guiPanel_1.active = 0;
    guiPanel_2.active = 0;
    guiPanel_3.active = 0;
    guiPanelData.active = 0;

    setActivePanel(0);
}
void GUI::setPanel1Active() {
    guiPanel_0.active = 0;
    guiPanel_1.active = 1;
    guiPanel_2.active = 0;
    guiPanel_3.active = 0;
    guiPanelData.active = 0;

    setActivePanel(1);
}
void GUI::setPanel2Active() {
    guiPanel_0.active = 0;
    guiPanel_1.active = 0;
    guiPanel_2.active = 1;
    guiPanel_3.active = 0;
    guiPanelData.active = 0;

    setActivePanel(2);
}

void GUI::setPanel3Active() {
    guiPanel_0.active = 0;
    guiPanel_1.active = 0;
    guiPanel_2.active = 0;
    guiPanel_3.active = 1;
    guiPanelData.active = 0;

    setActivePanel(3);
}

void GUI::setPanelFocusActive() {
    guiPanel_0.active = 0;
    guiPanel_1.active = 0;
    guiPanel_2.active = 0;
    guiPanel_3.active = 0;
    guiPanelData.active = 1;
    setActivePanel(4);
}

void GUI::setActivePanel(uint8_t id) {
    activePanel = panels[id];
}

#endif // ifdef POLYCONTROL
