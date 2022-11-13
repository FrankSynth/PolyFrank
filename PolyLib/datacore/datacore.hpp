#pragma once

#include "dataHelperFunctions.hpp"
#include "datadef.h"
#include "debughelper/debughelper.hpp"
#include "math/polyMath.hpp"
#include <cmath>
#include <functional>
#include <list>
#include <string>
#include <valarray>
#include <vector>

extern float ROTARYENCODERACCELERATION;

enum typeLinLog { linMap, logMap };

class Input;
class RenderBuffer;

#ifdef POLYCONTROL

uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount);
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount);
uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
#endif

class DataElement {
  public:
    DataElement() {}
    ~DataElement() {}

    void setValueChangedCallback(std::function<void()> fptr) { valueChangedCallback = fptr; }

    uint8_t id;
    uint8_t layerId;
    uint8_t moduleId;

    std::function<void()> valueChangedCallback = nullptr;

#ifdef POLYCONTROL

    inline const char *getName() { return name; }

    // Marker
    bool quickview = false;
    bool presetLock = false;
    bool storeable = false;
    uint8_t storeID = 0xFF; // 0xff == NOT SET
    bool displayVis = false;
    uint8_t sendOutViaCom;
    const char *name;
    std::string valueName; // value as string
#endif
};

#ifdef POLYCONTROL

// Settings element for PolyControl
class Setting : public DataElement {
  public:
    Setting(const char *name, int32_t value = 0, int32_t min = 0, int32_t max = 1,
            const std::vector<const char *> *valueNameList = nullptr, bool storeable = true, bool displayVis = true) {

        this->value = value;
        this->defaultValue = value;
        this->min = min;
        this->max = max;

        this->name = name;
        this->valueNameList = valueNameList;

        this->storeable = storeable;
        this->displayVis = displayVis;

        if (valueNameList == nullptr)
            valueName = std::to_string(value);
    }

    // Data Functions

    inline void setValue(int32_t newValue) {
        value = std::clamp(newValue, min, max);
        presetLock = 0;

        if (valueChangedCallback != nullptr)
            valueChangedCallback();
        if (valueNameList == nullptr)
            valueName = std::to_string(value);
    }

    inline int32_t getValue() { return value; }
    inline void resetValue() { setValue(defaultValue); }

    inline void increase(int32_t amount = 1) {
        if (useAcceleration) {
            setValue(changeInt(
                value, std::clamp((int32_t)((max - min) / 2 * ROTARYENCODERACCELERATION), (int32_t)1, (int32_t)max),
                min, max));
        }
        else {
            setValue(changeInt(value, amount, min, max));
        }
    }

    inline void decrease(int32_t amount = -1) {
        if (useAcceleration) {
            setValue(changeInt(
                value, -std::clamp((int32_t)((max - min) / 2 * ROTARYENCODERACCELERATION), (int32_t)1, (int32_t)max),
                min, max));
        }
        else {
            setValue(changeInt(value, amount, min, max));
        }
    }

    inline const std::string &getValueAsString() {
        if (valueNameList == nullptr) {
            return valueName;
        }
        else {
            if ((int32_t)valueNameList->size() == (max - min + 1)) {
                valueName = (*valueNameList)[value - min];
                return valueName;
            }
            else {
                PolyError_Handler("ERROR | Configuration | Setting -> NameList wrong lenght");
                return valueName;
            }
        }
    }

    // Data
    int32_t value;
    int32_t defaultValue; // init value
    int32_t min;
    int32_t max;

    // GUI | HID
    uint8_t disable;
    bool useAcceleration = false;

    const std::vector<const char *> *valueNameList; // custom Name List for different Values
};

#endif

// derived class Analog
class Analog : public DataElement {
  public:
    Analog(const char *name, float min = 0, float max = 1, float defaultValue = 0, bool sendOutViaCom = true,
           typeLinLog mapping = linMap, Input *input = nullptr, bool displayVis = true, bool storeable = true) {

        this->min = min;
        this->max = max;
        this->valueMapped = defaultValue;
        this->input = input;

#ifdef POLYCONTROL
        this->minInputValue = MIN_VALUE_12BIT;
        this->maxInputValue = MAX_VALUE_12BIT;

        this->outputRange = max - min;
        this->mapping = mapping;
        this->defaultValueMapped = defaultValue;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;
        this->displayVis = displayVis;

        this->inputRange = this->maxInputValue - this->minInputValue;

        this->value = reverseMapping(defaultValue);
        this->valueMapped = defaultValue; // need to be set again!
        this->storeable = storeable;
#endif
    }

