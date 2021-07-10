#include "datacore.hpp"
#include "debughelper/debughelper.hpp"

LogCurve logMapping(32, 0.1);
LogCurve antiLogMapping(32, 0.9);

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
    value = testInt(newValue, minInputValue, maxInputValue);

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

#ifdef POLYCONTROL
    valueName = std::to_string(valueMapped);
    if (sendOutViaCom) {
        sendSetting(layerId, moduleId, id, valueMapped);
    }
#endif
}

int32_t Analog::reverseMapping(float newValue) {

    newValue = testFloat(newValue, min, max);

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
    valueMapped = std::round(fast_lerp_f32(min, max + 1, (float)newValue / (float)MAX_VALUE_12BIT));

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

void Digital::nextValueLoop() {
    valueMapped = changeIntLoop(valueMapped, 1, min, max);

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

// TODO remove all out out patch stuff

// void BasePatch::removePatchOutOut(PatchElementOutOut &patch) {
//     for (uint32_t i = 0; i < patchesOutOut.size(); i++) {
//         if (patchesOutOut[i] == &patch) {
//             patchesOutOut.erase(patchesOutOut.begin() + i);
//             return;
//         }
//     }
// }

void Input::collectCurrentSample() {

    currentSample[0] = 0;
    currentSample[1] = 0;
    currentSample[2] = 0;
    currentSample[3] = 0;
    for (PatchElementInOut *patch : patchesInOut) {
        Output *sourceOut = patch->sourceOut;
        const float &amount = patch->amount;
        currentSample[0] += sourceOut->currentSample[0] * amount;
        currentSample[1] += sourceOut->currentSample[1] * amount;
        currentSample[2] += sourceOut->currentSample[2] * amount;
        currentSample[3] += sourceOut->currentSample[3] * amount;
    }
}

void PatchElementInOut::changeAmountEncoderAccelerationMapped(bool direction) {

    if (direction == 0) {
        amountRaw = amountRaw + (-0.5f * ROTARYENCODERACELLARATION);
    }
    if (direction == 1) {
        amountRaw = amountRaw + 0.5f * ROTARYENCODERACELLARATION;
    }
    setAmount(amountRaw);
}

void PatchElementInOut::setAmount(float amountRaw) {
    this->amountRaw = testFloat(amountRaw, -1, 1);

    if (targetIn->mapping == linMap) {
        amount = this->amountRaw;
    }
    else if (targetIn->mapping == logMap) {
        amount = logMapping.mapValueSigned(amountRaw);
    }
    else if (targetIn->mapping == antilogMap) {
        amount = antiLogMapping.mapValueSigned(amountRaw);
    }

#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
#endif
}

void PatchElementInOut::setAmountWithoutMapping(float amountRaw) {
    this->amountRaw = testFloat(amountRaw, -1, 1);
    amount = this->amountRaw;

#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
#endif
}

// void PatchElementOutOut::setAmountWithoutMapping(float amount) {
//     this->amount = testFloat(amount, -1, 1);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }

// void PatchElementOutOut::setAmount(float amount) {
//     this->amount = testFloat(amount, -1, 1);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }

// void PatchElementOutOut::setOffsetWithoutMapping(float offset) {
//     this->offset = testFloat(offset, -1, 1);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }

// void PatchElementOutOut::setOffset(float offset) {
//     this->offset = testFloat(offset, -1, 1);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }