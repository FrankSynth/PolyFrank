#pragma once

#include "dataHelperFunctions.hpp"
#include "debughelper/debughelper.hpp"
#include "math/polyMath.hpp"
#include <cmath>
#include <functional>
#include <list>
#include <vector>
#ifdef POLYCONTROL
#include <string>
#endif

#define MAX_VALUE_12BIT 3340 // TODO andere rail to rail Opamp.. dann auf größere range setzen
#define MIN_VALUE_12BIT 66   // TODO andere rail to rail Opamp.. dann auf größere range setzen

#define VECTORDEFAULTINITSIZE 5
#define VOICESPERCHIP 4

// only two (binary) or more options (continuous)
enum typeDisplayValue { continuous, binary };
enum typeLinLog { linMap, logMap, antilogMap };

#ifdef POLYCONTROL
extern uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount);
extern uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount);
#endif

class DataElement {
  public:
    DataElement() {}
    ~DataElement() {}

    const std::string &getName();

    typeDisplayValue getType();

    uint8_t id;
    uint8_t layerId;
    uint8_t moduleId;

    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, uint8_t *)> sendSetting;

  protected:
    typeDisplayValue type;

    uint8_t sendOutViaCom;

    std::string name;
    std::string valueName; // value as string
};

// basic data element
class Setting : public DataElement {
  public:
    Setting(const char *name, int32_t value = 0, int32_t min = 0, int32_t max = 1, bool sendOutViaCom = true,
            typeDisplayValue type = continuous, const std::vector<std::string> *valueNameList = nullptr) {

        this->value = value;
        this->defaultValue = value;
        this->valueToggle = value;
        this->min = min;
        this->max = max;

        this->type = type;

        this->name = name;
        this->valueNameList = valueNameList;
    }

    int32_t value;
    int32_t valueToggle;  // short push toggles between two values
    int32_t defaultValue; // init value
    int32_t min;
    int32_t max;

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    const std::vector<std::string> *valueNameList; // custom Name List for different Values

    int32_t getValue();
    void setValue(int32_t newValue);
    inline void resetValue() { setValue(defaultValue); }

    inline void setValueWithoutMapping(int32_t newValue) { value = newValue; }
    inline void setValueWithoutMapping(uint8_t *newValue) { value = *(int32_t *)newValue; }
    void increase(int32_t amount = 1);
    void decrease(int32_t amount = -1);
    void push();
    void pushAndHold();

    void reset();

    const std::string &getValueAsString();
};

class Input;

// derived class Poti
class Analog : public DataElement {
  public:
    Analog(const char *name, float min = 0, float max = 1, float defaultValue = 0, bool sendOutViaCom = true,
           typeLinLog mapping = linMap, Input *input = nullptr, bool displayVis = true) {
        this->value = 0;
        this->min = min;
        this->max = max;
        this->minMaxDifference = max - min;
        // this->defaultValue = fastMap(defaultValue, min, max, MIN_VALUE_12BIT, MAX_VALUE_12BIT);
        this->defaultValue =
            (((float)value - min) / (max - min)) * (MAX_VALUE_12BIT - MIN_VALUE_12BIT) + MIN_VALUE_12BIT;

        this->mapping = mapping;

        this->name = name;

        this->sendOutViaCom = sendOutViaCom;
        this->displayVis = displayVis;

        this->input = input;
    }

    void setValue(int32_t newValue);
    inline void resetValue() { setValue(defaultValue); }

    static std::function<uint8_t(uint8_t, uint8_t, float)> sendViaChipCom;

    inline void changeValue(int32_t change) { setValue(value + change); }
    inline void setValueWithoutMapping(float newValue) {
        valueMapped = newValue;
#ifdef POLYCONTROL
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
#endif
    }
    inline void setValueWithoutMapping(uint8_t *newValue) { value = *(float *)newValue; }

    const std::string &getValueAsString();

    std::string valueName;
    int32_t defaultValue = 0;

