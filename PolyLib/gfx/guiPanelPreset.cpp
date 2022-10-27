#ifdef POLYCONTROL

#include "guiPanelPreset.hpp"

void GUIPanelPreset::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y, std::string name, uint8_t id,
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
    uint16_t elementSpace = 2;
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
        0, {std::bind(&GUIPanelPreset::saveLayerToPreset, this, freePreset, firstName[scrollFirstName.position],
                      secondName[scrollSecondName.position], thirdName[scrollThirdName.position]),
            "NEW"});

    if (presetsSorted.size()) {
        actionHandler.registerActionRight(
            0,
            {std::bind(&GUIPanelPreset::loadPresetToLayer, this, presetsSorted[scrollPreset.position], LAYER_AB),
             "LOAD AB"},
            1);

        if (cachedFocus.layer == 0) {
            actionHandler.registerActionRight(
                1,
                {std::bind(&GUIPanelPreset::loadPresetToLayer, this, presetsSorted[scrollPreset.position], LAYER_A),
                 "LOAD A"},
                1);
        }
        else if (cachedFocus.layer == 1) {
            actionHandler.registerActionRight(
                1,
                {std::bind(&GUIPanelPreset::loadPresetToLayer, this, presetsSorted[scrollPreset.position], LAYER_B),
                 "LOAD B"},
                1);
        }

        actionHandler.registerActionRight(2, {std::bind(&removePreset, presetsSorted[scrollPreset.position]), "REMOVE"},
                                          1);

        actionHandler.registerActionLeft(
            1,
            {std::bind(&GUIPanelPreset::saveLayerToPreset, this, presetsSorted[scrollPreset.position],
                       firstName[scrollFirstName.position], secondName[scrollSecondName.position],
                       thirdName[scrollThirdName.position]),
             "SAVE"},
            1);
    }
    else {
        actionHandler.registerActionRight(0);
        actionHandler.registerActionRight(1);
        actionHandler.registerActionRight(2);
        actionHandler.registerActionLeft(1);
    }

    actionHandler.registerActionLeft(2, {std::bind(resetSystem), "RST ALL"}, true);
}

void GUIPanelPreset::updateEntrys() {

    entrys = presetsSorted.size();
    scrollPreset.entrys = entrys;

    // check Scroll position
    scrollPreset.checkScroll();
}

void GUIPanelPreset::saveLayerToPreset(presetStruct *preset, std::string firstName, std::string secondName,
                                       std::string thirdName) {

    allLayers[0]->collectLayerConfiguration((int32_t *)blockBuffer, false);
    allLayers[1]->collectLayerConfiguration((int32_t *)(blockBuffer + PRESET_SIZE), false);
    liveData.collectLiveConfiguration((int32_t *)(blockBuffer + PRESET_SIZE + PRESET_SIZE));

    writePresetBlock(preset, firstName + " " + secondName + " " + thirdName);
}

void GUIPanelPreset::loadPresetToLayer(presetStruct *preset, LayerSelect layer) {

    readPreset(preset);

    if (preset->usageState != PRESET_USED) {
        return;
    }

    if (layer == LAYER_AB || layer == LAYER_A)
        allLayers[0]->writeLayerConfiguration((int32_t *)blockBuffer, false);

    if (layer == LAYER_AB || layer == LAYER_B)
        allLayers[1]->writeLayerConfiguration((int32_t *)(blockBuffer + PRESET_SIZE), false);

    liveData.writeLiveConfiguration((int32_t *)(blockBuffer + PRESET_SIZE + PRESET_SIZE), layer);
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
            PanelElementPreset[elementIndex].addEntry(presetsSorted[dataIndex]);

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