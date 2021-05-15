#pragma once

#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "guiActionHandler.hpp"
#include "guiBase.hpp"
#include "guiButtons.hpp"
#include "guiPanelConfig.hpp"
#include "guiPanelError.hpp"
#include "guiPanelFocus.hpp"
#include "guiPanelLive.hpp"
#include "guiPanelPatch.hpp"
#include "guiPanelPath.hpp"
#include "guiPanelPreset.hpp"
#include "guiPanelVoice.hpp"
#include "layer/layer.hpp"
#include "tim.h"
#include <functional>
#include <string>

/// GUI///
class GUI {
  public:
    void Init();
    void Draw();
    void Clear();

    void checkFocusChange();

    // PanelSelect
    void setPanelActive(uint8_t panelID);

    GUIPanelBase *activePanel = nullptr;

    uint8_t activePanelID = 0;
    uint8_t oldActivePanelID = 0;

    std::vector<Layer *> layers;

  private:
    GUIPanelLive guiPanelLive;
    GUIPanelPatch guiPanel_1;
    GUIPanelPreset guiPanel_2;
    GUIPanelConfig guiPanel_3;

    GUIPanelVoice guiPanelVoice;

    std::vector<GUIPanelBase *> panels;

    GUIPanelFocus guiPanelFocus;

    GUIPanelPath guiPath;
    GUISide guiSide;

    GUIFooter guiFooter;
    GUIHeader guiHeader;

    GUIPanelError guiError;

    location oldFocus;
};
