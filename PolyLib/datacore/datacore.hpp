#pragma once

#include "dataHelperFunctions.hpp"
#include "datadef.h"
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

extern float ROTARYENCODERACCELERATION;
// extern uint32_t LayerRenBufferSw;

// only two (binary) or more options (continuous)
enum typeDisplayValue { continuous, binary };
enum typeLinLog { linMap, logMap, antilogMap };

#ifdef POLYCONTROL
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount);
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount);
uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
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

    void setValueChangedCallback(void (*fptr)()) { valueChangedCallback = fptr; }

  protected:
    void (*valueChangedCallback)() = nullptr;
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
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, value);
        }
#endif
    }

    int32_t value;
    int32_t valueToggle;  // short push toggles between two values
    int32_t defaultValue; // init value
    int32_t min;
    int32_t max;
    uint8_t disable;
    uint8_t useAcceleration = 0;

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    const std::vector<std::string> *valueNameList; // custom Name List for different Values

    int32_t getValue();
    void setValue(int32_t newValue);
    void resetValue() { setValue(defaultValue); }

    inline void setValueWithoutMapping(int32_t newValue) {
        value = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
    }
    inline void setValueWithoutMapping(uint8_t *newValue) {
        value = *(int32_t *)newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
    }
    void increase(int32_t amount = 1);
    void decrease(int32_t amount = -1);
    void push();
    void pushAndHold();

    void reset();

    const std::string &getValueAsString();
};

/**
 * @brief a renderbuffer similar to output modules, used where no visible output module with patchability is used
 *
 */
class RenderBuffer {
  public:
    RenderBuffer() {
        // currentSample = sample;
        // nextSample = &sample[1];
    }

    void updateToNextSample() {
        vec<VOICESPERCHIP> temp = currentSample;
        currentSample = nextSample;
        nextSample = temp;
    }

    uint8_t id;
    uint8_t layerId;
    uint8_t moduleId;

    // vec<VOICESPERCHIP> sample[2];
    vec<VOICESPERCHIP> currentSample;
    vec<VOICESPERCHIP> nextSample;

    operator float *() { return currentSample; }
    operator const float *() const { return currentSample; }

    // returns from currentSample
    float &operator[](int i) { return (currentSample)[i]; }
    const float &operator[](int i) const { return (currentSample)[i]; }

    operator vec<VOICESPERCHIP> &() { return (currentSample); }
    operator const vec<VOICESPERCHIP> &() const { return (currentSample); }

    // assigns to nextSample
    template <typename T> vec<VOICESPERCHIP> &operator=(const T &other) { return (nextSample) = other; }

    template <typename T> vec<VOICESPERCHIP> operator+(const T &other) const { return currentSample + other; }
    template <typename T> vec<VOICESPERCHIP> operator-(const T &other) const { return currentSample - other; }
    template <typename T> vec<VOICESPERCHIP> operator*(const T &other) const { return currentSample * other; }
    template <typename T> vec<VOICESPERCHIP> operator/(const T &other) const { return currentSample / other; }
};

class Input;

// derived class Poti
class Analog : public DataElement {
  public:
    Analog(const char *name, float min = 0, float max = 1, float defaultValue = 0, bool sendOutViaCom = true,
           typeLinLog mapping = linMap, Input *input = nullptr, bool displayVis = true) {

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

        this->value = reverseMapping(defaultValue);
        this->valueMapped = defaultValue;
    }

    Analog(const char *name, float min = 0, float max = 1,
           int32_t minInputValue = MIN_VALUE_12BIT, // contructor for MIDI
           int32_t maxInputValue = MAX_VALUE_12BIT, float defaultValue = 0, bool sendOutViaCom = true,
           typeLinLog mapping = linMap, Input *input = nullptr, bool displayVis = true) {

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

        this->value = reverseMapping(defaultValue);
        this->valueMapped = defaultValue;
    }

    void setValue(int32_t newValue);
    void resetValue() {
        defaultValue = reverseMapping(defaultValueMapped); // reverse mapping of the default value
        setValue(defaultValue);
    }

    static std::function<uint8_t(uint8_t, uint8_t, float)> sendViaChipCom;

    int32_t reverseMapping(float newValue);

    inline void changeValue(int32_t change) { setValue(value + change); }

    // FIXME calc mit ROTARYENCODERACCELERATION float oder int?
    inline void changeValueWithEncoderAcceleration(bool direction) { // direction 0 -> negative | 1 -> positive
        if (direction == 0) {
            setValue(
                value -
                (testInt(
                    (float)inputRange / 2.0f * ROTARYENCODERACCELERATION, 1,
                    maxInputValue))); // use Acellaration and test for min step of 1 -> for low resolution analog inputs
        }
        if (direction == 1) {
            setValue(value + (testInt((float)inputRange / 2.0f * ROTARYENCODERACCELERATION, 1, maxInputValue)));
        }
    }

