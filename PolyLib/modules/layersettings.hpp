#pragma once

#include "modules/modules.hpp"

extern const std::vector<std::string> arpeggiatorNameList;

class LayerSettings : public BaseModule {
  public:
    LayerSettings(const char *name) : BaseModule(name) { // call subclass

        // settings.push_back(Setting("BPM", 120, 1, 280, false, continuous));
        // settings.push_back(Setting("Channel", 1, 1, 16, false, continuous));
        // settings.push_back(Setting("Arpeggiator", 0, 0, 5, false, continuous, &arpeggiatorNameList));
    }
};