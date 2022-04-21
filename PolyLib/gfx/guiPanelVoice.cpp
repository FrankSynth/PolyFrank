#ifdef POLYCONTROL

#include "guiPanelVoice.hpp"

void drawVoiceStatus(voiceStateStruct *voiceState, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    std::string text;

    float amount = allLayers[voiceState->layerID]->out.left.currentSample[voiceState->voiceID];

    uint8_t color[4];
    *(uint32_t *)color = cWhite;

    color[3] *= amount;

    // if (voiceState->status == PLAY) {
    if (voiceState->status != FREE) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
    }
    drawRectangleChampfered(*(uint32_t *)color, x + 1, y + 1, w - 2, h - 2, 1);

    text = valueToNote(voiceState->note);

    drawString(text, cClear, x + w / 2, y + h / 2 - fontBig->size / 2, fontBig, CENTER);

    text = valueToOctave(voiceState->note);

    drawString(text, cClear, x + w / 2 + 18, y + h / 2 - 2, fontMedium, CENTER);

    text = valueToSharp(voiceState->note);

    drawString(text, cClear, x + w / 2 + 18, y + 2, fontMedium, CENTER);
}

void GUIPanelVoice::Draw() {
    uint16_t relX = 0;
    // register Panel Seetings.settings.

    for (uint16_t v = 0; v < NUMBERVOICES; v++) {
        drawVoiceStatus(&liveData.voiceHandler.voices[layerID][v], panelAbsX + relX + 1, panelAbsY, elementWidth - 2,
                        elementHeigth);
        relX += panelWidth / NUMBERVOICES;
    }
}
void GUIPanelVoice::init(uint8_t layerID, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    elementWidth = width / NUMBERVOICES;
    elementHeigth = height;
    this->layerID = layerID;
}

#endif