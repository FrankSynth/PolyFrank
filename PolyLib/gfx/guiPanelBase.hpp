#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "globalSettings/globalSettings.hpp"
#include "guiActionHandler.hpp"
#include "guiBase.hpp"
#include "img/polybitmap.h"
#include "layer/layer.hpp"
#include "livedata/liveData.hpp"
#include "poly.hpp"
#include "preset/preset.hpp"
#include <functional>
#include <string>

#define FOCUSPANELENTRYS 9
#define FOCUSPANELENTRYSWAVE 6

#define CONFIGPANELENTRYS 5
#define PATCHPANELENTRYS 7
#define PRESETPANELENTRYS 9
#define LIVEPANELENTRYS 3

extern const GUI_FONTINFO *fontSmall;
extern const GUI_FONTINFO *fontMedium;
extern const GUI_FONTINFO *fontBig;

/*Aufbau Data Panel

  - GUIPanelData
    -registriert Panel im ActionHandler
    -legt n Data_PanelElement an

    -> Data_PanelElement[n]
        -registriert Entry im ActionHandler

      - besteht aus m data entrys verschiedener Typen
      - entscheidet was gerendert wird

        -> entrys halten die information jedes Datenfelds
        - zu registrierende functionen
        - pointer zum daten element



*/
void drawDeviceManager(std::string *string, uint16_t rows, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void drawConsole(CircularBuffer<char, 1024>, uint16_t rows, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void drawPresetElemet(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawModuleElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawPatchInOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawDigitalElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawAnalogElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawSettingElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                        uint8_t hugeFont = 0);

void drawWaveFromModule(BaseModule *module, uint16_t x, uint16_t y);

void drawWave(int8_t *renderedWave, uint16_t samples, uint32_t repeats, uint16_t color);
void drawFrame(uint16_t color);
void drawGrid(uint16_t color);

void drawVecWave(vec<2> *renderedWave, uint16_t samples);

void calculateLFOWave(LFO *module, int8_t *waveBuffer, uint16_t samples);
void drawADSR(WaveBuffer &wavebuffer, ADSR *module);

void drawPhaseshaper(WaveBuffer &wavebuffer, Phaseshaper *module);
void drawWaveshaper(WaveBuffer &wavebuffer, Waveshaper *module);

const char *valueToNote(const byte &noteIn);

const char *valueToOctave(const byte &noteIn);

const char *valueToSharp(const byte &noteIn);

const char *tuningToChar(const byte &tuning);

// GUIHeader Box for Panel Selection
class Data_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->height = height;
        this->select = select;
    }
    void Draw();

    void setName(std::string *name);

    void addAnalogEntry(Analog *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{ANALOG, data, nullptr, nullptr, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addAnalogEntry(Analog *data) {

        entrys[numberEntrys] = entryStruct{ANALOG,
                                           data,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,

                                           {std::bind(&Analog::changeValueWithEncoderAcceleration, data, 1), "AMOUNT"},
                                           {std::bind(&Analog::changeValueWithEncoderAcceleration, data, 0), "AMOUNT"},
                                           {std::bind(&Analog::resetValue, data), "RESET"}};

        numberEntrys++;
        visible = 1;
    }

    void addModuleEntry(BaseModule *data, actionHandle functionCW, actionHandle functionCCW,
                        actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{MODULE, nullptr, nullptr, nullptr, nullptr, data, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addModuleEntry(BaseModule *data) {

        entrys[numberEntrys] =
            entryStruct{MODULE, nullptr, nullptr, nullptr, nullptr, data, {nullptr, ""}, {nullptr, ""}, {nullptr, ""}};

        numberEntrys++;
        visible = 1;
    }

    void addDigitalEntry(Digital *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{DIGITAL, nullptr, data, nullptr, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addDigitalEntry(Digital *data) {

        entrys[numberEntrys] = entryStruct{DIGITAL,
                                           nullptr,
                                           data,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           {std::bind(&Digital::nextValue, data), data->getName()},
                                           {std::bind(&Digital::previousValue, data), data->getName()},
                                           {std::bind(&Digital::resetValue, data), "RESET"}};

        numberEntrys++;
        visible = 1;
    }
    void addSettingsEntry(Setting *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{SETTING, nullptr, nullptr, data, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addPatchInputEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                            actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{PATCHINPUT, nullptr, nullptr, nullptr, patch, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addPatchInputEntry(PatchElement *patch) {

        entrys[numberEntrys] =
            entryStruct{PATCHINPUT,
                        nullptr,
                        nullptr,
                        nullptr,
                        patch,
                        nullptr,
                        {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 1), "AMOUNT"},
                        {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 0), "AMOUNT"},
                        {std::bind(&PatchElement::setAmount, patch, 0), "RESET"}};

        numberEntrys++;
        visible = 1;
    }

    void addPatchOutputEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                             actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{PATCHOUTPUT, nullptr, nullptr, nullptr, patch, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addPatchOutputEntry(PatchElement *patch) {

        entrys[numberEntrys] =
            entryStruct{PATCHOUTPUT,
                        nullptr,
                        nullptr,
                        nullptr,
                        patch,
                        nullptr,
                        {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 1), "AMOUNT"},
                        {std::bind(&PatchElement::changeAmountEncoderAccelerationMapped, patch, 0), "AMOUNT"},
                        {std::bind(&PatchElement::setAmount, patch, 0), "RESET"}};

        numberEntrys++;
        visible = 1;
    }
    void addEmptyEntry() {

        entrys[numberEntrys] = entryStruct{EMPTY,   nullptr,       nullptr,       nullptr,      nullptr,
                                           nullptr, {nullptr, ""}, {nullptr, ""}, {nullptr, ""}};

        numberEntrys++;
        visible = 1;
    }

    uint8_t select = 0;
    uint8_t visible = 0;
    uint16_t numberEntrys = 0;

    std::string *panelElementName; // custom Name

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    entryStruct entrys[4];

    uint16_t width;
    uint16_t height;
};

// GUIHeader Box for Panel Selection
class Live_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->height = height;
        this->select = select;
    }
    void Draw();

    void addSettingsEntry(Setting *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{SETTING, nullptr, nullptr, data, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addEmptyEntry() {

        entrys[numberEntrys] = entryStruct{EMPTY,   nullptr,       nullptr,       nullptr,      nullptr,
                                           nullptr, {nullptr, ""}, {nullptr, ""}, {nullptr, ""}};

        numberEntrys++;
        visible = 1;
    }

    uint8_t select = 0;
    uint8_t visible = 0;
    uint16_t numberEntrys = 0;

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    entryStruct entrys[4];

    uint16_t width;
    uint16_t height;
};

// GUIHeader Box for Panel Selection
class Patch_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->entryWidth = width;
        this->entryHeight = height;
    }
    void Draw();
    void addPatchEntry(PatchElement *patch) { this->patch = patch; }

    void addEntry(BasePatch *entry, uint8_t showModuleName = 0) {
        this->showModuleName = showModuleName;
        this->entry = entry;
        active = 1;
    }

    uint8_t patched = 0;
    uint8_t active = 0;
    uint8_t select = 0;
    uint16_t panelAbsX;
    uint8_t showModuleName = 0;

    BasePatch *entry = nullptr;

    PatchElement *patch = nullptr;

  private:
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;
};

class Module_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->entryWidth = width;
        this->entryHeight = height;
        this->select = select;
    }
    void Draw();

    void addEntry(BaseModule *modulePointer) {
        entry = modulePointer;
        active = 1;
    }
    uint8_t patched = 0;
    uint8_t select = 0;
    uint8_t active = 0;
    uint16_t panelAbsX;

    BaseModule *entry;

  private:
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;
};

// PresetPanelElemnt for Presets
class Preset_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->entryWidth = width;
        this->entryHeight = height;
    }
    void Draw();
    void addEntry(presetStruct *entry) {

        this->entry = entry;
        active = 1;
    }

    uint8_t active = 0;
    uint8_t select = 0;

    presetStruct *entry = nullptr;

  private:
    uint16_t panelAbsY;
    uint16_t panelAbsX;

    uint16_t entryWidth;
    uint16_t entryHeight;
};

class MatrixPatch_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->height = height;
    }
    void Draw();

    void addEntry(PatchElement *entry);
    uint16_t select = 0;
    PatchElement *entry;

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    uint16_t width;
    uint16_t height;
    uint16_t visible = 0;
};

class MatrixIn_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->height = height;
    }
    void Draw();

    void addEntry(Analog *entry);
    uint16_t select = 0;
    Analog *entry;

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    uint16_t width;
    uint16_t height;
    uint16_t visible = 0;
};

class MatrixOut_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->height = height;
    }
    void Draw();

    void addEntry(Output *entry);
    uint16_t select = 0;
    Output *entry;

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    uint16_t width;
    uint16_t height;
    uint16_t visible = 0;
};

class MatrixModule_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->height = height;
    }
    void Draw();

    void addEntry(BaseModule *entry);
    uint16_t select = 0;
    BaseModule *entry;

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    uint16_t width;
    uint16_t height;
    uint16_t visible = 0;
};
