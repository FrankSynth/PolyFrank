#ifdef POLYCONTROL

#include "guiPanelEffect.hpp"

#include "humanInterface/hid.hpp"
extern PanelTouch touch;

void GUIPanelEffect::registerElements() {
    for (uint32_t elementIndex = 0; elementIndex < 2; elementIndex++) {
        if (!touch.outputPatchActive() && !patch)
            effectPanelElements[elementIndex].select = true;

        effectPanelElements[elementIndex].addEntry(entrys[elementIndex], elementIndex * 2);
    }
}

void GUIPanelEffect::updateEntrys() {

    if (effectType == PHASE) {
        switch (scrollDotsPhase.position) {
            case 0:
                entrys[0] = nullptr;
                entrys[1] = &((Phaseshaper *)module)->aPoint1Y;
                break;
            case 1:
                entrys[0] = &((Phaseshaper *)module)->aPoint2X;
                entrys[1] = &((Phaseshaper *)module)->aPoint2Y;
                break;
            case 2:
                entrys[0] = &((Phaseshaper *)module)->aPoint3X;
                entrys[1] = &((Phaseshaper *)module)->aPoint3Y;
                break;
            case 3:
                entrys[0] = nullptr;
                entrys[1] = &((Phaseshaper *)module)->aPoint4Y;
                break;

            default:
                entrys[0] = nullptr;
                entrys[1] = nullptr;
                break;
        }

        scrollDotsPhase.entrys = 4;
    }
    else if (effectType == WAVE) {
        switch (scrollDotsWave.position) {
            case 0:
                entrys[0] = &((Waveshaper *)module)->aPoint1X;
                entrys[1] = &((Waveshaper *)module)->aPoint1Y;
                break;
            case 1:
                entrys[0] = &((Waveshaper *)module)->aPoint2X;
                entrys[1] = &((Waveshaper *)module)->aPoint2Y;
                break;
            case 2:
                entrys[0] = &((Waveshaper *)module)->aPoint3X;
                entrys[1] = &((Waveshaper *)module)->aPoint3Y;
                break;
            case 3:
                entrys[0] = nullptr;
                entrys[1] = &((Waveshaper *)module)->aPoint4Y;

                break;

            default:
                entrys[0] = nullptr;
                entrys[1] = nullptr;
                break;
        }
        scrollDotsWave.entrys = 4;
    }
    for (uint32_t entryID = 0; entryID < 2; entryID++) { // for poth rows
        activeEntryPatch[entryID] = nullptr;

        if (entrys[entryID] != nullptr) {
            if (entrys[entryID]->input->patchesInOut.size() != 0) {
                scrollPatches[entryID].entrys = entrys[entryID]->input->patchesInOut.size();
                if (scrollToPatchMax[entryID]) {
                    scrollToPatchMax[entryID] = false;
                    scrollPatches[entryID].scroll(100); // scroll to end
                }
                scrollPatches[entryID].checkScroll();

                uint32_t entryCounter = 0;

                for (uint32_t patchID = scrollPatches[entryID].offset;
                     patchID < entrys[entryID]->input->patchesInOut.size(); // for each Patch
                     patchID++) {
                    if (entryCounter < EFFECTPATCHELEMENTS) { // as long as we have free slots
                        effectPatchElements[entryID][entryCounter].addEntry(
                            entrys[entryID]->input->patchesInOut[patchID], 1 + entryID * 2);
                        entryCounter++;
                    }
                    else {
                        break;
                    }
                }
                if (!touch.outputPatchActive()) {
                    if (patch) {
                        activeEntryPatch[entryID] =
                            effectPatchElements[entryID][scrollPatches[entryID].relPosition].entry;
                        effectPatchElements[entryID][scrollPatches[entryID].relPosition].select = true;
                    }
                }
            }
            else {
                actionHandler.registerActionEncoder(1 + entryID * 2); // clear amount encoder
            }
        }
        else {
            actionHandler.registerActionEncoder(1 + entryID * 2); // clear amount encoder
        }
    }
}

void GUIPanelEffect::togglePatchView() {
    patch = !patch;
}

