#ifdef POLYCONTROL
#include "gui.hpp"

GUI ui;

void GUI::Init() { // add settings pointer
                   // init Display

    waveBuffer.height = WAVEFORMHEIGHT;
    waveQuickBuffer.height = WAVEFORMQUICKHEIGHT;

    waveBuffer.buffer = &waveformBuffer;
    waveQuickBuffer.buffer = &waveformQuickBuffer;

    GFX_Init();

    guiPanelQuickView.init(LCDWIDTH, LCDHEIGHT, 0, 0);

    guiPanelFocus.init(CENTERWIDTH, CENTERHEIGHT + FOOTERHEIGHT + SPACER, BOARDERWIDTH,
                       HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);

    guiPanelLiveData.init(CENTERWIDTH, CENTERHEIGHT - VOICEHEIGHT - SPACER, BOARDERWIDTH,
                          HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "LIVE", 1, 0);
    guiPanelArp.init(CENTERWIDTH, CENTERHEIGHT - VOICEHEIGHT - SPACER, BOARDERWIDTH,
                     HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "ARP", 1, 1);

    guiPanelPatch.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "PATCH",
                       1);
    guiPanelPreset.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "PRESET",
                        2);
    guiPanelConfig.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "CONFIG",
                        3);

    guiPanelEffect.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);

    // multiLayer detected
    guiPanelVoice[0].init(0, CENTERWIDTH, VOICEHEIGHT / 2 - 1, BOARDERWIDTH,
                          HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER + CENTERHEIGHT - VOICEHEIGHT);

    guiPanelVoice[1].init(1, CENTERWIDTH, VOICEHEIGHT / 2 - 1, BOARDERWIDTH,
                          HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER + CENTERHEIGHT - VOICEHEIGHT + VOICEHEIGHT / 2);

    // init Header
    guiHeader.init(&panels, &activePanelID, LCDWIDTH - 2 * BOARDERWIDTH, HEADERHEIGHT, BOARDERWIDTH);

    // init Footer
    guiFooter.init(LCDWIDTH - 2 * BOARDERWIDTH, FOOTERHEIGHT, BOARDERWIDTH);

    // init side
    guiSide.init(BOARDERWIDTH - 2, LCDHEIGHT);

    // init Path
    guiPath.init(CENTERWIDTH, FOCUSHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER);

    // init State
    guiState.init(CENTERWIDTH, FOCUSHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER);

    // init Error
    guiError.init(LCDWIDTH, LCDHEIGHT, 0, 0);

    // init Error
    guiPanelDebug.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);

    guiPanelStart.init(LCDWIDTH, LCDHEIGHT, 0, 0);

    panels.push_back(&guiPanelLiveData);
    panels.push_back(&guiPanelArp);
    panels.push_back(&guiPanelPatch);
    panels.push_back(&guiPanelEffect);
    panels.push_back(&guiPanelConfig);
    panels.push_back(&guiPanelPreset);
    panels.push_back(&guiPanelFocus);
    panels.push_back(&guiPanelDebug);

    clear();
    checkFocusChange();
    setPanelActive(0);

    // Set Focus for test
}

void GUI::clear() {
    // clear

    drawRectangleFill(0xFF000000, 0, 0, LCDWIDTH, LCDHEIGHT);
}