    int32_t value;
    float valueMapped;
    float min;
    float max;
    float minMaxDifference;
    bool displayVis;
    Input *input;

  protected:
    typeLinLog mapping;
};

// derived class Poti
class Digital : public DataElement {
  public:
    Digital(const char *name, int32_t min = 0, int32_t max = 1, int32_t defaultValue = 0, bool sendOutViaCom = true,
            const std::vector<std::string> *valueNameList = nullptr, Input *input = nullptr, bool displayVis = true) {
        setValue(value);

        this->min = min;
        this->max = max;
        this->minMaxDifference = max - min;
        this->defaultValue = defaultValue;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;

        this->displayVis = displayVis;

        this->valueNameList = valueNameList;

        this->input = input;
    }

    // Inputs range must be from 0 -> MAX_VALUE_12BIT
    void setValue(int32_t newValue);
    void nextValue();
    void previousValue();
    inline void resetValue() { setValueWithoutMapping(defaultValue); }

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    inline void setValueWithoutMapping(int32_t newValue) {
        valueMapped = newValue;
#ifdef POLYCONTROL
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
#endif
    }
    inline void setValueWithoutMapping(uint8_t *newValue) { valueMapped = *(int32_t *)newValue; }

    const std::string &getValueAsString();
    const std::vector<std::string> *valueNameList; // custom Name List for different Values

    int32_t defaultValue = 0;

    int32_t value;
    int32_t valueMapped;
    std::string valueName;

    int32_t min;
    int32_t max;
    int32_t minMaxDifference;

    bool displayVis;

    Input *input;

  protected:
    typeLinLog mapping;
};

// Name Base
class NameElement {
  public:
    NameElement(const char *name) { this->name = name; }
    ~NameElement() {}

    inline void setName(const char *name) { this->name = name; }

    inline const std::string &getName() { return name; }

  private:
    std::string name;
};

class PatchElementInOut;  // define class to fix no declaration error from Input and Output Class
class PatchElementOutOut; // define class to fix no declaration error from Input and Output Class

// input patchesInOut
class BasePatch {
  public:
    void clearPatches();

    void addPatchInOut(PatchElementInOut &patch);
    void addPatchOutOut(PatchElementOutOut &patch);
    void removePatchInOut(PatchElementInOut &patch);
    void removePatchOutOut(PatchElementOutOut &patch);

    inline const std::string &getName() { return name; };
    inline std::vector<PatchElementInOut *> &getPatchesInOut() { return patchesInOut; }
    inline std::vector<PatchElementOutOut *> &getPatchesOutOut() { return patchesOutOut; }

    uint8_t id;
    uint8_t moduleId;
    uint8_t layerId;

    uint8_t idGlobal;

  protected:
    std::vector<PatchElementInOut *> patchesInOut;
    std::vector<PatchElementOutOut *> patchesOutOut;
    std::string name;
};

class Input : public BasePatch {
  public:
    Input(const char *name) {
        this->name = name;
        patchesInOut.reserve(VECTORDEFAULTINITSIZE);
    }
    float currentSample[VOICESPERCHIP] = {0, 0, 0, 0};

    // calculate all inputs with their attached patchesInOut
    void collectCurrentSample();
};

class Output : public BasePatch {
  public:
    Output(const char *name) {
        this->name = name;
        patchesInOut.reserve(VECTORDEFAULTINITSIZE);
        patchesOutOut.reserve(VECTORDEFAULTINITSIZE);

        currentSample = bufferCurrentSample;
        nextSample = bufferNextSample;
    }

    // set next calculatedSample as current sample
    void updateToNextSample() {
        float *tempPointer = currentSample;
        currentSample = nextSample;
        nextSample = tempPointer;
    }

    float *currentSample;
    float *nextSample;

  private:
    float bufferCurrentSample[VOICESPERCHIP] = {0, 0, 0, 0};
    float bufferNextSample[VOICESPERCHIP] = {0, 0, 0, 0};
};