    Analog(const char *name, float min = 0, float max = 1, int32_t minInputValue = MIN_VALUE_12BIT,
           int32_t maxInputValue = MAX_VALUE_12BIT, float defaultValue = 0, bool sendOutViaCom = true,
           typeLinLog mapping = linMap, Input *input = nullptr, bool displayVis = true, bool storeable = true) {

        this->min = min;
        this->max = max;
        this->valueMapped = defaultValue;
        this->input = input;
        this->defaultValueMapped = defaultValue;

#ifdef POLYCONTROL

        this->minInputValue = minInputValue;
        this->maxInputValue = maxInputValue;

        this->outputRange = max - min;

        this->mapping = mapping;

        this->name = name;
        this->sendOutViaCom = sendOutViaCom;
        this->displayVis = displayVis;

        this->inputRange = this->maxInputValue - this->minInputValue;

        this->value = reverseMapping(defaultValue);
        this->storeable = storeable;

#endif
    }

#ifdef POLYCONTROL

    void setValue(int32_t newValue);
    void setValueInversePoti(int32_t newValue);
    void resetValue() {
        defaultValue = reverseMapping(defaultValueMapped); // reverse mapping of the default value
        setValue(defaultValue);
        presetLock = 0;
    }

    void setNewRange(float min, float max) {
        int32_t rawValue = reverseMapping(valueMapped);

        this->min = min;
        this->max = max;
        this->outputRange = max - min;

        setValue(rawValue);
    }

    static std::function<uint8_t(uint8_t, uint8_t, float)> sendViaChipCom;

    int32_t reverseMapping(float newValue);

    inline void changeValue(int32_t change) { setValue(value + change); }

    inline void changeValueWithEncoderAcceleration(bool direction) { // direction 0 -> negative | 1 -> positive
        if (direction == 0) {
            setValue(value - (testInt((float)inputRange / 2.0f * ROTARYENCODERACCELERATION, 1,
                                      maxInputValue))); // use Acellaration and test for min step of 1 -> for low
                                                        // resolution analog inputs
        }
        if (direction == 1) {
            setValue(value + (testInt((float)inputRange / 2.0f * ROTARYENCODERACCELERATION, 1, maxInputValue)));
        }
    }

    inline void setValueWithoutMapping(float newValue) {
        valueMapped = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
    }

    inline const std::string &getValueAsString() { return valueName; }
    std::string valueName;

#endif

#ifdef POLYRENDER

    void resetValue() {
        valueMapped = defaultValueMapped; // reverse mapping of the default value
    }

    inline void setValueWithoutMapping(float newValue) {
        valueMapped = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
    }
#endif

    // DATA
    float valueMapped;
    float min;
    float max;
    Input *input = nullptr;
    float defaultValueMapped;

// POLYCONTROL ONLY
#ifdef POLYCONTROL

    int32_t value;
    int32_t defaultValue = 0;
    float outputRange;

    int32_t minInputValue;
    int32_t maxInputValue;
    uint32_t inputRange;

    typeLinLog mapping;
#endif

    operator float &() { return valueMapped; }
    operator const float &() const { return valueMapped; }
};

// derived class Poti
class Digital : public DataElement {
  public:
    Digital(const char *name, int32_t min = 0, int32_t max = 1, int32_t defaultValue = 0, bool sendOutViaCom = true,
            const std::vector<const char *> *valueNameList = nullptr, Input *input = nullptr, bool displayVis = true,
            bool storeable = true) {

        this->valueMapped = defaultValue;
        this->input = input;
        this->defaultValue = defaultValue;
        this->min = min;
        this->max = max;
#ifdef POLYCONTROL

        this->value = MIN_VALUE_12BIT;

        this->outputRange = max - min;

        this->name = name;
        this->valueNameList = valueNameList;
        this->displayVis = displayVis;
        this->storeable = storeable;

        this->sendOutViaCom = sendOutViaCom;

#endif
    }
#ifdef POLYCONTROL

    void setValue(int32_t newValue);
    inline void resetValue() { setValueWithoutMapping(defaultValue); }

    void setValueRange(int32_t newValue, int32_t min, int32_t max);
    void nextValue();
    void nextValueLoop();

    void previousValue();

    static std::function<uint8_t(uint8_t, uint8_t, int32_t)> sendViaChipCom;

    inline void setValueWithoutMapping(int32_t newValue) {
        valueMapped = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
        valueName = std::to_string(valueMapped);
        if (sendOutViaCom) {
            sendSetting(layerId, moduleId, id, valueMapped);
        }
    }

    std::vector<uint8_t> LEDPortID;
    std::vector<uint8_t> LEDPinID;

    void configureNumberLEDs(uint8_t numLEDs) {
        for (uint32_t i = 0; i < numLEDs; i++) {

            LEDPortID.push_back(0xff);
            LEDPinID.push_back(0xff);
        }
    }

    int32_t value;
    std::string valueName;
    int32_t outputRange;

    typeLinLog mapping;

    const std::string &getValueAsString();
    const std::vector<const char *> *valueNameList; // custom Name List for different Values

#endif

#ifdef POLYRENDER

    inline void resetValue() { valueMapped = defaultValue; }

    inline void setValueWithoutMapping(int32_t newValue) {
        valueMapped = newValue;
        if (valueChangedCallback != nullptr)
            valueChangedCallback();
    }

#endif
    Input *input;

    int32_t valueMapped;
    int32_t min;
    int32_t max;
    int32_t defaultValue = 0;

