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

    text = "BPM ";
    int16_t BoxWidth = 105;

    if (liveData.livemodeClockSource.value < 2) {

        text.append("(EXT)");
        BoxWidth += 50;
    }

    text.append(": ");
    if (liveData.livemodeClockSource.value < 2 && liveData.extClockTimeout > 300) // TIMEOUT 300 sec
        text.append(" --");
    else
        text.append(std::to_string((uint16_t)std::round(clock.bpm)));

    relX -= drawBoxWithTextFixWidth(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                    panelHeight, spacer, 1, CENTER);

    // // Draw Temperature //
    // if (globalSettings.dispTemperature.value == 1) {
    //     relX -= 2; // some extra space between the boxes
    //     text = globalSettings.temperatureC.getValueAsString(true, true);

    //     BoxWidth = 70;

    //     relX -= drawBoxWithTextFixWidth(text, font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
    //                                     panelHeight, spacer, 1, CENTER);
    // }
}

#endif // ifdef POLYCONTROL
