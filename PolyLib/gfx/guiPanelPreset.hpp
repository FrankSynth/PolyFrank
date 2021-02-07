#pragma once

#include "guiPanelBase.hpp"

class GUIPanelPreset : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 1);
    void Draw();

    void updateEntrys();

    void registerElements();
    void registerPanelSettings();

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = 0;

    const uint16_t maxEntrys = PRESETPANELENTRYS;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scrollPreset = Scroller(PRESETPANELENTRYS);

    Scroller scrollFirstName = Scroller(PRESETPANELENTRYS);
    Scroller scrollSecondName = Scroller(PRESETPANELENTRYS);

    const std::vector<std::string> firstName = {"Space",  "Synth",  "Green",  "Blue",      "Pink",   "Return",
                                                "Master", "Radium", "Nucleo", "Comet",     "Sir",    "Chill",
                                                "Frank",  "Mono",   "Poly",   "Andromeda", "Temple", "Buffer"};
    const std::vector<std::string> secondName = {"Leavce", "Cowboy", "Traveler", "Tree",   "Floyd",   "Void", "Chief",
                                                 "Vortex", "Wave",   "Moody",    "Rhythm", "42",      "Keys", "Chords",
                                                 "Ship",   "Galaxy", "Quest",    "Error",  "Overflow"};

    Preset_PanelElement PanelElementPreset[PRESETPANELENTRYS];
};
