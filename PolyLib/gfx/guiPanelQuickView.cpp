#ifdef POLYCONTROL

#include "guiPanelQuickView.hpp"

void GUIPanelQuickView::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    this->width = width;
    this->height = height;
    this->x = x;
    this->y = y;
}

void GUIPanelQuickView::Draw() {
    if (quickView.modul >= allLayers[0]->getModules().size() || quickView.layer == 0xff) {
        return;
    }
    if (millis() < 5000) // skip on boot
        return;

    uint32_t elementWidth = (width + 2) / 4;
    // uint32_t elementHeight = elementWidth / 2;

    BaseModule *module = allLayers[quickView.layer]->modules[quickView.modul];

    ModuleType type = module->moduleType;
    if (type == MODULE_OSC_A || type == MODULE_OSC_B || type == MODULE_SUB || type == MODULE_LFO ||
        type == MODULE_ADSR) {

        drawRectangleFill(cBlackTransparent, x, y, width, height); // hide background

        drawRectangleFill(cHighlight, x, y, width, 50);
        drawString(module->getShortName(), cFont_Select, width / 2, 25 - fontBig->size / 2, fontBig, CENTER);

        drawWaveFromModule(waveQuickBuffer, allLayers[quickView.layer]->modules[quickView.modul],
                           LCDWIDTH / 2 - waveQuickBuffer.width / 2, height / 2 - waveQuickBuffer.height / 2);

        // switch (type) {
        //     case MODULE_OSC_A:
        //     case MODULE_OSC_B: {
        //         drawWaveFromModule(waveQuickBuffer, allLayers[quickView.layer]->modules[quickView.modul],
        //                            LCDWIDTH / 2 - waveQuickBuffer.width / 2, height / 2 - waveQuickBuffer.height /
        //                            2);
        //         break;
        //     }

        //     case MODULE_LFO: {

        //         break;
        //     }
        //     default: break;
        // }
    }

    // drawQuickViewDigital(&((OSC_A *)module)->dSample0, x, y + WAVEFORMHEIGHT, elementWidth - 2,

    // drawQuickViewAnalog(&((OSC_A *)module)->aMasterTune, x, y + WAVEFORMHEIGHT + elementHeight,
    //                     elementWidth - 2, elementHeight);
}

#endif