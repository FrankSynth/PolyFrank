#pragma once

#include "dataHelperFunctions.hpp"
#include "debughelper/debughelper.hpp"
#include "math/polyMath.hpp"
#include <cmath>
#include <functional>
#include <list>
#include <string>
#include <vector>

#define MAX_VALUE_16BIT 65535
#define VECTORDEFAULTINITSIZE 5
#define VOICESPERCHIP 4

// only two (binary) or more options (continuous)
enum typeDisplayValue { continuous, binary };
enum typeLinLog { linMap, logMap, antilogMap };

#ifdef POLYCONTROL
extern uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, uint8_t *amount);
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
    void setValueWithoutMapping(int32_t newValue) { value = newValue; }
    void setValueWithoutMapping(uint8_t *newValue) { value = *(int32_t *)newValue; }
    void increase(int32_t amount = 1);
    void decrease(int32_t amount = -1);
    void push();
    void pushAndHold();

    void reset();

    const std::string &getValueAsString();
};

// derived class Poti
class Analog : public DataElement {
  public:
    Analog(const char *name, float min = 0, float max = 1, bool sendOutViaCom = true, typeLinLog mapping = linMap) {

        this->value = 0;
        this->min = min;
        this->max = max;
        this->minMaxDifference = max - min;

        this->mapping = mapping;

        this->name = name;

        this->sendOutViaCom = sendOutViaCom;
    }

    void setValue(int32_t newValue);

    static std::function<uint8_t(uint8_t, uint8_t, float)> sendViaChipCom;

    void changeValue(int32_t change) { setValue(value + change); }
    void setValueWithoutMapping(float newValue) { valueMapped = newValue; }
    void setValueWithoutMapping(uint8_t *newValue) { value = *(float *)newValue; }

    const std::string &getValueAsString();

    std::string valueName;
    int32_t value;
    float valueMapped;
    float min;
    float max;
    float minMaxDifference;

  protected:
    typeLinLog mapping;
};

// derived class Poti
class Digital : public DataElement {
  public:
    Digital(const char *name, int32_t min = 0, int32_t max = 1, bool sendOutViaCom = true,
            const std::vector<std::string> *valueNameList = nullptr) {

        this->value = 0;
        this->min = min;
        this->max = max;
        this->minMaxDifference = max - min;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;

        this->valueNameList = valueNameList;
    }

    // Inputs range must be from 0 -> MAX_VALUE_16BIT
    void setValue(int32_t newValue);

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    void setValueWithoutMapping(int32_t newValue) { value = newValue; }
    void setValueWithoutMapping(uint8_t *newValue) { value = *(int32_t *)newValue; }

    const std::string &getValueAsString();
    const std::vector<std::string> *valueNameList; // custom Name List for different Values

    int32_t value;
    int32_t valueMapped;
    std::string valueName;

    int32_t min;
    int32_t max;
    int32_t minMaxDifference;

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

    const std::string &getName();
    std::vector<PatchElementInOut *> &getPatchesInOut() { return patchesInOut; }
    std::vector<PatchElementOutOut *> &getPatchesOutOut() { return patchesOutOut; }

    uint8_t id;
    uint8_t moduleId;
    uint8_t layerId;

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
    }

    // set next calculatedSample as current sample
    void updateToNextSample() {
        for (int16_t i = 0; i < VOICESPERCHIP; i++) {
            currentSample[i] = nextSample[i];
        }
    }

    float currentSample[VOICESPERCHIP] = {0, 0, 0, 0};
    float nextSample[VOICESPERCHIP] = {0, 0, 0, 0};
};

class PatchElementInOut {
  public:
    PatchElementInOut(Output &source, Input &targetIn, float value = 0) {
        this->sourceOut = &source;
        this->targetIn = &targetIn;
        this->value = value;
    }

    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendUpdatePatchInOut;

    inline float getAmount() { return value; }
    void setAmount(float value);

    void changeAmount(float change);

    bool remove = false;
    Output *sourceOut;
    Input *targetIn;

  private:
    float value;
    float defaultValue;
};

class PatchElementOutOut {
  public:
    PatchElementOutOut(Output &source, Output &targetOut, float value = 0, float offset = 0) {
        this->sourceOut = &source;
        this->targetOut = &targetOut;
        this->value = value;
        this->offset = offset;
    }

    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendUpdatePatchInOut;

    inline float getAmount() { return value; }
    inline float getOffset() { return offset; }
    void setAmount(float value);
    void setOffset(float offset);

    void changeAmount(float change);
    void changeOffset(float change);

    bool remove = false;
    Output *sourceOut;
    Output *targetOut;

  private:
    float value;
    float offset;
    float defaultValue;
};

class ID {
  public:
    uint8_t getNewId();

  private:
    uint8_t idCounter = 0;
};

//////////////////////////////// INLINE FUNCTIONS /////////////////////////////////

inline void PatchElementInOut::changeAmount(float change) {
    value = changeFloat(value, change, -1, 1);
}
inline void PatchElementInOut::setAmount(float value) {
    value = testFloat(value, -1, 1);
}
inline void PatchElementOutOut::changeAmount(float change) {
    value = changeFloat(value, change, -1, 1);
}
inline void PatchElementOutOut::changeOffset(float change) {
    offset = changeFloat(offset, change, -1, 1);
}
inline void PatchElementOutOut::setAmount(float value) {
    value = testFloat(value, -1, 1);
}
inline void PatchElementOutOut::setOffset(float offset) {
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
