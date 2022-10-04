#pragma once

#include "guiPanelBase.hpp"

class GUIPanelPreset : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 1);
    void Draw();

    void updateEntrys();

    void registerElements();
    void registerPanelSettings();

    void saveLayerToPreset(presetStruct *preset, std::string firstName, std::string secondName, std::string thirdName);
    void loadPresetToLayer(presetStruct *preset, LayerSelect layer);

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

    Scroller scrollFirstName = Scroller();
    Scroller scrollSecondName = Scroller();
    Scroller scrollThirdName = Scroller();

    const std::vector<std::string> firstName = {"Space",  "Synth",  "Green",  "Blue",      "Pink",   "Return",
                                                "Master", "Radium", "Nucleo", "Comet",     "Sir",    "Chill",
                                                "Frank",  "Mono",   "Poly",   "Andromeda", "Temple", "Buffer"};
    const std::vector<std::string> secondName = {"Leavce", "Cowboy", "Traveler", "Tree",   "Floyd",   "Void", "Chief",
                                                 "Vortex", "Wave",   "Moody",    "Rhythm", "42",      "Keys", "Chords",
                                                 "Ship",   "Galaxy", "Quest",    "Error",  "Overflow"};

    const std::vector<std::string> thirdName = {"", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "42", "#"};

    Preset_PanelElement PanelElementPreset[PRESETPANELENTRYS];
};
