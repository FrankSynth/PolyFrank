#ifdef POLYCONTROL

#include "guiPanelVoice.hpp"

void drawVoiceStatus(voiceStateStruct *voiceState, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    std::string text;

    if (voiceState->status == PLAY) {
        drawRectangleChampfered(cWhite, x, y, w, h, 1);
    }

    else if (voiceState->status == SUSTAIN) {
        drawRectangleChampfered(cWhiteBright, x, y, w, h, 1);
    }
    else if (voiceState->status == FREE) {
        drawRectangleChampfered(cWhiteMedium, x, y, w, h, 1);

        drawRectangleChampfered(cClear, x + 1, y + 1, w - 2, h - 2, 1);
        return; // no text lets go back
    }
    text = valueToNote(voiceState->note);

    drawString(text, cBlack, x + w / 2, y + h / 2 - fontBig->size / 2, fontBig, CENTER);

    text = valueToOctave(voiceState->note);

    drawString(text, cBlack, x + w / 2 + 18, y + h / 2 - 2, fontMedium, CENTER);

    text = valueToSharp(voiceState->note);

    drawString(text, cBlack, x + w / 2 + 18, y + 2, fontMedium, CENTER);
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