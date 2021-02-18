#pragma once

#include "dataHelperFunctions.hpp"
#include "debughelper/debughelper.hpp"
#include "math/polyMath.hpp"
#include <cmath>
#include <functional>
#include <list>
#include <valarray>
#include <vector>

#ifdef POLYCONTROL
#include <string>
#endif

#define MAX_VALUE_12BIT 3340 // TODO andere rail to rail Opamp.. dann auf größere range setzen
#define MIN_VALUE_12BIT 66   // TODO andere rail to rail Opamp.. dann auf größere range setzen

#define VECTORDEFAULTINITSIZE 5
#define VOICESPERCHIP 4

extern float ROTARYENCODERACELLARATION;

// only two (binary) or more options (continuous)
enum typeDisplayValue { continuous, binary };
enum typeLinLog { linMap, logMap, antilogMap };

#ifdef POLYCONTROL
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount);
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount);
uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
// uint8_t sendUpdatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount, float offset);
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

// Error element
class Error {
  public:
    void setErrorMessage(std::string errorMessage) {

        this->errorMessage = errorMessage;

        errorActive = 1;
    }

    void resetError() {
        errorMessage.clear();
        errorActive = 0;
    }
    int32_t errorActive = 0;  // no error = 0
    std::string errorMessage; // errorMessage as string
  private:
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

#ifdef POLYCONTROL
        if (valueNameList == nullptr)
            valueName = std::to_string(value);
            // if (sendOutViaCom) {
            //     sendSetting(layerId, moduleId, id, value);
            // }
#endif
    }

    int32_t value;
    int32_t valueToggle;  // short push toggles between two values
    int32_t defaultValue; // init value
    int32_t min;
    int32_t max;
    uint8_t disable;

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    const std::vector<std::string> *valueNameList; // custom Name List for different Values

    int32_t getValue();
    void setValue(int32_t newValue);
    void resetValue() { setValue(defaultValue); }

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
        this->value = MIN_VALUE_12BIT;
        this->valueMapped = min;

        this->min = min;
        this->max = max;
        this->minInputValue = MIN_VALUE_12BIT;
        this->maxInputValue = MAX_VALUE_12BIT;

        this->minMaxDifference = max - min;
        this->mapping = mapping;
        this->defaultValueMapped = defaultValue;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;
        this->displayVis = displayVis;

        this->input = input;

        this->inputRange = this->maxInputValue - this->minInputValue;
    }
    Analog(const char *name, float min = 0, float max = 1,
           int32_t minInputValue = MIN_VALUE_12BIT, // contructor for MIDI
           int32_t maxInputValue = MAX_VALUE_12BIT, float defaultValue = 0, bool sendOutViaCom = true,
           typeLinLog mapping = linMap, Input *input = nullptr, bool displayVis = true) {
        this->value = minInputValue;
        this->valueMapped = min;

        this->min = min;
        this->max = max;
        this->minInputValue = minInputValue;
        this->maxInputValue = maxInputValue;

        this->minMaxDifference = max - min;
        this->mapping = mapping;
        this->defaultValueMapped = defaultValue;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;
        this->displayVis = displayVis;

        this->input = input;

        this->inputRange = this->maxInputValue - this->minInputValue;
    }

    void setValue(int32_t newValue);
    void resetValue() {
        defaultValue = reverseMapping(defaultValueMapped); // reverse mapping of the default value
        setValue(defaultValue);
    }

    static std::function<uint8_t(uint8_t, uint8_t, float)> sendViaChipCom;

    int32_t reverseMapping(float newValue);

    inline void changeValue(int32_t change) { setValue(value + change); }
    inline void changeValueWithEncoderAcceleration(bool direction) { // direction 0 -> negative | 1 -> positive
        if (direction == 0) {
            setValue(
                value -
                (testInt(
                    inputRange / 2 * ROTARYENCODERACELLARATION, 1,
                    maxInputValue))); // use Acellaration and test for min step of 1 -> for low resolution analog inputs
        }
        if (direction == 1) {
            setValue(value + (testInt(inputRange / 2 * ROTARYENCODERACELLARATION, 1, maxInputValue)));
        }
    }

    inline void setValueWithoutMapping(float newValue) {
        valueMapped = newValue;
#ifdef POLYCONTROL
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
#endif
    }

    const std::string &getValueAsString();

    std::string valueName;
    int32_t defaultValue = 0;

    int32_t value;
    float defaultValueMapped;

    float valueMapped;
    float min;
    float max;
    float minMaxDifference;
    bool displayVis;
    Input *input;

    int32_t minInputValue;
    int32_t maxInputValue;
    uint32_t inputRange;

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

class PatchElementInOut; // define class to fix no declaration error from Input and Output Class
// class PatchElementOutOut; // define class to fix no declaration error from Input and Output Class

// input patchesInOut
class BasePatch {
  public:
    void clearPatches();

    void addPatchInOut(PatchElementInOut &patch);
    // void addPatchOutOut(PatchElementOutOut &patch);
    void removePatchInOut(PatchElementInOut &patch);
    // void removePatchOutOut(PatchElementOutOut &patch);