class PatchElement {
  public:
    inline float getAmount() { return amount; }

    // void setAmount(float value);
    // void resetAmount() { value = defaultValue; }
    // virtual void changeAmount(float change);

    // void setOffset(float offset);
    // void changeOffset(float change);

    float offset;
    float amount;
    uint8_t layerId;
};

class PatchElementInOut : public PatchElement {
  public:
    PatchElementInOut(Output &source, Input &targetIn, uint8_t layerId, float amount = 0) {
        this->sourceOut = &source;
        this->targetIn = &targetIn;
        this->amount = amount;
        this->layerId = layerId;
    }

    void setAmount(float amount);
    void changeAmount(float change);
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendUpdatePatchInOut;

    // inline float getAmount() { return amount; }
    // void setAmount(float amount);

    // void changeAmount(float change);
    // void resetAmount() { amount = defaultamount; }

    bool remove = false;
    Output *sourceOut;
    Input *targetIn;
};

class PatchElementOutOut : public PatchElement {
  public:
    PatchElementOutOut(Output &source, Output &targetOut, uint8_t layerId, float amount = 0, float offset = 0) {
        this->sourceOut = &source;
        this->targetOut = &targetOut;
        this->amount = amount;
        this->offset = offset;
        this->layerId = layerId;
    }
    inline float getOffset() { return offset; }

    void setAmount(float amount);
    void changeAmount(float change);
    void setOffset(float offset);
    void changeOffset(float change);
    void setAmountAndOffset(float amount, float offset);
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendUpdatePatchInOut;

    bool remove = false;
    Output *sourceOut;
    Output *targetOut;

  private:
    float offset;
};

class ID {
  public:
    uint8_t getNewId();

  private:
    uint8_t idCounter = 0;
};

//////////////////////////////// INLINE FUNCTIONS /////////////////////////////////

inline void PatchElementInOut::changeAmount(float change) {
    amount = changeFloat(amount, change, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, amount);
#endif
}
inline void PatchElementInOut::setAmount(float amount) {
    amount = testFloat(amount, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, amount);
#endif
}
inline void PatchElementOutOut::changeAmount(float change) {
    amount = changeFloat(amount, change, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, amount, offset);
#endif
}
inline void PatchElementOutOut::setAmount(float amount) {
    amount = testFloat(amount, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, amount, offset);
#endif
}
inline void PatchElementOutOut::changeOffset(float change) {
    offset = changeFloat(offset, change, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, amount, offset);
#endif
}
inline void PatchElementOutOut::setOffset(float offset) {
    offset = testFloat(offset, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, amount, offset);
#endif
}
inline void PatchElementOutOut::setAmountAndOffset(float amount, float offset) {
    amount = testFloat(amount, -1, 1);
    offset = testFloat(offset, -1, 1);
#ifdef POLYCONTROL
    sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, amount, offset);
#endif
}
inline void BasePatch::clearPatches() {
    patchesInOut.clear();
    patchesOutOut.clear();
}

inline void BasePatch::addPatchInOut(PatchElementInOut &patch) {
    patchesInOut.push_back(&patch);
}
inline void BasePatch::addPatchOutOut(PatchElementOutOut &patch) {
    patchesOutOut.push_back(&patch);
}

inline void Setting::reset() {
    value = defaultValue;
}

inline const std::string &Analog::getValueAsString() {
    return valueName;
}
inline void Setting::increase(int32_t amount) {
    value = changeInt(value, amount, min, max);
}

inline void Setting::decrease(int32_t amount) {
    value = changeInt(value, amount, min, max);
}

inline void Setting::push() {
    int32_t temp = value;
    value = valueToggle;
    valueToggle = temp;
}

inline void Setting::pushAndHold() {
    reset();
}

inline const std::string &DataElement::getName() {
    return name;
}

inline typeDisplayValue DataElement::getType() {
    return type;
}

inline int32_t Setting::getValue() {
    return value;
}

inline uint8_t ID::getNewId() {
    return idCounter++;
}