    operator int32_t &() { return valueMapped; }
    operator const int32_t &() const { return valueMapped; }

    explicit operator float() { return (float)valueMapped; }
    explicit operator float() const { return (float)valueMapped; }
};

///////////////////RENDER//////////////////

/**
 * @brief a renderbuffer similar to output modules, used where no visible output module with patchability is
 * used
 *
 */
class RenderBuffer {
  public:
    RenderBuffer(bool sendOutViaCom = true) { this->sendOutViaCom = sendOutViaCom; }

    uint8_t id;
    uint8_t layerId;
    uint8_t moduleId;

    uint8_t LEDPortID = 0xff;
    uint8_t LEDPinID = 0xff;

    bool sendOutViaCom;

    vec<VOICESPERCHIP> currentSample;

    operator float *() { return currentSample; }
    operator const float *() const { return currentSample; }

    // returns from currentSample
    float &operator[](int i) { return (currentSample)[i]; }
    const float &operator[](int i) const { return (currentSample)[i]; }

    operator vec<VOICESPERCHIP> &() { return (currentSample); }
    operator const vec<VOICESPERCHIP> &() const { return (currentSample); }

    // assigns to nextSample
    template <typename T> vec<VOICESPERCHIP> &operator=(const T &other) { return currentSample = other; }

    template <typename T> vec<VOICESPERCHIP> operator+(const T &other) const { return currentSample + other; }
    template <typename T> vec<VOICESPERCHIP> operator-(const T &other) const { return currentSample - other; }
    template <typename T> vec<VOICESPERCHIP> operator*(const T &other) const { return currentSample * other; }
    template <typename T> vec<VOICESPERCHIP> operator/(const T &other) const { return currentSample / other; }
};

///////////////////SYSTEM//////////////////

// Error element
class Error {
  public:
    void setErrorMessage(std::string &errorMessage) {

        this->errorMessage = errorMessage;

        errorActive = 1;
    }
    void setErrorMessage(std::string &&errorMessage) {

        this->errorMessage = errorMessage;

        errorActive = 1;
    }
    void setErrorMessage(const char *errorMessage) {

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
class Status {
  public:
    Status(const char *name, const char *unitName = nullptr, bool displayVis = true) {
        this->unitName = unitName;
        this->name = name;
        this->displayVis = displayVis;
    }

    void appendValueAsString(std::string &buffer, bool withName = true, bool withUnit = true, bool withReturn = true) {
        if (withName) {
            buffer += name;
            buffer += " ";
        }

        floatToString2(buffer, value);

        if (withUnit) {
            buffer += " ";
            buffer += unitName;
        }

        if (withReturn) {
            buffer += "\n\r";
        }
    }

    float value;
    const char *name;     // custom Name List for different Values
    const char *unitName; // custom Name List for different Values
    bool displayVis = false;
};

///////////////////PATCHES//////////////////

class PatchElement; // forward declaration necessary

class BasePatch {
  public:
    inline void clearPatches() { patchesInOut.clear(); }
    inline void addPatchInOut(PatchElement &patch) { patchesInOut.push_back(&patch); }

    void removePatchInOut(PatchElement &patch);
    bool findPatchInOut(uint8_t output, uint8_t input);

    inline const char *getName() { return name; };
    inline const char *getShortName() { return shortName; };
    inline std::vector<PatchElement *> &getPatchesInOut() { return patchesInOut; }

    uint8_t id;
    uint8_t moduleId;
    uint8_t layerId;
    uint8_t visible;
    uint8_t idGlobal;
    const char *name;
    const char *shortName;

    typeLinLog mapping = linMap;

    std::vector<PatchElement *> patchesInOut;

    uint8_t LEDPortID = 0xFF;
    uint8_t LEDPinID = 0xFF;

    uint8_t LEDPortID2 = 0xFF; // for mixer double use LED
    uint8_t LEDPinID2 = 0xFF;  // for mixer double use LED
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

    vec<VOICESPERCHIP> currentSample;

    operator float *() { return currentSample; }
    operator const float *() const { return currentSample; }

    // returns from currentSample
    float &operator[](int i) { return (currentSample)[i]; }
    const float &operator[](int i) const { return (currentSample)[i]; }

    operator vec<VOICESPERCHIP> &() { return (currentSample); }
    operator const vec<VOICESPERCHIP> &() const { return (currentSample); }

    // assigns to nextSample
    template <typename T> vec<VOICESPERCHIP> &operator=(const T &other) { return (currentSample) = other; }

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

#ifdef POLYCONTROL
    void changeAmountEncoderAccelerated(bool direction); // create Mapped
    inline void changeAmount(float change) {
        setAmount(amount + change);
        sendUpdatePatchInOut(layerId, sourceOut->idGlobal, targetIn->idGlobal, this->amount);
    }
#endif

    // float offset;
    float amount;

    bool remove = false;
    Output *sourceOut;
    Input *targetIn;
};

class ID {
  public:
    inline uint8_t getNewId() { return idCounter++; }

  private:
    uint8_t idCounter = 0;
};