    inline const std::string &getName() { return name; };
    inline std::vector<PatchElementInOut *> &getPatchesInOut() { return patchesInOut; }
    // inline std::vector<PatchElementOutOut *> &getPatchesOutOut() { return patchesOutOut; }

    uint8_t id;
    uint8_t moduleId;
    uint8_t layerId;

    uint8_t idGlobal;
    std::string name;

    typeLinLog mapping = linMap;

  protected:
    std::vector<PatchElementInOut *> patchesInOut;
    // std::vector<PatchElementOutOut *> patchesOutOut;
};

class Input : public BasePatch {
  public:
    Input(const char *name, typeLinLog mapping = linMap) {
        this->name = name;
        this->mapping = mapping;
        patchesInOut.reserve(VECTORDEFAULTINITSIZE);
        this->mapping = mapping;
    }

    float currentSample[VOICESPERCHIP] = {0, 0, 0, 0};

    // calculate all inputs with their attached patchesInOut
    void collectCurrentSample();
    typeLinLog mapping;

  protected:
};

class Output : public BasePatch {
  public:
    Output(const char *name) {
        this->name = name;
        patchesInOut.reserve(VECTORDEFAULTINITSIZE);
        // patchesOutOut.reserve(VECTORDEFAULTINITSIZE);

        currentSample = bufferCurrentSample;
        nextSample = bufferNextSample;
    }

    // set next calculatedSample as current sample
    inline void updateToNextSample() {
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
    void setAmountWithoutMapping(float amount);
    void changeAmount(float change);
    void changeAmountEncoderAccelerationMapped(bool direction); // create Mapped

    bool remove = false;
    Output *sourceOut;
    Input *targetIn;
    float amountRaw = 0;
};

// class PatchElementOutOut : public PatchElement {
//   public:
//     PatchElementOutOut(Output &source, Output &targetOut, uint8_t layerId, float amount = 0, float offset = 0) {
//         this->sourceOut = &source;
//         this->targetOut = &targetOut;
//         this->amount = amount;
//         this->offset = offset;
//         this->layerId = layerId;
//     }
//     inline float getOffset() { return offset; }

//     void setAmount(float amount);
//     void setAmountWithoutMapping(float amount);
//     void changeAmount(float change);
//     void setOffset(float offset);
//     void setOffsetWithoutMapping(float offset);
//     void changeOffset(float change);
//     void setAmountAndOffset(float amount, float offset);

//     bool remove = false;
//     Output *sourceOut;
//     Output *targetOut;

//   private:
//     float offset;
// };

class ID {
  public:
    uint8_t getNewId();

  private:
    uint8_t idCounter = 0;
};

class RenderBuffer {
  public:
    RenderBuffer() {
        currentSample = bufferCurrentSample;
        nextSample = bufferNextSample;
    }

    // set next calculatedSample as current sample
    inline void updateToNextSample() {
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

class I2CBuffer {
  public:
    I2CBuffer() {
        currentSample = bufferCurrentSample;
        nextSample = bufferNextSample;
    }

    // set next calculatedSample as current sample
    inline void updateToNextSample() {
        uint16_t *tempPointer = currentSample;
        currentSample = nextSample;
        nextSample = tempPointer;
    }

    uint16_t *currentSample;
    uint16_t *nextSample;

  private:
    uint16_t bufferCurrentSample[VOICESPERCHIP] = {0, 0, 0, 0};
    uint16_t bufferNextSample[VOICESPERCHIP] = {0, 0, 0, 0};
};

//////////////////////////////// INLINE FUNCTIONS /////////////////////////////////

inline void PatchElementInOut::changeAmount(float change) {
    setAmount(amount + change);
#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
#endif
}

// inline void PatchElementOutOut::changeAmount(float change) {
//     setAmount(amount + change);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }

// inline void PatchElementOutOut::changeOffset(float change) {
//     setOffset(offset + change);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }

// inline void PatchElementOutOut::setAmountAndOffset(float amount, float offset) {
//     setAmount(amount);
//     setOffset(offset);
// #ifdef POLYCONTROL
//     sendUpdatePatchOutOut(layerId, sourceOut->idGlobal, targetOut->idGlobal, this->amount, this->offset);
// #endif
// }
inline void BasePatch::clearPatches() {
    patchesInOut.clear();
    // patchesOutOut.clear();
}

inline void BasePatch::addPatchInOut(PatchElementInOut &patch) {
    patchesInOut.push_back(&patch);
}
// inline void BasePatch::addPatchOutOut(PatchElementOutOut &patch) {
//     patchesOutOut.push_back(&patch);
// }

inline void Setting::reset() {
    value = defaultValue;
}

inline const std::string &Analog::getValueAsString() {
    return valueName;
}
inline void Setting::increase(int32_t amount) {
    value = changeInt(value, amount, min, max);
#ifdef POLYCONTROL
    if (valueNameList == nullptr)
        valueName = std::to_string(value);
        // if (sendOutViaCom) {
        //     sendSetting(layerId, moduleId, id, value);
        // }
#endif
}

inline void Setting::decrease(int32_t amount) {
    value = changeInt(value, amount, min, max);
#ifdef POLYCONTROL
    if (valueNameList == nullptr)
        valueName = std::to_string(value);
        // if (sendOutViaCom) {
        //     sendSetting(layerId, moduleId, id, value);
        // }
#endif
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
