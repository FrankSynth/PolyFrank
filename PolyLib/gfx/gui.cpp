#ifdef POLYCONTROL

#include "gui.hpp"
#include "polyControl.hpp"
#include "tim.h"

GUI ui;

// colors
uint32_t cSelect = 0xD0FFFFFF;
uint32_t cDeselect = 0x00000000;
uint32_t cFont_Select = 0xFF000000;
uint32_t cFont_Deselect = 0xFFFFFFFF;

uint32_t cClear = 0x00000000;
uint32_t cBlack = 0xFF000000;

uint32_t cGreyLight = 0xd0707070;
uint32_t cGreyDark = 0xB0505050;

uint32_t cGrey = 0xB0505050;

uint32_t cWhite = 0xFFFFFFFF;
uint32_t cWhiteLight = 0xd0FFFFFF;

actionMapping actionHandler;

// Dummy Function for todos
void Todo() {}

uint16_t drawBoxWithText(std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText, uint16_t x,
                         uint16_t y, uint16_t heigth, uint16_t space, uint16_t champfer, FONTALIGN alignment) {
    uint16_t width = getStringWidth(text, font) + space; // text witdh + space

    drawRectangleChampfered(colorBox, x, y, width, heigth, champfer); // draw Box
    drawString(text, colorText, x + space / 2, y + (-font->size + heigth) / 2, font,
               LEFT); // draw text, height centered

    return width;
}

void actionMapping::registerActionHeader(actionHandle handle1, actionHandle handle2, actionHandle handle3,
                                         actionHandle handle4) {
    buttonHeader_1 = handle1;
    buttonHeader_2 = handle2;
    buttonHeader_3 = handle3;
    buttonHeader_4 = handle4;
}

void actionMapping::registerActionLeft(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    buttonLeft_1 = handle1;
    buttonLeft_2 = handle2;
    buttonLeft_3 = handle3;
}

void actionMapping::registerActionRight(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    buttonRight_1 = handle1;
    buttonRight_2 = handle2;
    buttonRight_3 = handle3;
}

void actionMapping::registerActionEncoder1(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder1_CW = handle1;
    encoder1_CCW = handle2;
    encoder1_Push = handle3;
}
void actionMapping::registerActionEncoder2(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder2_CW = handle1;
    encoder2_CCW = handle2;
    encoder2_Push = handle3;
}
void actionMapping::registerActionEncoder3(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder3_CW = handle1;
    encoder3_CCW = handle2;
    encoder3_Push = handle3;
}
void actionMapping::registerActionEncoder4(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder4_CW = handle1;
    encoder4_CCW = handle2;
    encoder4_Push = handle3;
}

void actionMapping::callActionHeader1() {
    if (buttonHeader_1.functionPointer != nullptr)
        buttonHeader_1.functionPointer();
};

void actionMapping::callActionHeader2() {
    if (buttonHeader_2.functionPointer != nullptr)
        buttonHeader_2.functionPointer();
};

void actionMapping::callActionHeader3() {
    if (buttonHeader_3.functionPointer != nullptr)
        buttonHeader_3.functionPointer();
};

void actionMapping::callActionHeader4() {
    if (buttonHeader_4.functionPointer != nullptr)
        buttonHeader_4.functionPointer();
};

void actionMapping::callActionLeft1() {
    if (buttonLeft_1.functionPointer != nullptr)
        buttonLeft_1.functionPointer();
};

void actionMapping::callActionLeft2() {
    if (buttonLeft_2.functionPointer != nullptr)
        buttonLeft_2.functionPointer();
};

void actionMapping::callActionLeft3() {
    if (buttonLeft_3.functionPointer != nullptr)
        buttonLeft_3.functionPointer();
};

void actionMapping::callActionRight1() {
    if (buttonRight_1.functionPointer != nullptr)
        buttonRight_1.functionPointer();
};

void actionMapping::callActionRight2() {
    if (buttonRight_2.functionPointer != nullptr)
        buttonRight_2.functionPointer();
};

void actionMapping::callActionRight3() {
    if (buttonRight_3.functionPointer != nullptr)
        buttonRight_3.functionPointer();
};

void actionMapping::callActionEncoder_1_CW() {
    if (encoder1_CW.functionPointer != nullptr)
        encoder1_CW.functionPointer();
};

