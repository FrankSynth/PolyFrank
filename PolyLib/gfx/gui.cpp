#ifdef POLYCONTROL
#include "gui.hpp"

GUI ui;

void GUI::Init() { // add settings pointer
    // init Display
    GFX_Init();

    guiPanelFocus.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER);
    guiPanelLive.init(CENTERWIDTH, CENTERHEIGHT - VOICEHEIGHT - SPACER, BOARDERWIDTH,
                      HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "LIVE", 0);
    guiPanel_1.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "PATCH", 1);
    guiPanel_2.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "PRESET", 2);
    guiPanel_3.init(CENTERWIDTH, CENTERHEIGHT, BOARDERWIDTH, HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER, "CONFIG", 3);

    // multiLayer detected
    if (globalSettings.multiLayer.value == 1) {
        guiPanelVoice.init(0, CENTERWIDTH, VOICEHEIGHT / 2, BOARDERWIDTH,
                           HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER + CENTERHEIGHT - VOICEHEIGHT);

        guiPanelVoice.init(1, CENTERWIDTH, VOICEHEIGHT / 2, BOARDERWIDTH,
                           HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER + CENTERHEIGHT - VOICEHEIGHT + VOICEHEIGHT / 2);
    }
    else {
        for (uint8_t i = 0; i < 2; i++) {
            if (allLayers[i]->LayerState.value == 1) {
                guiPanelVoice.init(i, CENTERWIDTH, VOICEHEIGHT, BOARDERWIDTH,
                                   HEADERHEIGHT + SPACER + FOCUSHEIGHT + SPACER + CENTERHEIGHT - VOICEHEIGHT);
            }
        }

    } // add Panels to vector
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

    checkFocusChange();
    setPanelActive(0);

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
    if (globalSettings.error.errorActive) {
        guiError.Draw();
    }
    else {
        checkFocusChange();

        // Draw Panel
        if (activePanel != nullptr) {
            activePanel->Draw();

            if (activePanel == &guiPanelFocus) { // focus Panel draw complete Path

                guiPath.Draw();
            }

            else if (activePanel == &guiPanelLive) { // liveMode Panel draw short Path and VoiceState

                guiPanelVoice.Draw();
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

        // Draw Footer
        guiFooter.Draw();
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
