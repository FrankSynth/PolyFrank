#ifdef POLYCONTROL

#include "guiPanelVoice.hpp"

void drawVoiceStatus(voiceStateStruct *voiceState, uint32_t x, uint32_t y, uint16_t w, uint16_t h) {
    std::string text;

    float amount = allLayers[voiceState->layerID]->out.vca.currentSample[voiceState->voiceID];

    uint8_t color[4];
    *(uint32_t *)color = cWhite;

    color[3] *= amount;

    drawRectangleChampfered(*(uint32_t *)color, x, y + 1, w, h - 2, 1);

    if (voiceState->status != FREE) {
        drawRectangleFill(cWhite, x, y, w, 1);
        drawRectangleFill(cWhite, x, y, 1, h);
        drawRectangleFill(cWhite, x + w - 1, y, 1, h);
        drawRectangleFill(cWhite, x, y + h - 1, w, 1);
    }

    text = valueToNote(voiceState->note);

    drawString(text, cGreyDark, x + w / 2, y + h / 2 - fontBig->size / 2, fontBig, CENTER);

    text = valueToOctave(voiceState->note);

    drawString(text, cGreyDark, x + w / 2 + 18, y + h / 2 - 2, fontMedium, CENTER);

    text = valueToSharp(voiceState->note);

    drawString(text, cGreyDark, x + w / 2 + 18, y + 2, fontMedium, CENTER);
}

void GUIPanelVoice::Draw() {
    if (layerID == 255)
        return;
    uint16_t relX = 0;
    // register Panel Seetings.settings.

    for (uint32_t v = 0; v < NUMBERVOICES; v++) {
        drawVoiceStatus(&liveData.voiceHandler.voices[layerID][v], panelAbsX + relX, panelAbsY, elementWidth - 2,
                        elementHeigth);
        relX += elementWidth;
    }
}
void GUIPanelVoice::init(uint8_t layerID, uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    elementWidth = (width + 2) / NUMBERVOICES;
    elementHeigth = height;
    this->layerID = layerID;
}

#endif