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
#include "guiPanelState.hpp"
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

    GUIPanelBase *activePanel = nullptr;

    std::vector<Layer *> layers;

  private:
    std::vector<GUIPanelBase *> panels;

    GUIPanelLive guiPanelLive;
    GUIPanelPatch guiPanelPatch;
    GUIPanelPreset guiPanelPreset;
    GUIPanelConfig guiPanelConfig;

    GUIPanelVoice guiPanelVoice;
    GUIPanelFocus guiPanelFocus;

    GUIPanelPath guiPath;
    GUIPanelState guiState;

    GUISide guiSide;
    GUIFooter guiFooter;
    GUIHeader guiHeader;

    GUIPanelError guiError;

    location oldFocus;
};