void actionMapping::callActionEncoder_1_CCW() {
    if (encoder1_CCW.functionPointer != nullptr)
        encoder1_CCW.functionPointer();
};

void actionMapping::callActionEncoder_1_Push() {
    if (encoder1_Push.functionPointer != nullptr)
        encoder1_Push.functionPointer();
};

void actionMapping::callActionEncoder_2_CW() {
    if (encoder2_CW.functionPointer != nullptr)
        encoder2_CW.functionPointer();
};

void actionMapping::callActionEncoder_2_CCW() {
    if (encoder2_CCW.functionPointer != nullptr)
        encoder2_CCW.functionPointer();
};

void actionMapping::callActionEncoder_2_Push() {
    if (encoder2_Push.functionPointer != nullptr)
        encoder2_Push.functionPointer();
};

void actionMapping::callActionEncoder_3_CW() {
    if (encoder3_CW.functionPointer != nullptr)
        encoder3_CW.functionPointer();
};

void actionMapping::callActionEncoder_3_CCW() {
    if (encoder3_CCW.functionPointer != nullptr)
        encoder3_CCW.functionPointer();
};

void actionMapping::callActionEncoder_3_Push() {
    if (encoder3_Push.functionPointer != nullptr)
        encoder3_Push.functionPointer();
};

void actionMapping::callActionEncoder_4_CW() {
    if (encoder4_CW.functionPointer != nullptr)
        encoder4_CW.functionPointer();
};

void actionMapping::callActionEncoder_4_CCW() {
    if (encoder4_CCW.functionPointer != nullptr)
        encoder4_CCW.functionPointer();
};

void actionMapping::callActionEncoder_4_Push() {
    if (encoder4_Push.functionPointer != nullptr)
        encoder4_Push.functionPointer();
};

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection

void Header_PanelBox::Draw() {
    if (pSource->active) {
        drawRectangleFill(cSelect, x, y, width, heigth);
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

    drawString(mainName, cWhite, x + width / 2, y + (-fontBig->size + heigth) / 2 - fontShift, fontBig, CENTER);
    drawString(secondName, cWhite, x + width / 2, y + (-fontSmall->size + heigth) / 2 + fontShift, fontSmall, CENTER);
}

void Side_PanelBox::Draw() {
    std::string mainName = main->name;

    if (mainName.empty()) {
        mainName = "-";
    }

    drawRectangleFill(cClear, x, y, width, heigth);

    drawString(mainName, cWhite, x + width / 2, y + (-fontBig->size + heigth) / 2, fontBig, CENTER);
}

void Focus_PanelDataElement::Draw() {
    uint16_t relX = 0;
    uint16_t relY = 0;

    if (pSource == nullptr) {
        drawRectangleFill(cClear, panelAbsX, panelAbsY, width, heigth);

        return;
    }

    // clear
    drawRectangleChampfered(cGrey, panelAbsX, panelAbsY, width, heigth, 1);

    // get text
    std::string text = pSource->getName();

    // Draw Name
    if (selected) {
        drawRectangleChampfered(cWhite, panelAbsX, panelAbsY, nameWidth, heigth, 1);
        drawString(text, cFont_Select, panelAbsX + nameWidth / 2, panelAbsY + (-font->size + heigth) / 2, font, CENTER);
    }
    else {
        drawRectangleFill(cWhite, panelAbsX + nameWidth - 1, panelAbsY + 2, 1, heigth - 4);
        drawString(text, cFont_Deselect, panelAbsX + nameWidth / 2, panelAbsY + (-font->size + heigth) / 2, font,
                   CENTER);
    }

    uint16_t valueBarWidth = width - nameWidth - 2 * 30;
    uint16_t valueBarHeigth = heigth - 2 * 12;

    relX = nameWidth + 30;
    relY = 12;

    // valueBar
    drawRectangleChampfered(cGreyLight, relX + panelAbsX, relY + panelAbsY, valueBarWidth, valueBarHeigth, 1);

    valueBarWidth = (float)valueBarWidth * pSource->valueMapped / (float)(pSource->max - pSource->min);

    drawRectangleChampfered(cWhite, relX + panelAbsX, relY + panelAbsY, valueBarWidth, valueBarHeigth, 1);

    // Draw Value
}

//////PANELS/////
// PANEL Path Type

void GUIPanelPath::init(std::vector<Layer *> *layers, location *path, uint16_t width, uint16_t height, uint16_t x,
                        uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    pLocation = path;
    this->layers = layers;
}

void GUIPanelPath::Draw() {
    drawRectangleFill(cClear, panelAbsX, panelAbsY, panelWidth, panelHeight);
    uint16_t relX = 0;
    uint16_t relY = 0;

    uint16_t spacer = 20;

    //// Draw Layer Field////

    uint8_t layerID = (*layers)[pLocation->layer]->id;     // get ID
    std::string text = "LAYER " + std::to_string(layerID); // Layer name

    relX +=
        drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1, CENTER);
    relX += 1;

    //// Draw Module Field////
    if (pLocation->type >= FOCUSMODULE) {
        if (!(*layers)[pLocation->layer]->getModules().size()) { //  empty
            return;
        }
        std::string text = (*layers)[pLocation->layer]->getModules()[pLocation->modul]->getName(); // Modul name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }

    //// Draw Focus Field////

    if (pLocation->type == FOCUSOUTPUT) {
        if (!(*layers)[pLocation->layer]->getModules()[pLocation->modul]->getOutputs().size()) { //  empty
            return;
        }
        std::string text = (*layers)[pLocation->layer]
                               ->getModules()[pLocation->modul]
                               ->getOutputs()[pLocation->id]
                               ->getName(); // Output name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
    else if (pLocation->type == FOCUSINPUT) {
        if (!(*layers)[pLocation->layer]->getModules()[pLocation->modul]->getInputs().size()) { //  empty
            return;
        }

        std::string text = (*layers)[pLocation->layer]
                               ->getModules()[pLocation->modul]
                               ->getInputs()[pLocation->id]
                               ->getName(); // Input name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
}

// PANEL Path Type

void GUIPanelFocus::init(std::vector<Layer *> *layers, location *path, uint16_t width, uint16_t height, uint16_t x,
                         uint16_t y) {

    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    pLocation = path;
    this->layers = layers;

    // elements Sizes
    uint16_t elementWidth = width;
    uint16_t elementSpace = 10;
    uint16_t elementHeight = (height - (elementsNb - 1) * elementSpace) / elementsNb;

    // init Elements
    for (int i = 0; i < elementsNb; i++) {
        elementsModule.push_back(Focus_PanelDataElement(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i,
                                                        elementWidth, elementHeight));
    }

    registerPanelSettings();
}
void GUIPanelFocus::registerPanelSettings() {

    // register Scroll
    actionHandler.registerActionEncoder1({std::bind(&GUIPanelFocus::changeScroll, this, 1), "SCROLL"},
                                         {std::bind(&GUIPanelFocus::changeScroll, this, -1), "SCROLL"},
                                         {std::bind(&GUIPanelFocus::resetScroll, this), "RESET"});

    // register Panel Seetings Left
    actionHandler.registerActionLeft({nullptr, ""}, {std::bind(Todo), "UP"},      // focus Up
                                     {std::bind(&GUI::nextLayer, &ui), "LAYER"}); // Layer Switch

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({nullptr, ""}, {std::bind(Todo), "RESET"}, {std::bind(Todo), "PATCH"});
}

void GUIPanelFocus::Draw() {

    // register Panel Seetings
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    // check Scroll position
    checkScroll();

    if (pLocation->type == FOCUSMODULE) {
        registerModuleSettings();

        for (Focus_PanelDataElement i : elementsModule) {
            i.Draw();
        }
    }
    else if (pLocation->type == FOCUSOUTPUT) {
        // registerModuleOutputs();

        for (Focus_PanelDataElement i : elementsModule) {
            i.Draw();
        }
        //  registerModuleSettings();
    }
    else if (pLocation->type == FOCUSINPUT) {
        //   registerModuleSettings();
    }
}

uint16_t GUIPanelFocus::updateEntrys() {
    entrys = 0;

    if (pLocation->type == FOCUSINPUT) {
        entrys = 1;                                                                                 // BaseValue
        entrys += (*layers)[pLocation->layer]->getModules()[pLocation->modul]->getOutputs().size(); // all Outputs
        return entrys;
    }
    else if (pLocation->type == FOCUSOUTPUT) {
        entrys = 1;                                                                                // BaseValue
        entrys += (*layers)[pLocation->layer]->getModules()[pLocation->modul]->getInputs().size(); // all Inputs
        return entrys;
    }
    else if (pLocation->type == FOCUSSETTING) {
    }
    else if (pLocation->type == FOCUSMODULE) {

        // TODO Switches
        for (Analog *a : (*layers)[pLocation->layer]->getModules()[pLocation->modul]->getPotis())
            if (a->displayVis) { // filter Display Visibility
                entrys++;
            }

        return entrys;
    }
    else if (pLocation->type == FOCUSLAYER) {
        return entrys = (*layers)[pLocation->layer]->getModules().size(); // all Modules
    }

    return 0;
}

void GUIPanelFocus::registerModuleSettings() {

    // TODO Switches

    std::vector<Analog *> *analogSetting = &(*layers)[pLocation->layer]->getModules()[pLocation->modul]->getPotis();

    unsigned int skipped = 0;
    for (unsigned int i = 0; i < elementsModule.size(); i++) {
        while (true) {
            if ((i + skipped) < analogSetting->size()) {

                if ((*analogSetting)[i + skipped + scrollOffset]->displayVis) { // element Visible

                    elementsModule[i].pSource = (*analogSetting)[i + skipped + scrollOffset];
                    break;
                }
                else {
                    skipped++;
                }
            }

            else {
                elementsModule[i].pSource = nullptr;
                break;
            }
        }

        if (scroll == i + skipped + scrollOffset) {
            elementsModule[i].selected = 1;
            actionHandler.registerActionEncoder2(
                {std::bind(&Analog::changeValue, elementsModule[i].pSource, 1000), "AMOUNT"},
                {std::bind(&Analog::changeValue, elementsModule[i].pSource, -1000), "AMOUNT"},
                {std::bind(&Analog::resetValue, elementsModule[i].pSource), "RESET"});
        }
        else {
            elementsModule[i].selected = 0;
        }
    }
}

void GUIPanelFocus::registerModuleOutputs() {

    // std::vector<Output *> *outputs = &(*layers)[pLocation->layer]->getModules()[pLocation->modul]->getOutputs();

    // unsigned int skipped = 0;
    // for (unsigned int i = 0; i < elementsModule.size(); i++) {
    //     if (i < outputs->size()) {

    //         elementsModule[i].pSource = (*outputs)[i + scrollOffset];
    //     }
    //     else {
    //         elementsModule[i].pSource = nullptr;
    //         break;
    //     }
    //     if (scroll == i + scrollOffset) {
    //         elementsModule[i].selected = 1;
    //         actionHandler.registerActionEncoder2(
    //             {std::bind(&PatchElement::changeAmount, elementsModule[i].pSource, 0.02), "AMOUNT"},
    //             {std::bind(&PatchElement::changeAmount, elementsModule[i].pSource, -0.02), "AMOUNT"},
    //             {std::bind(&PatchElement::resetAmount, elementsModule[i].pSource), "RESET"});
    //     }
    //     else {
    //         elementsModule[i].selected = 0;
    //     }
    // }
}

void GUIPanelFocus::changeScroll(int16_t change) {

    if (scroll + change != 0) {
        this->scroll = testInt(scroll + change, 0, entrys - 1);
    }
    else
        this->scroll = 0;

    if (scroll >= entrys) {
        scroll = 0;
    }

    if (scroll == entrys - 1) {
        scrollOffset = entrys - elementsNb;
    }
    else if (scroll == 0) {
        scrollOffset = 0;
    }

    else if (scroll >= (elementsNb + scrollOffset - 1)) {
        scrollOffset++;
    }
    else if (scroll < (scrollOffset + 1)) {
        scrollOffset--;
    }
}

void GUIPanelFocus::checkScroll() {
    changeScroll(0);
}

void GUIPanelFocus::resetScroll() {
    scroll = 0;
    scrollOffset = 0;
}

// Header

void GUIHeader::init(std::vector<GUIPanelBase *> *panels, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelCount = panels->size();
    panelWidth = width / panels->size();
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
        drawRectangleFill(cWhiteLight, panelWidth * p + panelAbsX, 0 + panelAbsY, 1, panelHeight);
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

void GUISIDE::init(uint16_t width, uint16_t height, uint16_t y) {
    panelWidth = width;
    panelHeight = height / panelCount;
    panelAbsY = y;

    // boxes on the left
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft_1, panelAbsX, panelAbsY + panelHeight * 1, panelWidth, panelHeight));
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft_2, panelAbsX, panelAbsY + panelHeight * 2, panelWidth, panelHeight));
    boxes.push_back(
        Side_PanelBox(&actionHandler.buttonLeft_3, panelAbsX, panelAbsY + panelHeight * 3, panelWidth, panelHeight));

    // boxes on the rigth
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight_1, LCDWIDTH - panelWidth, panelAbsY + panelHeight * 1,
                                  panelWidth, panelHeight));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight_2, LCDWIDTH - panelWidth, panelAbsY + panelHeight * 2,
                                  panelWidth, panelHeight));
    boxes.push_back(Side_PanelBox(&actionHandler.buttonRight_3, LCDWIDTH - panelWidth, panelAbsY + panelHeight * 3,
                                  panelWidth, panelHeight));
}
void GUISIDE::Draw() {
    // draw boxes

    for (Side_PanelBox i : boxes) {
        i.Draw();
    }
}

