#ifdef POLYCONTROL
#include "guiPanelState.hpp"
#include "humanInterface/hid.hpp"

extern Clock clock;

extern SLOTSTATE saveSlotState[2][3];

void GUIPanelState::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelState::Draw() {
    uint16_t relX = panelWidth;
    uint16_t relY = 0;

    uint16_t spacer = 10;
    std::string text;

    uint32_t traffiColor;

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
    relX -= 1;

    spacer = 10;

    if (FlagHandler::USB_FS_CONNECTED) { // IF COM Connected
        BoxWidth = 118;

        relX -= drawBoxWithTextFixWidth("COM", font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                        panelHeight, spacer, 1, LEFT);

        if (FlagHandler::COM_USB_TRAFFIC) {
            FlagHandler::COM_USB_TRAFFIC = false;
            comTraffic = 0;
        }
        if (comTraffic < 1000) {
            traffiColor = cLayer;
        }
        else {
            traffiColor = cBlack;
        }

        copyBitmapToBuffer(bmpUSBLogo, traffiColor, relX + panelAbsX + BoxWidth - bmpUSBLogo.XSize - 3, panelAbsY);

        relX -= 1;
    }

    if (globalSettings.midiSource.value == 0) {
        BoxWidth = 118;

        if (FlagHandler::USB_HS_CONNECTED) { // IF MIDI Connected

            relX -= drawBoxWithTextFixWidth("MIDI", font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                            panelHeight, spacer, 1, LEFT);

            if (FlagHandler::MIDI_USB_TRAFFIC) {
                FlagHandler::MIDI_USB_TRAFFIC = false;
                midiTraffic = 0;
            }
            if (midiTraffic < 1000) {
                traffiColor = cLayer;
            }
            else {
                traffiColor = cBlack;
            }
            copyBitmapToBuffer(bmpUSBLogo, traffiColor, relX + panelAbsX + BoxWidth - bmpUSBLogo.XSize - 3, panelAbsY);
            // drawRectangleFill(traffiColor, relX + panelAbsX + BoxWidth - bmpUSBLogo.XSize - 12, panelAbsY + 6, 4,
            //                   panelHeight - 10);
            relX -= 1;
        }
    }
    else if (globalSettings.midiSource.value == 1) {
        BoxWidth = 85;

        relX -= drawBoxWithTextFixWidth("MIDI", font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                        panelHeight, spacer, 1, LEFT);

        if (FlagHandler::MIDI_DIN_TRAFFIC) {
            FlagHandler::MIDI_DIN_TRAFFIC = false;
            midiTraffic = 0;
        }
        if (midiTraffic < 1000) {
            traffiColor = cLayer;
        }
        else {
            traffiColor = cBlack;
        }

        copyBitmapToBuffer(bmpDINLogo, traffiColor, relX + panelAbsX + BoxWidth - bmpDINLogo.XSize - 5, panelAbsY);
        // drawRectangleFill(traffiColor, relX + panelAbsX + BoxWidth - bmpUSBLogo.XSize - 12, panelAbsY + 6, 4,
        //                   panelHeight - 10);

        relX -= 1;
    }

    if (globalSettings.functionButtons.value == 0) { // saveSlots
        BoxWidth = 116;

        relX -= drawBoxWithTextFixWidth("SLOTS", font, cWhite, cBlack, relX + panelAbsX, relY + panelAbsY, BoxWidth,
                                        panelHeight, spacer, 1, LEFT);

        uint32_t slotWidth = 10;
        uint32_t slotColor = cBlack;
        for (size_t slot = 0; slot < 3; slot++) {
            if (saveSlotState[cachedFocus.layer][slot] == SLOTUSED) {
                slotColor = cLayer;
            }
            else {
                slotColor = cBlack;
            }

            drawRectangleChampfered(slotColor, relX + panelAbsX + 72 + (slotWidth + 3) * slot, relY + panelAbsY + 4,
                                    slotWidth, panelHeight - 8, 2);
        }
    }
}

#endif // ifdef POLYCONTROL
