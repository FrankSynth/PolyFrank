#ifdef POLYCONTROL

#include "guiPanelPreset.hpp"

void GUIPanelPreset::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id,
                          uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;
    this->pathVisible = pathVisible;

    // elements Sizes
    uint16_t elementWidth = width - SCROLLBARWIDTH - 2;
    uint16_t elementSpace = 3;
    uint16_t elementHeight = (height - (maxEntrys - 2) * elementSpace) / PRESETPANELENTRYS;
    // init Elements
    for (int i = 0; i < maxEntrys; i++) {

        PanelElementPreset[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth,
                                   elementHeight);
    }

    scrollSecondName.entrys = secondName.size();
    scrollFirstName.entrys = firstName.size();
}

void GUIPanelPreset::registerPanelSettings() {

    actionHandler.registerActionEncoder5({std::bind(&Scroller::scroll, &(this->scrollPreset), 1), "SCROLL"},
                                         {std::bind(&Scroller::scroll, &(this->scrollPreset), -1), "SCROLL"},
                                         {nullptr, "Scroll"});

    actionHandler.registerActionEncoder1({nullptr, ""}, {nullptr, ""}, {nullptr, ""});
    actionHandler.registerActionEncoder2(
        {std::bind(&Scroller::scroll, &(this->scrollFirstName), 1), firstName[scrollFirstName.position]},
        {std::bind(&Scroller::scroll, &(this->scrollFirstName), -1), ""}, {nullptr, "SCROLL"});
    actionHandler.registerActionEncoder3(
        {std::bind(&Scroller::scroll, &(this->scrollSecondName), 1), secondName[scrollSecondName.position]},
        {std::bind(&Scroller::scroll, &(this->scrollSecondName), -1), ""}, {nullptr, "SCROLL"});
    actionHandler.registerActionEncoder4({nullptr, ""}, {nullptr, ""}, {nullptr, ""});

    // register Panel Seetings Left
    if (globalSettings.multiLayer.value == 1) {

        actionHandler.registerActionLeft({nullptr, ""}, {nullptr, ""}, {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeft({nullptr, ""}, {nullptr, ""}, {nullptr, ""});
    }
    // register Panel Seetings Rigth
    actionHandler.registerActionRight(
        {std::bind(&Layer::loadLayerFromPreset, (allLayers[currentFocus.layer]), scrollPreset.position), "LOAD"},
        {std::bind(&Layer::saveLayerToPreset, (allLayers[currentFocus.layer]), scrollPreset.position,
                   firstName[scrollFirstName.position], secondName[scrollSecondName.position]),
         "SAVE"},
        {std::bind(&removePreset, scrollPreset.position), "CLEAR"});

    // clear Encoder 4
    actionHandler.registerActionEncoder4({nullptr, ""}, {nullptr, ""}, {nullptr, ""}); // clear action
}

void GUIPanelPreset::updateEntrys() {
    entrys = presets.size();
    scrollPreset.entrys = entrys;

    // check Scroll position
    scrollPreset.checkScroll();
}

void GUIPanelPreset::registerElements() {
    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;

    dataIndex = scrollPreset.offset;

    // register Preset Elements
    while (true) {
        if (elementIndex >= maxEntrys) {
            break;
        }

        if (dataIndex < entrys) {
            PanelElementPreset[elementIndex].addEntry(&presets[dataIndex]);

            dataIndex++;
        }
        else {
            break;
        }
        elementIndex++;
    }
    PanelElementPreset[scrollPreset.relPosition].select = 1;
}

void GUIPanelPreset::Draw() {
    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    registerElements();

    for (int i = 0; i < maxEntrys; i++) {
        PanelElementPreset[i].Draw();
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scrollPreset.offset,
                  entrys, PRESETPANELENTRYS);
}

#endif