void GUI::Init(std::vector<Layer *> &layers) { // add settings pointer
    // init Display
    GFX_Init();

    // pass Pointer
    this->layers = layers;

    // add Panels to vector
    panels.push_back(&guiPanel_1);
    panels.push_back(&guiPanel_2);
    panels.push_back(&guiPanel_3);
    panels.push_back(&guiPanel_4);

    // init Header
    guiHeader.init(&panels, LCDWIDTH, HEADERHEIGHT);

    // init Footer
    guiFooter.init(LCDWIDTH, FOOTERHEIGHT);

    // init side
    guiSide.init(BOARDERWIDTH, CENTERHEIGHT);

    // init Footer
    guiFooter.init(LCDWIDTH, FOOTERHEIGHT);

    // init Path
    guiPath.init(&layers, &focus, CENTERWIDTH, FOCUSHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER);

    // register Header action
    actionHandler.registerActionHeader(
        {std::bind(&GUI::setPanel1Active, this), "LIVEMODE"}, {std::bind(&GUI::setPanel2Active, this), "PATCH"},
        {std::bind(&GUI::setPanel3Active, this), "PRESET"}, {std::bind(&GUI::setPanel4Active, this), "CONFIG"});

    guiPanelFocus.init(&layers, &focus, CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH,
                       HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);

    setPanelFocusActive();

    // Set Focus for test
    setFocus({0, 0, 0xff, FOCUSMODULE});
}

void GUI::Clear() {
    // clear

    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);
}

void GUI::Draw() {
    setRenderState(RENDER_PROGRESS);

    // clear

    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);

    // Draw Header
    guiHeader.Draw();

    // Draw Path
    guiPath.Draw();

    // Draw Footer
    guiFooter.Draw();

    // Draw Side
    guiSide.Draw();

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
void GUI::setPanel1Active() {
    guiPanel_1.active = 1;
    guiPanel_2.active = 0;
    guiPanel_3.active = 0;
    guiPanel_4.active = 0;

    setActivePanel(0);
}
void GUI::setPanel2Active() {
    guiPanel_1.active = 0;
    guiPanel_2.active = 1;
    guiPanel_3.active = 0;
    guiPanel_4.active = 0;

    setActivePanel(1);
}
void GUI::setPanel3Active() {
    guiPanel_1.active = 0;
    guiPanel_2.active = 0;
    guiPanel_3.active = 1;
    guiPanel_4.active = 0;

    setActivePanel(2);
}

void GUI::setPanel4Active() {
    guiPanel_1.active = 0;
    guiPanel_2.active = 0;
    guiPanel_3.active = 0;
    guiPanel_4.active = 1;

    setActivePanel(3);
}

void GUI::setPanelFocusActive() {
    guiPanel_1.active = 0;
    guiPanel_2.active = 0;
    guiPanel_3.active = 0;
    guiPanel_4.active = 0;

    activePanel = &guiPanelFocus;
}

void GUI::setActivePanel(uint8_t id) {
    activePanel = panels[id];
}

#endif // ifdef POLYCONTROL
