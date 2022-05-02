#ifdef POLYCONTROL

#include "guiPanelEffect.hpp"

void GUIPanelEffect::registerElements() {

    uint16_t entryIndex = 0;

    for (uint16_t elementIndex = 0; elementIndex < LIVEPANELENTRYS; elementIndex++) {

        for (int x = 0; x < EntrysPerElement; x++) { // start new Element

            if (entryIndex < entrys.size()) { // no more entry available
                effectPanelElements[elementIndex].addEntry(entrys[entryIndex]);
            }
            else {
                effectPanelElements[elementIndex].addEntry(); // empty entry
            }

            entryIndex++;
        }
    }

    effectPanelElements[scroll.relPosition].select = 1;
}

void GUIPanelEffect::updateEntrys(BaseModule *module) {
    entrys.clear();

    if (effectType == CRUSH) {
        if (moduleType == OSCA) {

            entrys.push_back(&((OSC_A *)module)->aSamplecrusher);
            entrys.push_back(&((OSC_A *)module)->aBitcrusher);
        }
        if (moduleType == OSCB) {

            entrys.push_back(&((OSC_B *)module)->aSamplecrusher);
            entrys.push_back(&((OSC_B *)module)->aBitcrusher);
        }
        if (moduleType == NOISE) {
            entrys.push_back(&((Noise *)module)->aSamplecrusher);
        }
    }
    else {
        if (effectType == PHASE) {
            entrys.push_back(&((Phaseshaper *)module)->aPoint2X);
            entrys.push_back(&((Phaseshaper *)module)->aPoint2Y);
            entrys.push_back(&((Phaseshaper *)module)->aPoint3X);
            entrys.push_back(&((Phaseshaper *)module)->aPoint3Y);
            entrys.push_back(&((Phaseshaper *)module)->aPoint1Y);
            entrys.push_back(&((Phaseshaper *)module)->aPoint4Y);
            entrys.push_back(&((Phaseshaper *)module)->aDryWet);
        }
        else {
            uint8_t index = 0;
            for (Analog *a : module->getPotis()) {
                entrys.push_back(a);

                index++;
            }
        }
    }
    scroll.entrys = ceil(((float)entrys.size() / EntrysPerElement));
    scroll.checkScroll();
}

void GUIPanelEffect::registerOverviewEntrys() {
    entrys.clear();

    overviewPanelElements[0].addEntry(&allLayers[currentFocus.layer]->oscA.aSamplecrusher);
    overviewPanelElements[0].addEntry(&allLayers[currentFocus.layer]->oscA.aBitcrusher);
    overviewPanelElements[0].addEntry(&allLayers[currentFocus.layer]->waveshaperA.aDryWet, true);
    overviewPanelElements[0].addEntry(&allLayers[currentFocus.layer]->phaseshaperA.aDryWet, true);

    overviewPanelElements[1].addEntry(&allLayers[currentFocus.layer]->oscB.aSamplecrusher);
    overviewPanelElements[1].addEntry(&allLayers[currentFocus.layer]->oscB.aBitcrusher);
    overviewPanelElements[1].addEntry(&allLayers[currentFocus.layer]->waveshaperB.aDryWet, true);
    overviewPanelElements[1].addEntry(&allLayers[currentFocus.layer]->phaseshaperB.aDryWet, true);

    overviewPanelElements[2].addEntry(&allLayers[currentFocus.layer]->noise.aSamplecrusher);
    overviewPanelElements[2].addEntry(nullptr);
    overviewPanelElements[2].addEntry(nullptr);
    overviewPanelElements[2].addEntry(nullptr);

    scroll.entrys = 3;
    scroll.checkScroll();

    overviewPanelElements[scroll.relPosition].select = 1;
}

void GUIPanelEffect::selectModule(moduleSelect module) {
    if (module == moduleType) {
        overview = true;
        moduleSelected[moduleType] = 0;
        moduleType = NOMODULE;
        return;
    }
    overview = false;
    if (module == NOISE) {
        selectEffect(CRUSH);
    }
    moduleSelected[moduleType] = 0;
    moduleType = module;
    moduleSelected[moduleType] = 1;
}

void GUIPanelEffect::selectEffect(effectSelect effect) {
    effectSelected[effectType] = 0;
    effectType = effect;
    effectSelected[effectType] = 1;
}