void GUIPanelEffect::registerOverviewEntrys() {
    for (uint32_t layer = 0; layer < 2; layer++) {

        overviewPanelElements[layer][0].name = "OSC A";
        overviewPanelElements[layer][1].name = "OSC B";
        overviewPanelElements[layer][2].name = "NOISE";

        overviewPanelElements[layer][0].addEntry(&allLayers[layer]->oscA.aSamplecrusher);
        overviewPanelElements[layer][0].addEntry(&allLayers[layer]->oscA.aBitcrusher);
        overviewPanelElements[layer][0].addEntry(&allLayers[layer]->waveshaperA.aDryWet, true);
        overviewPanelElements[layer][0].addEntry(&allLayers[layer]->phaseshaperA.aDryWet, true);

        overviewPanelElements[layer][1].addEntry(&allLayers[layer]->oscB.aSamplecrusher);
        overviewPanelElements[layer][1].addEntry(&allLayers[layer]->oscB.aBitcrusher);
        overviewPanelElements[layer][1].addEntry(&allLayers[layer]->waveshaperB.aDryWet, true);
        overviewPanelElements[layer][1].addEntry(&allLayers[layer]->phaseshaperB.aDryWet, true);

        overviewPanelElements[layer][2].addEntry(&allLayers[layer]->noise.aSamplecrusher);
        overviewPanelElements[layer][2].addEntry(nullptr);
        overviewPanelElements[layer][2].addEntry(nullptr);
        overviewPanelElements[layer][2].addEntry(nullptr);

        overviewPanelElements[layer][0].addEffectAmt(&allLayers[layer]->oscA.aEffect);
        overviewPanelElements[layer][1].addEffectAmt(&allLayers[layer]->oscB.aEffect);
        overviewPanelElements[layer][2].addEffectAmt(&allLayers[layer]->noise.aSamplecrusher);

        scrollModule.entrys = 3;
        scrollModule.checkScroll();
    }
}

void GUIPanelEffect::selectModule(moduleSelect module) {
    newModuleType = module;
    updateModule = true;
    overview = false;
}

void GUIPanelEffect::selectEffect(effectSelect effect) {
    newEffectType = effect;
    updateEffect = true;
}

void GUIPanelEffect::enableOverview() {
    overview = true;
    moduleSelected[0] = false;
    moduleSelected[1] = false;
}

void GUIPanelEffect::updateModuleSelection() {

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
    // clear encoder

    registerPanelSettings();

    if (overview) {
        overviewPanelElements[cachedFocus.layer][scrollModule.relPosition].select = 1;

        for (int i = 0; i < LIVEPANELENTRYS; i++) {
            overviewPanelElements[cachedFocus.layer][i].Draw();
        }
    }
    else {
        // empty waveBuffer
        uint32_t data = 0x00000000;
        fastMemset(&data, (uint32_t *)*(waveBuffer.buffer), (waveBuffer.width * waveBuffer.height) / 2);

        // Draw Waves
        if (moduleType == OSCA) {
            if (effectType == WAVE) {
                module = &(allLayers[cachedFocus.layer]->waveshaperA);
                drawWaveShaperPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscA, (Waveshaper *)module);
            }
            else if (effectType == PHASE) {
                module = &(allLayers[cachedFocus.layer]->phaseshaperA);
                drawPhaseShaperPanel(allLayers[cachedFocus.layer]->renderedAudioWavesOscA, (Phaseshaper *)module);
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
        }
        if (module == nullptr)
            return;

        // DRY WET bar
        Analog *dryWet = nullptr;
        if (module->moduleType == MODULE_WAVESHAPER) {
            dryWet = &((Waveshaper *)module)->aDryWet;
        }
        else if (module->moduleType == MODULE_PHASE) {
            dryWet = &((Phaseshaper *)module)->aDryWet;
        }
        drawAnalogElementReduced(dryWet, panelAbsX, panelAbsY + waveBuffer.height, panelWidth,
                                 20); // DryWet

        SCB_CleanDCache_by_Addr((uint32_t *)waveBuffer.buffer, waveBuffer.height * waveBuffer.width * 2);
        copyWaveBuffer(waveBuffer, panelAbsX, panelAbsY);

        // Register Entrys

        updateEntrys();
        registerElements();

        for (int i = 0; i < 2; i++) {
            effectPanelElements[i].Draw();
        }

        for (int rows = 0; rows < 2; rows++) {
            for (int i = 0; i < EFFECTPATCHELEMENTS; i++) {
                effectPatchElements[rows][i].Draw();
            }
        }
    }
}

void GUIPanelEffect::drawWaveShaperPanel(int8_t *renderedWave, Waveshaper *module) {
    // drawGrid(waveBuffer, c4444gridcolor);
    drawWave(waveBuffer, renderedWave, 100, 2, c4444wavecolorTrans);
    drawWaveshaper(waveBuffer, module, scrollDotsWave.position + 1);
    drawFrame(waveBuffer, c4444framecolor);
}

void GUIPanelEffect::drawPhaseShaperPanel(int8_t *renderedWave, Phaseshaper *module) {
    // drawGrid(waveBuffer, c4444gridcolor);
    drawWave(waveBuffer, renderedWave, 100, 1, c4444wavecolorTrans);
    drawPhaseshaper(waveBuffer, module, scrollDotsPhase.position + 1);
    drawFrame(waveBuffer, c4444framecolor);
}

