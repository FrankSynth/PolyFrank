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

#define MAX_VALUE_12BIT 3340 // todo andere rail to rail Opamp.. dann auf größere range setzen
#define MIN_VALUE_12BIT 66   // todo andere rail to rail Opamp.. dann auf größere range setzen

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
    void resetValue() { setValue(defaultValue); }

    void setValueWithoutMapping(int32_t newValue) { value = newValue; }
    void setValueWithoutMapping(uint8_t *newValue) { value = *(int32_t *)newValue; }
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
        this->defaultValue = defaultValue;

        this->mapping = mapping;

        this->name = name;

        this->sendOutViaCom = sendOutViaCom;
        this->displayVis = displayVis;

        this->input = input;
    }

    void setValue(int32_t newValue);
    void resetValue() { setValueWithoutMapping(defaultValue); }

    static std::function<uint8_t(uint8_t, uint8_t, float)> sendViaChipCom;

    void changeValue(int32_t change) { setValue(value + change); }
    void setValueWithoutMapping(float newValue) {
        valueMapped = newValue;
#ifdef POLYCONTROL
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
#endif
    }
    void setValueWithoutMapping(uint8_t *newValue) { value = *(float *)newValue; }

    const std::string &getValueAsString();

    std::string valueName;
    float defaultValue = 0;

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
    void resetValue() { setValueWithoutMapping(defaultValue); }

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    void setValueWithoutMapping(int32_t newValue) {
        valueMapped = newValue;
#ifdef POLYCONTROL
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
#endif
    }
    void setValueWithoutMapping(uint8_t *newValue) { valueMapped = *(int32_t *)newValue; }

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

    void setName(const char *name) { this->name = name; }

    const std::string &getName() { return name; }

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

    const std::string &getName() { return name; };
    std::vector<PatchElementInOut *> &getPatchesInOut() { return patchesInOut; }
    std::vector<PatchElementOutOut *> &getPatchesOutOut() { return patchesOutOut; }

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

    float bufferCurrentSample[VOICESPERCHIP] = {0, 0, 0, 0};
    float bufferNextSample[VOICESPERCHIP] = {0, 0, 0, 0};
};

class PatchElement {
  public:
    inline float getAmount() { return value; }
    inline float getOffset() { return offset; }

    void setAmount(float value);
    void resetAmount() { value = defaultValue; }
    void changeAmount(float change);

    void setOffset(float offset);
    void changeOffset(float change);

    float defaultValue = 0.5;
    float value = defaultValue;
    float offset;
};

class PatchElementInOut : public PatchElement {
  public:
    PatchElementInOut(Output &source, Input &targetIn, float value = 0) {
        this->sourceOut = &source;
        this->targetIn = &targetIn;
        this->value = value;
    }

    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendUpdatePatchInOut;

    // inline float getAmount() { return value; }
    // void setAmount(float value);

    // void changeAmount(float change);
    // void resetAmount() { value = defaultValue; }

    bool remove = false;
    Output *sourceOut;
    Input *targetIn;

  private:
    float value;
    float defaultValue = 0.5;
};

class PatchElementOutOut : public PatchElement {
  public:
    PatchElementOutOut(Output &source, Output &targetOut, float value = 0, float offset = 0) {
        this->sourceOut = &source;
        this->targetOut = &targetOut;
        this->value = value;
        this->offset = offset;
    }

    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendUpdatePatchInOut;

    bool remove = false;
    Output *sourceOut;
    Output *targetOut;

  private:
};

class ID {
  public:
    uint8_t getNewId();

  private:
    uint8_t idCounter = 0;
};

//////////////////////////////// INLINE FUNCTIONS /////////////////////////////////

inline void PatchElement::changeAmount(float change) {
    value = changeFloat(value, change, -1, 1);
}
inline void PatchElement::setAmount(float value) {
    value = testFloat(value, -1, 1);
}
inline void PatchElement::changeOffset(float change) {
    offset = changeFloat(offset, change, -1, 1);
}
inline void PatchElement::setOffset(float offset) {
    offset = testFloat(offset, -1, 1);
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