void GUIPanelEffect::Draw() {
    registerPanelSettings();

    if (overview) {
        registerOverviewEntrys();

        for (int i = 0; i < LIVEPANELENTRYS; i++) {
            overviewPanelElements[i].Draw();
        }
    }
    else {
        // empty waveBuffer
        uint32_t data = 0x00000000;
        fastMemset(&data, (uint32_t *)*(waveBuffer.buffer), (waveBuffer.width * waveBuffer.height) / 2);
        BaseModule *module = nullptr;

        if (moduleType == OSCA) {
            if (effectType == WAVE) {
                module = &(allLayers[currentFocus.layer]->waveshaperA);
                drawWaveShaperPanel(allLayers[currentFocus.layer]->renderedAudioWavesOscA, (Waveshaper *)module);
            }
            else if (effectType == PHASE) {
                module = &(allLayers[currentFocus.layer]->phaseshaperA);
                drawPhaseShaperPanel(allLayers[currentFocus.layer]->renderedAudioWavesOscA, (Phaseshaper *)module);
            }
            else if (effectType == CRUSH) {
                module = &(allLayers[currentFocus.layer]->oscA);
                drawCrushPanel(allLayers[currentFocus.layer]->renderedAudioWavesOscA);
            }
        }
        else if (moduleType == OSCB) {
            if (effectType == WAVE) {
                module = &(allLayers[currentFocus.layer]->waveshaperB);
                drawWaveShaperPanel(allLayers[currentFocus.layer]->renderedAudioWavesOscB, (Waveshaper *)module);
            }
            else if (effectType == PHASE) {
                module = &(allLayers[currentFocus.layer]->phaseshaperB);
                drawPhaseShaperPanel(allLayers[currentFocus.layer]->renderedAudioWavesOscB, (Phaseshaper *)module);
            }
            else if (effectType == CRUSH) {
                module = &(allLayers[currentFocus.layer]->oscB);
                drawCrushPanel(allLayers[currentFocus.layer]->renderedAudioWavesOscB);
            }
        }

        else if (moduleType == NOISE) {
            if (effectType == CRUSH) {
                module = &(allLayers[currentFocus.layer]->noise);
                int8_t wave[100];
                calculateNoiseWave((Noise *)module, wave, 100);
                drawCrushPanel(wave);
            }
        }

        if (module == nullptr)
            return;

        updateEntrys(module);
        registerElements();

        SCB_CleanDCache_by_Addr((uint32_t *)waveBuffer.buffer, waveBuffer.height * waveBuffer.width * 2);

        copyWaveBuffer(waveBuffer, panelAbsX, panelAbsY);

        for (int i = 0; i < LIVEPANELENTRYS; i++) {
            effectPanelElements[i].Draw();
        }
    }
}

void GUIPanelEffect::drawWaveShaperPanel(int8_t *renderedWave, Waveshaper *module) {
    drawGrid(c4444gridcolor);
    drawWave(renderedWave, 100, 2, c4444wavecolorTrans);
    drawWaveshaper(waveBuffer, module);
    drawFrame(c4444framecolor);
}

void GUIPanelEffect::drawPhaseShaperPanel(int8_t *renderedWave, Phaseshaper *module) {
    drawGrid(c4444gridcolor);
    drawWave(renderedWave, 100, 1, c4444wavecolorTrans);
    drawPhaseshaper(waveBuffer, module);
    drawFrame(c4444framecolor);
}

void GUIPanelEffect::drawCrushPanel(int8_t *renderedWave) {
    drawGrid(c4444gridcolor);
    drawWave(renderedWave, 100, 2, c4444wavecolor);
    drawFrame(c4444framecolor);
}

void GUIPanelEffect::registerPanelSettings() {

    actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, &(this->scroll), 1), "SCROLL"},
                                        {std::bind(&Scroller::scroll, &(this->scroll), -1), "SCROLL"}, {nullptr, ""});

    actionHandler.registerActionLeftData(0, {std::bind(&GUIPanelEffect::selectModule, this, OSCA), "OSC A"},
                                         &(moduleSelected[0]));

    actionHandler.registerActionLeftData(1, {std::bind(&GUIPanelEffect::selectModule, this, OSCB), "OSC B"},
                                         &(moduleSelected[1]));
    actionHandler.registerActionLeftData(2, {std::bind(&GUIPanelEffect::selectModule, this, NOISE), "NOISE"},
                                         &(moduleSelected[2]));
    if (overview == true) {
        actionHandler.registerActionRightData(0);
        actionHandler.registerActionRightData(1);
        actionHandler.registerActionRightData(2);
    }
    else if (moduleType == NOISE) {

        actionHandler.registerActionRightData(0);
        actionHandler.registerActionRightData(1);
        actionHandler.registerActionRightData(2, {std::bind(&GUIPanelEffect::selectEffect, this, CRUSH), "CRUSH"},
                                              &(effectSelected[2]));
    }
    else {

        actionHandler.registerActionRightData(0, {std::bind(&GUIPanelEffect::selectEffect, this, WAVE), "WAVE"},
                                              &(effectSelected[0]));

        actionHandler.registerActionRightData(1, {std::bind(&GUIPanelEffect::selectEffect, this, PHASE), "PHASE"},
                                              &(effectSelected[1]));
        actionHandler.registerActionRightData(2, {std::bind(&GUIPanelEffect::selectEffect, this, CRUSH), "CRUSH"},
                                              &(effectSelected[2]));
    }
}

void GUIPanelEffect::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->pathVisible = pathVisible;

    // elements Sizes
    uint16_t elementWidth = width;
    uint16_t elementSpace = 2;
    uint16_t elementHeight = (height - WAVEFORMHEIGHT - (LIVEPANELENTRYS - 2) * elementSpace) / LIVEPANELENTRYS;

    // init Elements
    for (int i = 0; i < LIVEPANELENTRYS; i++) {
        effectPanelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i + WAVEFORMHEIGHT,
                                    elementWidth, elementHeight);
    }

    // elements Sizes
    elementHeight = 60;
    // init Elements
    for (int i = 0; i < 3; i++) {
        overviewPanelElements[i].init(panelAbsX, panelAbsY + (elementHeight + 40) * i + 40, elementWidth,
                                      elementHeight);
    }

    overviewPanelElements[0].name = "OSC A";
    overviewPanelElements[1].name = "OSC B";
    overviewPanelElements[2].name = "NOISE";

    name = "EFFECT";

    selectModule(OSCA);
    selectEffect(WAVE);
}

#endif