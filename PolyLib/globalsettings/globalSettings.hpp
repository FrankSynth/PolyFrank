#pragma once

#include "datacore/datacore.hpp"
#include <vector>

class GlobalSettings {
  public:
    GlobalSettings() {

        // push all settings here
        __globSettings.push_back(&amountLayers);

        // init setting IDs
        initID();
    }

    // all Settings, don't forget to push to __globSettings vector of this class
    Setting amountLayers = Setting("Active Layers", 0, 0, 1, false, binary, &amountLayerNameList);

    // init
    void initID() {
        ID modID;
        for (Setting *setting : __globSettings) {
            setting->id = modID.getNewId();
        }
    }

    // return all settings
    std::vector<Setting *> getSettings() { return __globSettings; }

  private:
    const std::vector<std::string> amountLayerNameList = {"A", "A+B"};

    std::vector<Setting *> __globSettings;
};