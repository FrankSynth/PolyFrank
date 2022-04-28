#include "flagHandler.hpp"
#include "debughelper/debughelper.hpp"

namespace FlagHandler {

void initFlagHandler() {
#ifdef POLYCONTROL

    renderingDoneSwitchBuffer = false;

    HID_Initialized = false;
    Panel_0_EOC_Interrupt = false;
    Panel_1_EOC_Interrupt = false;
    Panel_EOC_ISR = nullptr;

    Control_Touch_Interrupt = false;
    Control_Touch_ISR = nullptr;

    Panel_0_Touch_Interrupt = false;
    Panel_0_Touch_ISR = nullptr;

    Panel_1_Touch_Interrupt = false;
    Panel_1_Touch_ISR = nullptr;

    readTemperature = false;

#elif POLYRENDER

    for (uint32_t i = 0; i < 10; i++)
        cvDacStarted[i] = false;

    for (uint32_t i = 0; i < 10; i++)
        cvDacFinished[i] = false;

    for (uint32_t i = 0; i < 3; i++)
        cvDacLastFinished[i] = true;

    renderNewCV = false;
    // cvRendered = false;
    // cvSent = false;
    renderNewCVFunc = nullptr;
    // sendRenderedCVsFunc = nullptr;

#endif
}

#ifdef POLYCONTROL

// bool receiveDMARunning = false;

volatile bool Control_Encoder_Interrupt = false;
std::function<void()> Control_Encoder_ISR = nullptr;
elapsedMicros Control_Encoder_Interrupt_Timer;
volatile bool HID_Initialized = false;

volatile bool Panel_0_EOC_Interrupt = false;
volatile bool Panel_1_EOC_Interrupt = false;
std::function<void()> Panel_EOC_ISR = nullptr;

volatile bool Control_Touch_Interrupt = false;
std::function<void()> Control_Touch_ISR = nullptr;

volatile bool Panel_0_Touch_Interrupt = false;
std::function<void()> Panel_0_Touch_ISR = nullptr;

volatile bool Panel_1_Touch_Interrupt = false;
std::function<void()> Panel_1_Touch_ISR = nullptr;

volatile bool USB_HS_CONNECTED = false;
volatile bool USB_FS_CONNECTED = false;

// Display
volatile bool renderingDoneSwitchBuffer;

volatile bool readTemperature;
std::function<void()> readTemperature_ISR = nullptr;

#elif POLYRENDER

// InterChip receive flags
bool interChipReceive_DMA_Started;
bool interChipReceive_DMA_Finished;
std::function<uint8_t()> interChipReceive_DMA_FinishedFunc;
bool interChipReceive_newDataAvailable;
std::function<uint8_t()> interChipReceive_newDataAvailableFunc;

bool cvDacStarted[10];
bool cvDacFinished[10];
bool cvDacLastFinished[3];

volatile bool outputCollect = true;
volatile bool outputReady = false;
// std::function<void()> outputCollectFunc;
void (*outputCollectFunc)();

volatile bool renderNewCV;
void (*renderNewCVFunc)();

#endif

// handle all interrupts
void handleFlags() {

#ifdef POLYCONTROL

    // HID Flags

    // HID ICs initialized
    if (HID_Initialized == true) {
        // Control Panel Encoder
        if (Control_Encoder_Interrupt) {
            if (Control_Encoder_Interrupt_Timer > 250) {
                if (Control_Encoder_ISR != nullptr) {
                    Control_Encoder_ISR();
                }

                // interrupt cleared?
                if (HAL_GPIO_ReadPin(Encoder_Interrupt_GPIO_Port, Encoder_Interrupt_Pin) == 1) {
                    Control_Encoder_Interrupt = false;
                }
            }
        }
        // Control Panel Touch
        if (Control_Touch_Interrupt) {
            // Control_Touch_Interrupt = 0;
            if (Control_Touch_ISR != nullptr) {
                Control_Touch_ISR();
            }
        }

        // Panel Touch
        if (Panel_0_Touch_Interrupt) {
            // Panel_0_Touch_Interrupt = 0;
            if (Panel_0_Touch_ISR != nullptr) {
                Panel_0_Touch_ISR();
            }
        }
        if (Panel_1_Touch_Interrupt) {
            // Panel_1_Touch_Interrupt = 0;
            if (Panel_1_Touch_ISR != nullptr) {
                Panel_1_Touch_ISR();
            }
        }

        // Panel ADC
        if (Panel_0_EOC_Interrupt && Panel_1_EOC_Interrupt) {
            Panel_0_EOC_Interrupt = 0;
            Panel_1_EOC_Interrupt = 0;

            if (Panel_EOC_ISR != nullptr) {
                Panel_EOC_ISR();
            }
        }
    }
    if (readTemperature) {
        readTemperature = false;
        if (readTemperature_ISR != nullptr) {
            readTemperature_ISR();
        }
    }

#elif POLYRENDER

    // if (renderNewCV) {
    //     renderNewCV = false;
    //     renderNewCVFunc();
    // }

    if (outputCollect) {
        // outputReady = false;

        if (outputCollectFunc != nullptr) {
            outputCollectFunc();
        }
        outputCollect = false;
        outputReady = true;
    }

#endif
}

}; // namespace FlagHandler
