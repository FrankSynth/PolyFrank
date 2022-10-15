#pragma once

#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "guiActionHandler.hpp"
#include "guiButtons.hpp"
#include "guiPanelConfig.hpp"
#include "guiPanelEffect.hpp"
#include "guiPanelError.hpp"
#include "guiPanelFocus.hpp"
#include "guiPanelLive.hpp"
#include "guiPanelPatch.hpp"
#include "guiPanelPatchMatrix.hpp"
#include "guiPanelPath.hpp"
#include "guiPanelPreset.hpp"
#include "guiPanelQuickView.hpp"
#include "guiPanelStart.hpp"
#include "guiPanelState.hpp"
#include "guiPanelString.hpp"
#include "guiPanelVoice.hpp"
#include "layer/layer.hpp"

/// GUI///
class GUI {
  public:
    void Init();
    void Draw();
    void Clear();

    void checkFocusChange();

    GUIPanelBase *activePanel = nullptr;

    std::vector<Layer *> layers;

    std::vector<GUIPanelBase *> panels;

    GUIPanelLive guiPanelLiveData;
    GUIPanelLive guiPanelArp;

    GUIPanelPatchMatrix guiPanelPatch;
    GUIPanelPreset guiPanelPreset;
    GUIPanelConfig guiPanelConfig;

    GUIPanelVoice guiPanelVoice[2];
    GUIPanelFocus guiPanelFocus;

    GUIPanelPath guiPath;
    GUIPanelState guiState;
    GUIPanelStart guiPanelStart;
    GUIPanelEffect guiPanelEffect;

    GUIPanelString guiPanelDebug;

    GUIPanelQuickView guiPanelQuickView;

    GUISide guiSide;
    GUIFooter guiFooter;
    GUIHeader guiHeader;

    GUIPanelError guiError;

    location oldFocus;

  private:
};
