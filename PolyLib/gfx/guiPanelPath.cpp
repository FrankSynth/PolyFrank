#ifdef POLYCONTROL
#include "guiPanelPath.hpp"

void GUIPanelPath::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelPath::Draw(uint8_t onlyLayer) {
    uint16_t relX = 0;
    uint16_t relY = 0;

    uint16_t spacer = 20;

    //// Draw Layer Field////

    uint8_t layerID = allLayers[cachedFocus.layer]->id; // get ID
    std::string text;
    if (layerID == 0) {
        text = "A"; // Layer name
        relX += drawBoxWithText(text, &GUI_FontBahnschriftSemiBold28_FontInfo, cLayerA, cBlack, relX + panelAbsX,
                                relY + panelAbsY, panelHeight, spacer, 1, CENTER);
    }
    else if (layerID == 1) {
        text = "B"; // Layer name
        relX += drawBoxWithText(text, &GUI_FontBahnschriftSemiBold28_FontInfo, cLayerB, cBlack, relX + panelAbsX,
                                relY + panelAbsY, panelHeight, spacer, 1, CENTER);
    }

    relX += 1;

    // Only Layer mode?
    if (onlyLayer) {
        return;
    }
    //// Draw Module Field////
    if (cachedFocus.type >= FOCUSMODULE) {
        if (!allLayers[cachedFocus.layer]->getModules().size()) { //  empty
            return;
        }
        std::string text = allLayers[cachedFocus.layer]->getModules()[cachedFocus.modul]->getShortName(); // Modul name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }

    //// Draw Focus Field////

    if (cachedFocus.type == FOCUSOUTPUT) {
        if (!allLayers[cachedFocus.layer]->getModules()[cachedFocus.modul]->getOutputs().size()) { //  empty
            return;
        }
        std::string text = allLayers[cachedFocus.layer]
                               ->getModules()[cachedFocus.modul]
                               ->getOutputs()[cachedFocus.id]
                               ->getName(); // Output name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
    else if (cachedFocus.type == FOCUSINPUT) {
        if (!allLayers[cachedFocus.layer]->getModules()[cachedFocus.modul]->getInputs().size()) { //  empty
            return;
        }

        std::string text = allLayers[cachedFocus.layer]
                               ->getModules()[cachedFocus.modul]
                               ->getInputs()[cachedFocus.id]
                               ->getName(); // Input name

        relX += drawBoxWithText(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, panelHeight, spacer, 1,
                                CENTER);
        relX += 1;
    }
}

#endif // ifdef POLYCONTROL
