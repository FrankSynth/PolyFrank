#include "datacore.hpp"

LogCurve logMapping(16, 0.1);
LogCurve antiLogMapping(16, 0.9);

void Setting::setValue(int32_t newValue) {
    value = testInt(newValue, min, max);

#ifdef POLYCONTROL
    if (valueNameList == nullptr)
        valueName = std::to_string(value);
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
        if ((int32_t)valueNameList->size() == (max - min + 1))
            return (*valueNameList)[value - min];
        else {
            // wrong amount of custom names defined error
            PolyError_Handler("ERROR | Configuration | Setting -> NameList wrong lenght");
            return valueName;
        }
    }
}

void Analog::setValue(int32_t newValue) {
    value = testInt(newValue, MIN_VALUE_12BIT, MAX_VALUE_12BIT);

    if (mapping == linMap) {
        valueMapped =
            fast_lerp_f32(min, max, (float)(value - MIN_VALUE_12BIT) / (float)(MAX_VALUE_12BIT - MIN_VALUE_12BIT));
    }
    else if (mapping == logMap) {
        valueMapped =
            fast_lerp_f32(0, 1, (float)(value - MIN_VALUE_12BIT) / (float)(MAX_VALUE_12BIT - MIN_VALUE_12BIT));
        valueMapped = logMapping.mapValue(valueMapped) * (max - min) + min;
    }
    else if (mapping == antilogMap) {
        valueMapped =
            fast_lerp_f32(0, 1, (float)(value - MIN_VALUE_12BIT) / (float)(MAX_VALUE_12BIT - MIN_VALUE_12BIT));
        valueMapped = antiLogMapping.mapValue(valueMapped) * (max - min) + min;
    }

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

int32_t Analog::reverseMapping(float newValue) {

    newValue = testFloat(newValue, min, max);

    int32_t reverseMapped = MIN_VALUE_12BIT;

    if (mapping == linMap) {
        newValue = (newValue - min) / (max - min);
        reverseMapped = (int32_t)(newValue * (MAX_VALUE_12BIT - MIN_VALUE_12BIT) + MIN_VALUE_12BIT);
    }
    else if (mapping == antilogMap) {
        newValue = (newValue - min) / (max - min);
        reverseMapped =
            (int32_t)(logMapping.mapValue(newValue) * (MAX_VALUE_12BIT - MIN_VALUE_12BIT) + MIN_VALUE_12BIT);
    }
    else if (mapping == logMap) {
        newValue = (newValue - min) / (max - min);
        reverseMapped =
            (int32_t)(antiLogMapping.mapValue(newValue) * (MAX_VALUE_12BIT - MIN_VALUE_12BIT) + MIN_VALUE_12BIT);
    }
    return reverseMapped;
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
    valueMapped = round(fast_lerp_f32(min, max + 1, (float)newValue / (float)MAX_VALUE_12BIT));

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::nextValue() {
    valueMapped = changeInt(valueMapped, 1, min, max);

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::previousValue() {
    valueMapped = changeInt(valueMapped, -1, min, max);

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
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

    // TODO Log input amount
    currentSample[0] = 0;
    currentSample[1] = 0;
    currentSample[2] = 0;
    currentSample[3] = 0;
    for (PatchElementInOut *patch : patchesInOut) {
        Output *sourceOut = patch->sourceOut;
        float amount = patch->getAmount();
        currentSample[0] += sourceOut->currentSample[0] * amount;
        currentSample[1] += sourceOut->currentSample[1] * amount;
        currentSample[2] += sourceOut->currentSample[2] * amount;
        currentSample[3] += sourceOut->currentSample[3] * amount;
    }
}