    inline void setValueWithoutMapping(float newValue) {
        valueMapped = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
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

    operator float &() { return valueMapped; }
    operator const float &() const { return valueMapped; }

  protected:
    typeLinLog mapping;
};

// derived class Poti
class Digital : public DataElement {
  public:
    Digital(const char *name, int32_t min = 0, int32_t max = 1, int32_t defaultValue = 0, bool sendOutViaCom = true,
            const std::vector<const char *> *valueNameList = nullptr, Input *input = nullptr, bool displayVis = true) {

        this->value = MIN_VALUE_12BIT;

        this->min = min;
        this->max = max;
        this->minMaxDifference = max - min;
        this->defaultValue = defaultValue;
        this->valueMapped = defaultValue;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;

        this->displayVis = displayVis;

        this->valueNameList = valueNameList;

        this->input = input;
    }

    // Inputs range must be from 0 -> MAX_VALUE_12BIT
    void setValue(int32_t newValue);
    void nextValue();
    void nextValueLoop();

    void previousValue();
    inline void resetValue() { setValueWithoutMapping(defaultValue); }

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    inline void setValueWithoutMapping(int32_t newValue) {
        valueMapped = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
#ifdef POLYCONTROL
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
#endif
    }
    // inline void setValueWithoutMapping(uint8_t *newValue) {
    //     valueMapped = *(int32_t *)newValue;

    //     if (valueChangedCallback != nullptr)
    //         valueChangedCallback();
    // }

    const std::string &getValueAsString();
    const std::vector<const char *> *valueNameList; // custom Name List for different Values

    int32_t defaultValue = 0;

    int32_t value;
    int32_t valueMapped;
    std::string valueName;

    int32_t min;
    int32_t max;
    int32_t minMaxDifference;

    bool displayVis;

    Input *input;

    operator int32_t &() { return valueMapped; }
    operator const int32_t &() const { return valueMapped; }

    explicit operator float() { return (float)valueMapped; }
    explicit operator float() const { return (float)valueMapped; }

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

class PatchElement; // forward declaration necessary

// input patchesInOut
class BasePatch {
  public:
    void clearPatches();

    void addPatchInOut(PatchElement &patch);
    // void addPatchOutOut(PatchElementOutOut &patch);
    void removePatchInOut(PatchElement &patch);
    // void removePatchOutOut(PatchElementOutOut &patch);
    bool findPatchInOut(uint8_t output, uint8_t input);

    inline const std::string &getName() { return name; };
    inline std::vector<PatchElement *> &getPatchesInOut() { return patchesInOut; }

    uint8_t id;
    uint8_t moduleId;
    uint8_t layerId;
    uint8_t visible;
    uint8_t idGlobal;
    std::string name;
    std::string shortName;

    typeLinLog mapping = linMap;

    std::vector<PatchElement *> patchesInOut;
};

class Input : public BasePatch {
  public:
    Input(const char *name, const char *shortName = nullptr, RenderBuffer *renderBuffer = nullptr,
          typeLinLog mapping = linMap, uint8_t visible = 1) {
        this->name = name;

        if (shortName == nullptr) {

            this->shortName = name;
        }
        else {
            this->shortName = shortName;
        }
        this->mapping = mapping;
        patchesInOut.reserve(VECTORDEFAULTINITSIZE);
        this->visible = visible;

        this->renderBuffer = renderBuffer;
    }

    vec<VOICESPERCHIP> sample;

    RenderBuffer *renderBuffer;
    float &operator[](int i) { return sample[i]; }
    const float &operator[](int i) const { return sample[i]; }

    operator vec<VOICESPERCHIP> &() { return sample; }
    operator const vec<VOICESPERCHIP> &() const { return sample; }

    template <typename T> vec<VOICESPERCHIP> operator+(const T &other) const { return sample + other; }
    template <typename T> vec<VOICESPERCHIP> operator-(const T &other) const { return sample - other; }
    template <typename T> vec<VOICESPERCHIP> operator*(const T &other) const { return sample * other; }
    template <typename T> vec<VOICESPERCHIP> operator/(const T &other) const { return sample / other; }
    template <typename T> vec<VOICESPERCHIP> &operator=(const T &other) { return sample = other; }

    // calculate all inputs with their attached patchesInOut
    void collectCurrentSample();
    typeLinLog mapping;

    uint8_t LEDPortID;
    uint8_t LEDPinID;
};

class Output : public BasePatch {
  public:
    Output(const char *name, const char *shortName = nullptr, uint8_t visible = 1) {
        this->name = name;

        if (shortName == nullptr) {

            this->shortName = name;
        }
        else {
            this->shortName = shortName;
        }

        patchesInOut.reserve(VECTORDEFAULTINITSIZE);

        this->visible = visible;
    }

