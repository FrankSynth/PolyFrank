#include "flagHandler.hpp"
#include "debughelper/debughelper.hpp"

namespace FlagHandler {

#ifdef POLYCONTROL

// bool receiveDMARunning = false;

volatile bool Control_Encoder_Interrupt = false;
std::function<void()> Control_Encoder_ISR = nullptr;
elapsedMicros Control_Encoder_Interrupt_Timer;
volatile bool HID_Initialized = false;

volatile bool Panel_0_EOC_Interrupt = false;
volatile bool Panel_1_EOC_Interrupt = false;
std::function<void()> Panel_0_EOC_ISR = nullptr;
std::function<void()> Panel_1_EOC_ISR = nullptr;
std::function<void()> Panel_nextChannel_ISR = nullptr;

volatile bool Panel_0_RXTX_Interrupt = false;
volatile bool Panel_1_RXTX_Interrupt = false;
std::function<void()> Panel_0_RXTX_ISR = nullptr;
std::function<void()> Panel_1_RXTX_ISR = nullptr;

volatile bool Control_Touch_Interrupt = false;
std::function<void()> Control_Touch_ISR = nullptr;

volatile bool Panel_0_Touch_Interrupt = false;
std::function<void()> Panel_0_Touch_ISR = nullptr;

volatile bool Panel_1_Touch_Interrupt = false;
std::function<void()> Panel_1_Touch_ISR = nullptr;

volatile bool USB_HS_CONNECTED = false;
volatile bool USB_FS_CONNECTED = false;

volatile bool MIDI_USB_TRAFFIC = false;
volatile bool MIDI_DIN_TRAFFIC = false;

volatile bool COM_USB_TRAFFIC = false;

// Display
volatile bool renderingDoneSwitchBuffer = false;

volatile bool readTemperature = false;
std::function<void()> readTemperature_ISR = nullptr;

volatile ledDriverState ledDriverATransmit = DRIVER_IDLE;
volatile ledDriverState ledDriverBTransmit = DRIVER_IDLE;
volatile ledDriverState ledDriverControlTransmit = DRIVER_IDLE;

volatile bool ledDriverA_Interrupt = false;
volatile bool ledDriverB_Interrupt = false;
volatile bool ledDriverControl_Interrupt = false;

volatile bool ledDriverUpdateCurrent = false;

std::function<void()> ledDriverA_ISR[2];
std::function<void()> ledDriverB_ISR[2];
std::function<void()> ledDriverControl_ISR;
std::function<void()> ledDriverUpdateCurrent_ISR;

#elif POLYRENDER

// InterChip receive flags
bool interChipReceive_DMA_Started;
bool interChipReceive_DMA_Finished;
std::function<uint8_t()> interChipReceive_DMA_FinishedFunc;
bool interChipReceive_newDataAvailable;
std::function<uint8_t()> interChipReceive_newDataAvailableFunc;

volatile bool outputCollect = true;
volatile bool outputReady = false;
void (*outputCollectFunc)();

volatile bool renderNewCV;

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
                if (HAL_GPIO_ReadPin(Control_Enc_INT_GPIO_Port, Control_Enc_INT_Pin) == 1) {
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

        if (Panel_0_Touch_Interrupt && (ledDriverATransmit == DRIVER_IDLE)) {
            if (Panel_0_Touch_ISR != nullptr) {
                Panel_0_Touch_ISR();
            }
        }
        if (Panel_1_Touch_Interrupt && (ledDriverBTransmit == DRIVER_IDLE)) {
            if (Panel_1_Touch_ISR != nullptr) {
                Panel_1_Touch_ISR();
            }
        }

        // Panel ADC
        if (Panel_0_EOC_Interrupt && Panel_1_EOC_Interrupt && !Panel_0_RXTX_Interrupt && !Panel_1_RXTX_Interrupt) {
            Panel_0_EOC_Interrupt = 0;
            Panel_1_EOC_Interrupt = 0;

            if (Panel_nextChannel_ISR != nullptr) {
                Panel_nextChannel_ISR();
            }

            if (Panel_0_EOC_ISR != nullptr) {
                Panel_0_EOC_ISR();
            }

            if (Panel_1_EOC_ISR != nullptr) {
                Panel_1_EOC_ISR();
            }
        }

        // Panel ADC
        if (Panel_0_RXTX_Interrupt) {

            if (Panel_0_RXTX_ISR != nullptr) {
                Panel_0_RXTX_ISR();
            }
            Panel_0_RXTX_Interrupt = 0;
        }

        // Panel ADC
        if (Panel_1_RXTX_Interrupt) {

            if (Panel_1_RXTX_ISR != nullptr) {
                Panel_1_RXTX_ISR();
            }
            Panel_1_RXTX_Interrupt = 0;
        }

        if (ledDriverA_Interrupt) { // LED Driver A Statemachine
            ledDriverA_Interrupt = false;

            switch (ledDriverATransmit) {
                case DRIVER_START:
                    ledDriverATransmit = DRIVER_0_TRANSMIT;
                    ledDriverA_ISR[0]();
                    break;
                case DRIVER_0_TRANSMIT:
                    ledDriverATransmit = DRIVER_1_TRANSMIT;
                    ledDriverA_ISR[1]();
                    break;
                case DRIVER_1_TRANSMIT: ledDriverATransmit = DRIVER_IDLE; break;

                default: break;
            }
        }

        if (ledDriverB_Interrupt) { // LED Driver B Statemachine
            ledDriverB_Interrupt = false;

            switch (ledDriverBTransmit) {
                case DRIVER_START:
                    ledDriverBTransmit = DRIVER_0_TRANSMIT;
                    ledDriverB_ISR[0]();
                    break;
                case DRIVER_0_TRANSMIT:
                    ledDriverBTransmit = DRIVER_1_TRANSMIT;
                    ledDriverB_ISR[1]();
                    break;
                case DRIVER_1_TRANSMIT: ledDriverBTransmit = DRIVER_IDLE; break;

                default: break;
            }
        }

        if (ledDriverControl_Interrupt) { // LED Driver B Statemachine
            ledDriverControl_Interrupt = false;

            ledDriverControlTransmit = DRIVER_1_TRANSMIT;
            ledDriverControl_ISR();
            ledDriverControlTransmit = DRIVER_IDLE;
        }

        if (ledDriverUpdateCurrent && ledDriverATransmit == DRIVER_IDLE &&
            ledDriverBTransmit == DRIVER_IDLE) { // Update LED Drive Current
            ledDriverUpdateCurrent = false;

            ledDriverUpdateCurrent_ISR();
        }
    }
    if (readTemperature) {
        readTemperature = false;
        if (readTemperature_ISR != nullptr) {
            readTemperature_ISR();
        }
    }

#elif POLYRENDER

    if (outputCollect) {
        if (outputCollectFunc != nullptr) {
            outputCollectFunc();
        }
        outputCollect = false;
        outputReady = true;
    }

#endif
}
}; // namespace FlagHandler