void GUI::Draw() {
    setRenderState(RENDER_PROGRESS);

    // setDisplayBrightness
    __HAL_TIM_SET_COMPARE(&htim13, TIM_CHANNEL_1,
                          globalSettings.dispBrightness.getValue() * 1000); // 6553* 1-10 -> 65530

    static bool introFrame = true;
    static elapsedMillis timerIntroScreen;

    // clear
    drawRectangleFill(cBackground, 0, 0, LCDWIDTH, LCDHEIGHT);

    // Error Occurred?
    if (globalSettings.error.errorActive) {
        guiError.Draw();
    }

    else {
        checkFocusChange();

        if (panelChanged) {
            activePanel = panels[activePanelID];
            actionHandler.clear();
            activePanel->activate();

            panelChanged = 0;
        }

        // Draw Panel
        if (activePanel != nullptr) {
            activePanel->Draw();

            if (activePanel == &guiPanelFocus) { // focus Panel draw complete Path

                guiPath.Draw();
            }

            else if (activePanel == &guiPanelLiveData ||
                     activePanel == &guiPanelArp) { // liveMode Panel draw short Path and VoiceState

                guiPanelVoice[0].Draw();
                guiPanelVoice[1].Draw();

                guiPath.Draw(1);
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

        // Draw Header
        guiState.Draw();

        // Draw Footer
        if (activePanel != &guiPanelFocus) {
            guiFooter.Draw();
        }
    }
    // Draw Side
    guiSide.Draw();

    if (quickViewTimer < quickViewTimeout &&
        !(cachedFocus.type == FOCUSMODULE && cachedFocus.modul == quickView.modul &&
          cachedFocus.layer == quickView.layer && ui.activePanel == &ui.guiPanelFocus)) {

        // println(quickViewTimer);
        guiPanelQuickView.Draw();
    }

    if (introFrame) {
        if (timerIntroScreen < 2750) {
            guiPanelStart.Draw();
        }
        else {
            introFrame = false;
        }
    }

    setRenderState(RENDER_WAIT);
}
void GUI::checkFocusChange() {

    __disable_irq();

    if (newFocus.type != NOFOCUS) { // check new focus set and activate Focus Panel
        if (currentFocus.id != newFocus.id || currentFocus.modul != newFocus.modul ||
            currentFocus.layer != newFocus.layer || currentFocus.type != newFocus.type) { // something changed?
            oldActivePanelID = activePanelID;
            activePanel = panels[6];
            activePanelID = 6;
            currentFocus = newFocus;
            newFocus.type = NOFOCUS;
        }
        else { // nothing change -> same button pressed twice ->back to last Panel
            setPanelActive(6);
            newFocus.type = NOFOCUS;
        }
    }

    cachedFocus = currentFocus; // make focus position safe
    __enable_irq();

    if (cachedFocus.layer == 0) {
        cLayer = cLayerA;
        c4444dot = ((uint16_t(((uint8_t *)&cLayerA)[3] & 0xF0)) << 8) |
                   ((uint16_t(((uint8_t *)&cLayerA)[2] & 0xF0)) << 4) | (uint16_t(((uint8_t *)&cLayerA)[1] & 0xF0)) |
                   ((uint16_t(((uint8_t *)&cLayerA)[0] & 0xF0)) >> 4);
    }
    else {
        cLayer = cLayerB;
        c4444dot = ((uint16_t(((uint8_t *)&cLayerB)[3] & 0xF0)) << 8) |
                   ((uint16_t(((uint8_t *)&cLayerB)[2] & 0xF0)) << 4) | (uint16_t(((uint8_t *)&cLayerB)[1] & 0xF0)) |
                   ((uint16_t(((uint8_t *)&cLayerB)[0] & 0xF0)) >> 4);
    }
}

void GUI::drawSysInfo(std::string message) {
    renderQueue.clear();
    renderState = RENDER_PROGRESS; // block framebuffer Switch
                                   // setDisplayBrightness
    __HAL_TIM_SET_COMPARE(&htim13, TIM_CHANNEL_1,
                          5 * 1000); // 6553* 1-10 -> 65530

    clearFrameBuffer();
    drawStringSoftware("SUPERVISOR", 20, 20, fontBig);
    drawStringSoftware("The system was struck by a cosmic particle", 20, 70, fontMedium);
    drawStringSoftware("Message :", 20, 100, fontMedium);
    drawStringSoftware(message, 20, 150, fontBig);
    SwitchFrameBuffer();
}
#endif // ifdef POLYCONTROL
