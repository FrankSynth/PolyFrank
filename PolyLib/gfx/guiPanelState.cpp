#ifdef POLYCONTROL
#include "guiPanelState.hpp"

extern Clock clock;

void GUIPanelState::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelState::Draw() {
    uint16_t relX = panelWidth;
    uint16_t relY = 0;

    uint16_t spacer = 20;
    std::string text;

    // Draw BPM//

    text = "BPM : " + std::to_string((uint16_t)std::round(clock.bpm));

    int16_t BoxWidth = 105;

    relX -= drawBoxWithTextFixWidth(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                    panelHeight, spacer, 1, CENTER);

    // Draw Temperature //
    if (globalSettings.dispTemperature.value == 1) {
        relX -= 5; // some extra space between the boxes
        text = "T:" + std::to_string((uint16_t)globalSettings.temperature) + " C";

        BoxWidth = 70;

        relX -= drawBoxWithTextFixWidth(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                        panelHeight, spacer, 1, CENTER);
    }
}

#endif // ifdef POLYCONTROL
