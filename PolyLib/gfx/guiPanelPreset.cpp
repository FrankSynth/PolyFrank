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
    scrollThirdName.entrys = thirdName.size();
}

void GUIPanelPreset::registerPanelSettings() {

    actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, &(this->scrollPreset), 1), "SCROLL"},
                                        {std::bind(&Scroller::scroll, &(this->scrollPreset), -1), "SCROLL"},
                                        {nullptr, "Scroll"});

    actionHandler.registerActionEncoder(3, {nullptr, ""}, {nullptr, ""}, {nullptr, ""});
    actionHandler.registerActionEncoder(
        0, {std::bind(&Scroller::scroll, &(this->scrollFirstName), 1), firstName[scrollFirstName.position]},
        {std::bind(&Scroller::scroll, &(this->scrollFirstName), -1), ""}, {nullptr, "SCROLL"});
    actionHandler.registerActionEncoder(
        1, {std::bind(&Scroller::scroll, &(this->scrollSecondName), 1), secondName[scrollSecondName.position]},
        {std::bind(&Scroller::scroll, &(this->scrollSecondName), -1), ""}, {nullptr, "SCROLL"});
    actionHandler.registerActionEncoder(
        2, {std::bind(&Scroller::scroll, &(this->scrollThirdName), 1), thirdName[scrollThirdName.position]},
        {std::bind(&Scroller::scroll, &(this->scrollThirdName), -1), ""}, {nullptr, "SCROLL"});

    actionHandler.registerActionLeft(
        0, {std::bind(&Layer::saveLayerToPreset, allLayers[currentFocus.layer], freePresetID(),
                      firstName[scrollFirstName.position], secondName[scrollSecondName.position],
                      thirdName[scrollThirdName.position]),
            "SAVE"});

    actionHandler.registerActionRight(1);

    if (presetEntrys.size()) {
        actionHandler.registerActionRight(
            0, {std::bind(&Layer::loadLayerFromPreset, allLayers[currentFocus.layer], scrollPreset.position), "LOAD"});
        actionHandler.registerActionRight(
            2, {std::bind(&removePreset, presetEntrys[scrollPreset.position]->ID), "REMOVE"}, 0);

        actionHandler.registerActionLeft(
            1,
            {std::bind(&Layer::saveLayerToPreset, allLayers[currentFocus.layer],
                       presetEntrys[scrollPreset.position]->ID, firstName[scrollFirstName.position],
                       secondName[scrollSecondName.position], thirdName[scrollThirdName.position]),
             "OVERRIDE"},
            0);
    }
    else {

        actionHandler.registerActionRight(0);
        actionHandler.registerActionRight(2);
        actionHandler.registerActionLeft(1);
    }

    if (liveData.voiceHandler.livemodeMergeLayer.value == 0 && globalSettings.multiLayer.value == 1) {
        actionHandler.registerActionLeft(2, {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeft(2);
    }
}

void GUIPanelPreset::updateEntrys() {
    presetEntrys.clear();

    for (uint16_t i = 0; i < presets.size(); i++) {
        if (presets[i].usageState == PRESET_USED) {
            presetEntrys.push_back(&presets[i]);
        }
    }

    for (presetStruct *p : presetEntrys) {

        println("  name  ", p->name, "  id  ", p->ID, "  state  ", p->usageState);
    }

    entrys = presetEntrys.size();
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
            PanelElementPreset[elementIndex].addEntry(presetEntrys[dataIndex]);

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

    // update number ob entrys
    updateEntrys();
    // register Panel Seetings.settings.
    registerPanelSettings();

    registerElements();

    for (int i = 0; i < maxEntrys; i++) {
        PanelElementPreset[i].Draw();
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scrollPreset.offset,
                  entrys, PRESETPANELENTRYS);
}

#endif