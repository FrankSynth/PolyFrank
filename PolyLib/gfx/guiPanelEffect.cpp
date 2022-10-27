#ifdef POLYCONTROL

#include "guiPanelEffect.hpp"

void GUIPanelEffect::registerElements() {

    uint32_t entryIndex = 0;

    for (uint32_t elementIndex = 0; elementIndex < LIVEPANELENTRYS; elementIndex++) {

        for (uint32_t x = 0; x < EntrysPerElement; x++) { // start new Element

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

    overviewPanelElements[0].addEntry(&allLayers[cachedFocus.layer]->oscA.aSamplecrusher);
    overviewPanelElements[0].addEntry(&allLayers[cachedFocus.layer]->oscA.aBitcrusher);
    overviewPanelElements[0].addEntry(&allLayers[cachedFocus.layer]->waveshaperA.aDryWet, true);
    overviewPanelElements[0].addEntry(&allLayers[cachedFocus.layer]->phaseshaperA.aDryWet, true);

    overviewPanelElements[1].addEntry(&allLayers[cachedFocus.layer]->oscB.aSamplecrusher);
    overviewPanelElements[1].addEntry(&allLayers[cachedFocus.layer]->oscB.aBitcrusher);
    overviewPanelElements[1].addEntry(&allLayers[cachedFocus.layer]->waveshaperB.aDryWet, true);
    overviewPanelElements[1].addEntry(&allLayers[cachedFocus.layer]->phaseshaperB.aDryWet, true);

    overviewPanelElements[2].addEntry(&allLayers[cachedFocus.layer]->noise.aSamplecrusher);
    overviewPanelElements[2].addEntry(nullptr);
    overviewPanelElements[2].addEntry(nullptr);
    overviewPanelElements[2].addEntry(nullptr);

    overviewPanelElements[0].addEffectAmt(&allLayers[cachedFocus.layer]->oscA.aEffect);
    overviewPanelElements[1].addEffectAmt(&allLayers[cachedFocus.layer]->oscB.aEffect);
    overviewPanelElements[2].addEffectAmt(&allLayers[cachedFocus.layer]->noise.aSamplecrusher);

    scroll.entrys = 3;
    scroll.checkScroll();

    overviewPanelElements[scroll.relPosition].select = 1;
}

void GUIPanelEffect::selectModule(moduleSelect module) {
    newModuleType = module;
    updateModule = true;
}

void GUIPanelEffect::selectEffect(effectSelect effect) {
    newEffectType = effect;
    updateEffect = true;
}

void GUIPanelEffect::updateModuleSelection() {

    if (newModuleType == moduleType) {
        overview = true;
        moduleSelected[moduleType] = 0;
        moduleType = NOMODULE;
        return;
    }
    overview = false;
    if (newModuleType == NOISE) {
        selectEffect(CRUSH);
    }
    moduleSelected[moduleType] = 0;
    moduleType = newModuleType;
    moduleSelected[moduleType] = 1;

    updateEffectSelection();
}

void GUIPanelEffect::updateEffectSelection() {
    effectSelected[effectType] = 0;
    effectType = newEffectType;
    effectSelected[effectType] = 1;
}

void GUIPanelEffect::Draw() {

    if (updateModule) {
        updateModuleSelection();
        updateModule = false;
    }
    if (updateEffect) {
        updateEffectSelection();
        updateEffect = false;
    }

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
                module = &(allLayers[cachedFocus.layer]->waveshaperA);
                drawWaveShaperPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscA, (Waveshaper *)module);
            }
            else if (effectType == PHASE) {
                module = &(allLayers[cachedFocus.layer]->phaseshaperA);
                drawPhaseShaperPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscA, (Phaseshaper *)module);
            }
            else if (effectType == CRUSH) {
                module = &(allLayers[cachedFocus.layer]->oscA);
                drawCrushPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscA);
            }
        }
        else if (moduleType == OSCB) {
            if (effectType == WAVE) {
                module = &(allLayers[cachedFocus.layer]->waveshaperB);
                drawWaveShaperPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscB, (Waveshaper *)module);
            }
            else if (effectType == PHASE) {
                module = &(allLayers[cachedFocus.layer]->phaseshaperB);
                drawPhaseShaperPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscB, (Phaseshaper *)module);
            }
            else if (effectType == CRUSH) {
                module = &(allLayers[cachedFocus.layer]->oscB);
                drawCrushPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscB);
            }
        }

        else if (moduleType == NOISE) {
            if (effectType == CRUSH) {
                module = &(allLayers[cachedFocus.layer]->noise);
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
    // drawGrid(waveBuffer, c4444gridcolor);
    drawWave(waveBuffer, renderedWave, 100, 2, c4444wavecolorTrans);
    drawWaveshaper(waveBuffer, module);
    drawFrame(waveBuffer, c4444framecolor);
}

void GUIPanelEffect::drawPhaseShaperPanel(int8_t *renderedWave, Phaseshaper *module) {
    // drawGrid(waveBuffer, c4444gridcolor);
    drawWave(waveBuffer, renderedWave, 100, 1, c4444wavecolorTrans);
    drawPhaseshaper(waveBuffer, module);
    drawFrame(waveBuffer, c4444framecolor);
}

void GUIPanelEffect::drawCrushPanel(int8_t *renderedWave) {
    // drawGrid(waveBuffer, c4444gridcolor);
    drawWave(waveBuffer, renderedWave, 100, 2, c4444wavecolor);
    drawFrame(waveBuffer, c4444framecolor);
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

void GUIPanelEffect::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint8_t pathVisible) {
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