    void updateToNextSample() {
        vec<VOICESPERCHIP> temp = currentSample;
        currentSample = nextSample;
        nextSample = temp;
    }
    vec<VOICESPERCHIP> currentSample;
    vec<VOICESPERCHIP> nextSample;

    operator float *() { return currentSample; }
    operator const float *() const { return currentSample; }

    // returns from currentSample
    float &operator[](int i) { return (currentSample)[i]; }
    const float &operator[](int i) const { return (currentSample)[i]; }

    operator vec<VOICESPERCHIP> &() { return (currentSample); }
    operator const vec<VOICESPERCHIP> &() const { return (currentSample); }

    // assigns to nextSample
    template <typename T> vec<VOICESPERCHIP> &operator=(const T &other) { return (nextSample) = other; }

    template <typename T> vec<VOICESPERCHIP> operator+(const T &other) const { return currentSample + other; }
    template <typename T> vec<VOICESPERCHIP> operator-(const T &other) const { return currentSample - other; }
    template <typename T> vec<VOICESPERCHIP> operator*(const T &other) const { return currentSample * other; }
    template <typename T> vec<VOICESPERCHIP> operator/(const T &other) const { return currentSample / other; }

    uint8_t LEDPortID = 0xFF;
    uint8_t LEDPinID = 0xFF;
};

class PatchElement {
  public:
    inline float getAmount() { return amount; }

    uint8_t layerId;
    PatchElement(Output &source, Input &targetIn, uint8_t layerId) {
        this->sourceOut = &source;
        this->targetIn = &targetIn;
        this->layerId = layerId;
    }

    void setAmount(float amount);
    void setAmountWithoutMapping(float amount);
    void changeAmount(float change);
    void changeAmountEncoderAccelerationMapped(bool direction); // create Mapped

    float offset;
    float amount;

    bool remove = false;
    Output *sourceOut;
    Input *targetIn;
    float amountRaw = 0;
};

class ID {
  public:
    uint8_t getNewId();

  private:
    uint8_t idCounter = 0;
};

class I2CBuffer {
  public:
    I2CBuffer() {
        // currentSample = bufferCurrentSample;
        // nextSample = bufferNextSample;
    }

    // set next calculatedSample as current sample
    // inline void updateToNextSample() {
    //     uint16_t *tempPointer = currentSample;
    //     currentSample = nextSample;
    //     nextSample = tempPointer;
    // }

    uint16_t currentSample[VOICESPERCHIP];
    // uint16_t *nextSample;

    //   private:
    //     uint16_t bufferCurrentSample[VOICESPERCHIP] = {0, 0, 0, 0};
    //     uint16_t bufferNextSample[VOICESPERCHIP] = {0, 0, 0, 0};
};

//////////////////////////////// INLINE FUNCTIONS /////////////////////////////////

inline void PatchElement::changeAmount(float change) {
    setAmount(amount + change);
#ifdef POLYCONTROL
    sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
#endif
}

inline void BasePatch::clearPatches() {
    patchesInOut.clear();
}

inline void BasePatch::addPatchInOut(PatchElement &patch) {
    patchesInOut.push_back(&patch);
}

inline void Setting::reset() {
    value = defaultValue;
    if (valueChangedCallback != nullptr)
        valueChangedCallback();
}

inline const std::string &Analog::getValueAsString() {
    return valueName;
}
inline void Setting::increase(int32_t amount) {
    if (useAcceleration) {
        value = changeInt(value,
                          std::clamp((int32_t)((max - min) / 2 * ROTARYENCODERACCELERATION), (int32_t)1, (int32_t)max),
                          min, max);
    }
    else {
        value = changeInt(value, amount, min, max);
    }
#ifdef POLYCONTROL
    if (valueNameList == nullptr)
        valueName = std::to_string(value);
        // if (sendOutViaCom) {
        //     sendSetting(layerId, moduleId, id, value);
        // }
#endif
    if (valueChangedCallback != nullptr)
        valueChangedCallback();
}

inline void Setting::decrease(int32_t amount) {
    if (useAcceleration) {
        value = changeInt(value,
                          -std::clamp((int32_t)((max - min) / 2 * ROTARYENCODERACCELERATION), (int32_t)1, (int32_t)max),
                          min, max);
    }
    else {
        value = changeInt(value, amount, min, max);
    }
#ifdef POLYCONTROL
    if (valueNameList == nullptr)
        valueName = std::to_string(value);
        // if (sendOutViaCom) {
        //     sendSetting(layerId, moduleId, id, value);
        // }
#endif
    if (valueChangedCallback != nullptr)
        valueChangedCallback();
}

inline void Setting::push() {
    int32_t temp = value;
    value = valueToggle;
    valueToggle = temp;
    if (valueChangedCallback != nullptr)
        valueChangedCallback();
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