void GUIPanelEffect::drawCrushPanel(int8_t *renderedWave) {
    // drawGrid(waveBuffer, c4444gridcolor);
    drawWave(waveBuffer, renderedWave, 100, 2, c4444wavecolor);
    drawFrame(waveBuffer, c4444framecolor);
}

void GUIPanelEffect::registerPanelSettings() {

    actionHandler.registerActionLeftData(0, {std::bind(&GUIPanelEffect::enableOverview, this), "OVER"}, &(overview));

    actionHandler.registerActionLeftData(1, {std::bind(&GUIPanelEffect::selectModule, this, OSCA), "OSC A"},
                                         &(moduleSelected[0]));

    actionHandler.registerActionLeftData(2, {std::bind(&GUIPanelEffect::selectModule, this, OSCB), "OSC B"},
                                         &(moduleSelected[1]));

    if (overview == true) {
        actionHandler.registerActionRightData(0);
        actionHandler.registerActionRightData(1);
        actionHandler.registerActionRightData(2);

        actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, &(this->scrollModule), 1), "SCROLL"},
                                            {std::bind(&Scroller::scroll, &(this->scrollModule), -1), "SCROLL"},
                                            {nullptr, ""});

        actionHandler.registerActionEncoder(5);
    }

    else {

        actionHandler.registerActionRightData(0, {std::bind(&GUIPanelEffect::selectEffect, this, WAVE), "WAVE"},
                                              &(effectSelected[0]));

        actionHandler.registerActionRightData(1, {std::bind(&GUIPanelEffect::selectEffect, this, PHASE), "PHASE"},
                                              &(effectSelected[1]));

        actionHandler.registerActionRightData(2, {std::bind(&GUIPanelEffect::togglePatchView, this), "PATCHES"},
                                              &patch);
        if (effectType == WAVE) {
            actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, &(this->scrollDotsWave), 1), "SCROLL"},
                                                {std::bind(&Scroller::scroll, &(this->scrollDotsWave), -1), "SCROLL"},
                                                {nullptr, ""});

            if (moduleType == OSCA) {
                actionHandler.registerActionEncoder(
                    5,
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->waveshaperA.aDryWet), 1),
                     "SCROLL"},
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->waveshaperA.aDryWet), 0),
                     "SCROLL"},
                    {std::bind(&Analog::resetValue, &(allLayers[cachedFocus.layer]->waveshaperA.aDryWet)), ""});
            }
            if (moduleType == OSCB) {
                actionHandler.registerActionEncoder(
                    5,
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->waveshaperB.aDryWet), 1),
                     "SCROLL"},
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->waveshaperB.aDryWet), 0),
                     "SCROLL"},
                    {std::bind(&Analog::resetValue, &(allLayers[cachedFocus.layer]->waveshaperB.aDryWet)), ""});
            }
        }
        else if (effectType == PHASE) {
            actionHandler.registerActionEncoder(
                4, {std::bind(&Scroller::scroll, &(this->scrollDotsPhase), 1), "SCROLL"},
                {std::bind(&Scroller::scroll, &(this->scrollDotsPhase), -1), "SCROLL"}, {nullptr, ""});

            if (moduleType == OSCA) {
                actionHandler.registerActionEncoder(
                    5,
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->phaseshaperA.aDryWet), 1),
                     "SCROLL"},
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->phaseshaperA.aDryWet), 0),
                     "SCROLL"},
                    {std::bind(&Analog::resetValue, &(allLayers[cachedFocus.layer]->phaseshaperA.aDryWet)), ""});
            }
            if (moduleType == OSCB) {
                actionHandler.registerActionEncoder(
                    5,
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->phaseshaperB.aDryWet), 1),
                     "SCROLL"},
                    {std::bind(&Analog::changeValueWithEncoderAcceleration,
                               &(allLayers[cachedFocus.layer]->phaseshaperB.aDryWet), 0),
                     "SCROLL"},
                    {std::bind(&Analog::resetValue, &(allLayers[cachedFocus.layer]->phaseshaperB.aDryWet)), ""});
            }
        }

        if (patch) {

            if (touch.outputPatchActive()) { // we are in patching mode
                actionHandler.registerActionEncoder(0, {std::bind(&GUIPanelEffect::patchToOutput, this, 0), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 0), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 0), "PATCH"});

                actionHandler.registerActionEncoder(2, {std::bind(&GUIPanelEffect::patchToOutput, this, 1), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 1), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 1), "PATCH"});

                actionHandler.registerActionEncoder(1);
                actionHandler.registerActionEncoder(3);
            }
            else {
                if (activeEntryPatch[0] != nullptr) {
                    actionHandler.registerActionEncoder(
                        0, {std::bind(&Scroller::scroll, &(this->scrollPatches[0]), 1), "SCROLL"},
                        {std::bind(&Scroller::scroll, &(this->scrollPatches[0]), -1), "SCROLL"},
                        {std::bind(&Layer::removePatchInOutById, allLayers[cachedFocus.layer],
                                   activeEntryPatch[0]->sourceOut->idGlobal, activeEntryPatch[0]->targetIn->idGlobal),
                         "REMOVE"});
                }
                else {
                    actionHandler.registerActionEncoder(
                        0, {std::bind(&Scroller::scroll, &(this->scrollPatches[0]), 1), "SCROLL"},
                        {std::bind(&Scroller::scroll, &(this->scrollPatches[0]), -1), "SCROLL"}, {nullptr, ""});
                }
                if (activeEntryPatch[1] != nullptr) {

                    actionHandler.registerActionEncoder(
                        2, {std::bind(&Scroller::scroll, &(this->scrollPatches[1]), 1), "SCROLL"},
                        {std::bind(&Scroller::scroll, &(this->scrollPatches[1]), -1), "SCROLL"},
                        {std::bind(&Layer::removePatchInOutById, allLayers[cachedFocus.layer],
                                   activeEntryPatch[1]->sourceOut->idGlobal, activeEntryPatch[1]->targetIn->idGlobal),
                         "REMOVE"});
                }
                else {
                    actionHandler.registerActionEncoder(
                        2, {std::bind(&Scroller::scroll, &(this->scrollPatches[1]), 1), "SCROLL"},
                        {std::bind(&Scroller::scroll, &(this->scrollPatches[1]), -1), "SCROLL"}, {nullptr, ""});
                }
            }
        }
        else {

            if (touch.outputPatchActive()) { // we are in patching mode
                actionHandler.registerActionEncoder(0, {std::bind(&GUIPanelEffect::patchToOutput, this, 0), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 0), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 0), "PATCH"});

                actionHandler.registerActionEncoder(2, {std::bind(&GUIPanelEffect::patchToOutput, this, 1), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 1), "PATCH"},
                                                    {std::bind(&GUIPanelEffect::patchToOutput, this, 1), "PATCH"});
            }

            actionHandler.registerActionEncoder(1);
            actionHandler.registerActionEncoder(3);
        }
    }
}

