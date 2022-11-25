#include "datacore.hpp"
#include "debughelper/debughelper.hpp"

LogCurve logMapping(64, 0.1);
LogCurve antiLogMapping(64, 0.9);

/////////////ANALOG////////////
#ifdef POLYCONTROL

void Analog::setValue(int32_t newValue) {
    value = std::clamp(newValue, minInputValue, maxInputValue);

    if (mapping == linMap) {
        valueMapped =
            faster_lerp_f32(min, max, (float)(value - minInputValue) / (float)(maxInputValue - minInputValue));
    }
    else if (mapping == logMap) {
        if (min < 0) { // we expect an symmetrical value{
            float valueFloat = 0;
            valueFloat =
                faster_lerp_f32(-1, 1, (float)(value - minInputValue) / (float)(maxInputValue - minInputValue));

            valueMapped = logMapping.mapValueSigned(valueFloat) * (outputRange / 2);
        }
        else { // we expect an symmetrical value{
            valueMapped =
                faster_lerp_f32(0, 1, (float)(value - minInputValue) / (float)(maxInputValue - minInputValue));
            valueMapped = logMapping.mapValueSigned(valueMapped) * (max - min) + min;
        }
    }

    if (valueChangedCallback != nullptr)
        valueChangedCallback();

    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
}

// function for inversed fader
void Analog::setValueInversePoti(int32_t newValue) {
    setValue(4096 - newValue);
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

#endif

/////////////DIGITAL////////////

#ifdef POLYCONTROL
const std::string &Digital::getValueAsString() {
    if (valueNameList == nullptr) {
        return valueName;
    }
    else {
        if ((int32_t)valueNameList->size() >= max - min + 1) {
            valueName = (*valueNameList)[valueMapped - min];
            return valueName;
        }
        else {
            // wrong amount of custom names defined error
            return valueName;
        }
    }
}

void Digital::setValue(int32_t newValue) {
    this->value = newValue;
    valueMapped = std::round(faster_lerp_f32(min, max + 1, (float)newValue / (float)MAX_VALUE_12BIT));

    if (valueChangedCallback != nullptr)
        valueChangedCallback();

    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
}

void Digital::setValueRange(int32_t newValue, int32_t inputMin, int32_t inputMax) {
    this->value = newValue;
    valueMapped = std::round(faster_lerp_f32(min, max, (float)(newValue - inputMin) / (float)(inputMax - inputMin)));

    if (valueChangedCallback != nullptr)
        valueChangedCallback();

    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
}

void Digital::nextValue() {
    valueMapped = changeInt(valueMapped, 1, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
}

void Digital::nextValueLoop() {
    valueMapped = changeIntLoop(valueMapped, 1, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
}

void Digital::previousValue() {
    valueMapped = changeInt(valueMapped, -1, min, max);

    if (valueChangedCallback != nullptr)
        valueChangedCallback();
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
}

#endif

/////////////PATCH////////////

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

#ifdef POLYRENDER

void Input::collectCurrentSample() {
    sample = 0;
    for (PatchElement *patch : patchesInOut) {
        sample += *patch->sourceOut * patch->amount;
    }
}

void PatchElement::setAmount(float amountRaw) {
    amount = std::clamp(amountRaw, -1.0f, 1.0f);
}

#endif

#ifdef POLYCONTROL

void PatchElement::changeAmountEncoderAccelerated(bool direction) {

    if (direction == 0) {
        setAmount(amount + (-0.5f * ROTARYENCODERACCELERATION));
    }
    if (direction == 1) {
        setAmount(amount + 0.5f * ROTARYENCODERACCELERATION);
    }
}

void PatchElement::setAmount(float amountRaw) {
    amount = std::clamp(amountRaw, -1.0f, 1.0f);
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
}

#endif
