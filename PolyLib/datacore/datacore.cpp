#include "datacore.hpp"
#include "debughelper/debughelper.hpp"

LogCurve logMapping(64, 0.1);
LogCurve antiLogMapping(64, 0.9);

void Setting::setValue(int32_t newValue) {
    value = std::clamp(newValue, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
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
    value = std::clamp(newValue, minInputValue, maxInputValue);

    if (mapping == linMap) {
        valueMapped = fast_lerp_f32(min, max, (float)(value - minInputValue) / (float)(maxInputValue - minInputValue));
    }
    else if (mapping == logMap) {
        valueMapped = fast_lerp_f32(0, 1, (float)(value - minInputValue) / (float)(maxInputValue - minInputValue));
        valueMapped = logMapping.mapValueSigned(valueMapped) * (max - min) + min;
    }
    else if (mapping == antilogMap) {
        valueMapped = fast_lerp_f32(0, 1, (float)(value - minInputValue) / (float)(maxInputValue - minInputValue));
        valueMapped = antiLogMapping.mapValueSigned(valueMapped) * (max - min) + min;
    }

    if (valueChangedCallback != nullptr)
        valueChangedCallback();

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

int32_t Analog::reverseMapping(float newValue) {

    newValue = std::clamp(newValue, min, max);

    if (min < 0) { // centered value
        newValue = (newValue) / ((max - min) / 2);
    }
    else {
        newValue = (newValue - min) / (max - min);
    }

    int32_t reverseMapped = minInputValue;

    if (mapping == linMap) {
        reverseMapped = (int32_t)(newValue * (maxInputValue - minInputValue) + minInputValue);
    }
    else if (mapping == antilogMap) {
        reverseMapped =
            (int32_t)(logMapping.mapValueSigned(newValue) * (maxInputValue - minInputValue) + minInputValue);
    }
    else if (mapping == logMap) {
        reverseMapped =
            (int32_t)(antiLogMapping.mapValueSigned(newValue) * (maxInputValue - minInputValue) + minInputValue);
    }

    if (min < 0) { // centered value
        return reverseMapped + inputRange / 2;
    }
    else {
        return reverseMapped;
    }
}

const std::string &Digital::getValueAsString() {
    if (valueNameList == nullptr) {
        return valueName;
    }
    else {
        if ((int32_t)valueNameList->size() >= max - min + 1) {
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
    valueMapped = std::round(fast_lerp_f32(min, max + 1, (float)newValue / (float)MAX_VALUE_12BIT));

    if (valueChangedCallback != nullptr)
        valueChangedCallback();

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::nextValue() {
    valueMapped = changeInt(valueMapped, 1, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::nextValueLoop() {
    valueMapped = changeIntLoop(valueMapped, 1, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void Digital::previousValue() {
    valueMapped = changeInt(valueMapped, -1, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

void BasePatch::removePatchInOut(PatchElement &patch) {
    for (uint32_t i = 0; i < patchesInOut.size(); i++) {
        if (patchesInOut[i] == &patch) {
            patchesInOut.erase(patchesInOut.begin() + i);
            return;
        }
    }
}

bool BasePatch::findPatchInOut(uint8_t output, uint8_t input) {
    for (PatchElement *p : patchesInOut) {
        if (p == nullptr) {
            return false;
        }
        if (p->sourceOut->idGlobal == output && p->targetIn->idGlobal == input)
            return true;
    }
    return false;
}

void Input::collectCurrentSample() {
    sample = 0;
    for (PatchElement *patch : patchesInOut) {
        sample += *patch->sourceOut * patch->amount;
    }
}

void PatchElement::changeAmountEncoderAccelerationMapped(bool direction) {

    if (direction == 0) {
        amountRaw = amountRaw + (-0.5f * ROTARYENCODERACCELERATION);
    }
    if (direction == 1) {
        amountRaw = amountRaw + 0.5f * ROTARYENCODERACCELERATION;
    }
    setAmount(amountRaw);
}

void PatchElement::setAmount(float amountRaw) {
    this->amountRaw = std::clamp(amountRaw, -1.0f, 1.0f);
    amount = this->amountRaw;

    // if (targetIn->mapping == linMap) {
    //     amount = this->amountRaw;
    // }
    // else if (targetIn->mapping == logMap) {
    //     amount = logMapping.mapValueSigned(amountRaw);
    // }
    // else if (targetIn->mapping == antilogMap) {
    //     amount = antiLogMapping.mapValueSigned(amountRaw);
    // }

#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
#endif
}

void PatchElement::setAmountWithoutMapping(float amountRaw) {
    this->amountRaw = std::clamp(amountRaw, -1.0f, 1.0f);
    amount = this->amountRaw;

#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
#endif
}