void GUIPanelEffect::patchToOutput(uint32_t entryID) {
    if (touch.activeOutput->layerId != cachedFocus.layer) {
        nextLayer();
    }
    touch.externPatchInput(entrys[entryID]->input);

    patch = true;
    scrollToPatchMax[entryID] = true;
}

void GUIPanelEffect::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint8_t pathVisible) {

    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->pathVisible = pathVisible;

    // elements Sizes
    uint16_t elementWidth = width / 2;
    // uint16_t elementSpace = 2;
    uint16_t elementHeight = 60;

    // init Elements
    for (int i = 0; i < 2; i++) {
        effectPanelElements[i].init(panelAbsX + i * elementWidth, panelAbsY + WAVEFORMHEIGHT + DRYWETINFO, elementWidth,
                                    elementHeight);
    }
    uint16_t PatchElementHeight = (height - WAVEFORMHEIGHT - elementHeight - DRYWETINFO) / EFFECTPATCHELEMENTS;

    // init Elements
    for (int entryID = 0; entryID < 2; entryID++) {

        for (int i = 0; i < EFFECTPATCHELEMENTS; i++) {
            effectPatchElements[entryID][i].init(panelAbsX + entryID * elementWidth,
                                                 panelAbsY + WAVEFORMHEIGHT + DRYWETINFO + elementHeight +
                                                     i * PatchElementHeight + 1,
                                                 elementWidth, PatchElementHeight - 2);
        }
    }
    // elements Sizes
    elementWidth = width;

    elementHeight = 60;
    // init Elements
    for (int layer = 0; layer < 2; layer++) {
        for (int i = 0; i < 3; i++) {
            overviewPanelElements[layer][i].init(panelAbsX, panelAbsY + (elementHeight + 40) * i + 40, elementWidth,
                                                 elementHeight - 1);
        }
    }
    registerOverviewEntrys();

    name = "EFFECT";

    selectModule(OSCA);
    selectEffect(WAVE);

    scrollPatches[0].maxEntrysVisible = EFFECTPATCHELEMENTS;
    scrollPatches[1].maxEntrysVisible = EFFECTPATCHELEMENTS;
}

#endif