#include "datacore.hpp"

void Setting::setValue(int32_t newValue) {
    value = testInt(newValue, min, max);
    if (valueNameList == nullptr)
        valueName = std::to_string(value);

#ifdef POLYCONTROL
        // if (sendOutViaCom) {
        //     sendSetting(layerId, moduleId, id, value);
        // }
#endif
}

const std::string &Setting::getValueAsString() {
    if (valueNameList == nullptr) {
        return valueName;
    }
    else {
        if ((int32_t)valueNameList->size() == max - min + 1)
            return (*valueNameList)[value - min];
        else {
            // wrong amount of custom names defined error
            return valueName;
        }
    }
}

void Analog::setValue(int32_t newValue) {
    value = testInt(newValue, 0, MAX_VALUE_16BIT);

    // if (mapping == linMap) {
    valueMapped = fast_lerp_f32(min, max, (float)value / MAX_VALUE_16BIT);
    // }
    // else if (mapping == logMap) {
    //     valueMapped = (powf(1.5, value) - 1) / powf(1.5, MAX_VALUE_16BIT) * (max - min) + min; // Log with mapping
    // }
    // else if (mapping == antilogMap) {
    //     valueMapped = (logf(value + 1) * (max - min)) / logf(MAX_VALUE_16BIT + 1) + min; // Antilog with mapping
    // }

    valueName = std::to_string(valueMapped);

#ifdef POLYCONTROL
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

const std::string &Digital::getValueAsString() {
    if (valueNameList == nullptr) {
        return valueName;
    }
    else {
        if ((int32_t)valueNameList->size() == max - min + 1) {
            return (*valueNameList)[valueMapped - min];
        }
        else {
            // wrong amount of custom names defined error
            return valueName;
        }
    }
}

void Digital::setValue(int32_t newValue) {
    this->value = newValue;
    valueMapped = round(fast_lerp_f32(min, max + 1, (float)newValue / (float)MAX_VALUE_16BIT));
    valueName = std::to_string(valueMapped);

#ifdef POLYCONTROL
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::nextValue() {
    valueMapped = changeInt(valueMapped, 1, min, max);
    valueName = std::to_string(valueMapped);

#ifdef POLYCONTROL
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::previousValue() {
    valueMapped = changeInt(valueMapped, -1, min, max);
    valueName = std::to_string(valueMapped);

#ifdef POLYCONTROL
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void BasePatch::removePatchInOut(PatchElementInOut &patch) {
    for (uint32_t i = 0; i < patchesInOut.size(); i++) {
        if (patchesInOut[i] == &patch) {
            patchesInOut.erase(patchesInOut.begin() + i);
            return;
        }
    }
}

void BasePatch::removePatchOutOut(PatchElementOutOut &patch) {
    for (uint32_t i = 0; i < patchesOutOut.size(); i++) {
        if (patchesOutOut[i] == &patch) {
            patchesOutOut.erase(patchesOutOut.begin() + i);
            return;
        }
    }
}

void Input::collectCurrentSample() {
    currentSample[0] = 0;
    currentSample[1] = 0;
    currentSample[2] = 0;
    currentSample[3] = 0;
    for (PatchElementInOut *patch : patchesInOut) {
        currentSample[0] += patch->sourceOut->currentSample[0] * patch->getAmount();
        currentSample[1] += patch->sourceOut->currentSample[1] * patch->getAmount();
        currentSample[2] += patch->sourceOut->currentSample[2] * patch->getAmount();
        currentSample[3] += patch->sourceOut->currentSample[3] * patch->getAmount();
    }
}