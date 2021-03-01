#ifdef POLYCONTROL
#include "guiPanelPath.hpp"

extern Clock clock;

void GUIPanelPath::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelPath::Draw(uint8_t onlyLayer) {
    drawRectangleFill(cClear, panelAbsX, panelAbsY, panelWidth, panelHeight);
    uint16_t relX = 0;
    uint16_t relY = 0;

    uint16_t spacer = 20;

    //// Draw Layer Field////

    uint8_t layerID = allLayers[currentFocus.layer]->id; // get ID
    std::string text;
    if (layerID == 0) {
        text = "LAYER A"; // Layer name
    }
    else if (layerID == 1) {
        text = "LAYER B"; // Layer name
    }
    relX +=
        drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1, CENTER);
    relX += 1;

    // Draw BPM//

    text = "BPM : " + std::to_string((uint16_t)clock.bpm);

    int16_t bpmBoxWidth = 100;

    drawRectangleChampfered(cWhite, panelAbsX + panelWidth - bpmBoxWidth, relY + panelAbsY, bpmBoxWidth, panelHeight,
                            1); // draw Box
    drawString(text, cBlack, panelAbsX + panelWidth - bpmBoxWidth / 2,
               relY + panelAbsY + (-font->size + panelHeight) / 2, font, CENTER); // draw text, height centered

    // Only Layer mode?
    if (onlyLayer) {
        return;
    }
    //// Draw Module Field////
    if (currentFocus.type >= FOCUSMODULE) {
        if (!allLayers[currentFocus.layer]->getModules().size()) { //  empty
            return;
        }
        std::string text = allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getName(); // Modul name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }

    //// Draw Focus Field////

    if (currentFocus.type == FOCUSOUTPUT) {
        if (!allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getOutputs().size()) { //  empty
            return;
        }
        std::string text = allLayers[currentFocus.layer]
                               ->getModules()[currentFocus.modul]
                               ->getOutputs()[currentFocus.id]
                               ->getName(); // Output name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
    else if (currentFocus.type == FOCUSINPUT) {
        if (!allLayers[currentFocus.layer]->getModules()[currentFocus.modul]->getInputs().size()) { //  empty
            return;
        }

        std::string text = allLayers[currentFocus.layer]
                               ->getModules()[currentFocus.modul]
                               ->getInputs()[currentFocus.id]
                               ->getName(); // Input name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
}

#endif // ifdef POLYCONTROL
