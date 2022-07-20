
#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"

#include "midiInterface/MIDIInterface.h"
#include "usbd_midi_if.hpp"

// GUI
extern midi::MidiInterface<midiUSB::COMusb> mididevice;
extern GUI ui;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern ADC_HandleTypeDef hadc3;

extern devManager deviceManager;

// List of all busObjects
extern spiBus spiBusEEPROM;
extern spiBus spiBusPanel;

// Buffer for InterChip Com
extern volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];
extern volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];

// USB
extern midi::MidiInterface<midiUSB::COMusb> mididevice;

// Layer
extern Layer layerA;
extern Layer layerB;

void sendRequestAllUIData();
void receiveFromRenderChip(uint8_t layer, uint8_t chip);
void checkLayerRequests();

// InterChip Com
extern COMinterChip layerCom;

void temperature();

void polyControlLoop() { // Here the party starts

    elapsedMicros timer;
    elapsedMillis timerWFI;

    bool enableWFI = false;

    while (1) {

        // checkLayerRequests();
        // __disable_irq();
        if (getRenderState() == RENDER_DONE) {
            // timer = 0;
            ui.Draw();
            // println(timer);
            renderLED();
            sendRequestAllUIData();
        }
        // __enable_irq();
        if (enableWFI) {
            __disable_irq();
            __DSB();
            __WFI();
            __enable_irq();
        }
        else {
            if (timerWFI > 60000)
                enableWFI = true;
        }
    }
}

void checkLayerRequests() {

    if (!layerCom.sentRequestUICommand)
        return;

    if (layerCom.singleChipRequested) {
        if (layerCom.chipState[layerCom.receiveLayer][layerCom.receiveChip] == CHIP_DATAREADY || layerCom.requestSize) {
            receiveFromRenderChip(layerCom.receiveLayer, layerCom.receiveChip);
        }
    }
    else {
        for (uint32_t layer = 0; layer < 2; layer++)
            for (uint32_t chip = 0; chip < 2; chip++)
                if (allLayers[layer]->layerState.value) {
                    if (layerCom.requestState[layer][chip] == RQ_REQUESTDATA) {
                        layerCom.sendRequestUIData(layer, chip);
                        return;
                    }
                }
    }
}

//////////////LAYER SPECIFIC HANDLING////////////

uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, int32_t amount) {
    return layerCom.sendSetting(layerId, moduleId, settingsId, amount);
}
uint8_t sendSetting(uint8_t layerId, uint8_t moduleId, uint8_t settingsId, float amount) {
    return layerCom.sendSetting(layerId, moduleId, settingsId, amount);
}
uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    return layerCom.sendCreatePatchInOut(layerId, outputId, inputId, amount);
}
uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount) {
    return layerCom.sendUpdatePatchInOut(layerId, outputId, inputId, amount);
}
uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId) {
    return layerCom.sendDeletePatchInOut(layerId, outputId, inputId);
}
uint8_t sendDeleteAllPatches(uint8_t layerId) {
    return layerCom.sendDeleteAllPatches(layerId);
}

//////////////TEMPERATURE////////////

void temperature() {

    static unsigned int adc_v;
    static double adcx;

    adc_v = HAL_ADC_GetValue(&hadc3);
    adcx = (110.0 - 30.0) / (*(unsigned short *)(0x1FF1E840) - *(unsigned short *)(0x1FF1E820));
    globalSettings.temperature = (uint32_t)round(adcx * (adc_v - *(unsigned short *)(0x1FF1E820)) + 30);

    HAL_ADC_Start(&hadc3);
}

//////////////MIDI////////////
void midiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    liveData.keyPressed(channel, note, velocity);
}
void midiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    liveData.keyReleased(channel, note);
}
void midiControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    liveData.controlChange(channel, cc, value);
}
void midiPitchBend(uint8_t channel, int value) {
    liveData.controlChange(channel, midi::PitchBend, value);
}

void midiAfterTouch(uint8_t channel, byte value) {
    liveData.controlChange(channel, midi::AfterTouchChannel, value);
}
void midiClock() {
    liveData.midiClockTick();
}
void midiStart() {
    liveData.receivedStart();
}
void midiStop() {
    liveData.receivedStop();
}
void midiContinue() {
    liveData.receivedContinue();
}
void midiReset() {
    liveData.receivedReset();
}
void receivedSPP(unsigned int spp) {
    liveData.receivedMidiSongPosition(spp);
}

/**
 * @brief retreive data from other chips
 *
 * @param layer layer to retreive
 * @param chip chip to retreive
 */
void receiveFromRenderChip(uint8_t layer, uint8_t chip) {
    layerCom.beginReceiveTransmission(layer, chip);
}

void sendRequestAllUIData() {
    if (layerCom.sentRequestUICommand == true) {
        println("not all data received yet");
        return;
    }
    __disable_irq();

    for (int i = 0; i < 2; i++)
        for (int v = 0; v < 2; v++)
            if (allLayers[i]->layerState.value) {
                layerCom.requestState[i][v] = RQ_REQUESTDATA;
            }
    layerCom.sentRequestUICommand = true;
    __enable_irq();
}

void setCSLine(uint8_t layer, uint8_t chip, GPIO_PinState state) {
    if (layer) {
        if (chip)
            HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, state);
        else
            HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, state);
    }
    else {
        if (chip)
            HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, state);
        else
            HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, state);
    }
}

//////////////Callback////////////
// SPI Callback

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // InterChip Com
    if (hspi == layerCom.spi->hspi) {
        layerCom.spi->callTxComplete();
        // close ChipSelectLine

        if (layerA.layerState.value) {
            HAL_GPIO_WritePin(Layer_1_CS_1_GPIO_Port, Layer_1_CS_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_1_CS_2_GPIO_Port, Layer_1_CS_2_Pin, GPIO_PIN_SET);
        }
        if (layerB.layerState.value) {
            HAL_GPIO_WritePin(Layer_2_CS_1_GPIO_Port, Layer_2_CS_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Layer_2_CS_2_GPIO_Port, Layer_2_CS_2_Pin, GPIO_PIN_SET);
        }
    }
    else if (hspi == spiBusEEPROM.hspi) {
        spiBusEEPROM.callTxComplete();
    }
    else if (hspi == spiBusPanel.hspi) {
        spiBusPanel.callTxComplete();
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com
    if (hspi == layerCom.spi->hspi) {
        if (layerCom.requestSize) {
            layerCom.spi->callRxComplete();
        }
        else {
            layerCom.chipState[layerCom.receiveLayer][layerCom.receiveChip] = CHIP_DATASENT;
            layerCom.requestState[layerCom.receiveLayer][layerCom.receiveChip] = RQ_READY;
            layerCom.singleChipRequested = false;

            if (layerCom.sentRequestUICommand && layerCom.requestState[0][0] == RQ_READY &&
                layerCom.requestState[0][1] == RQ_READY && layerCom.requestState[1][0] == RQ_READY &&
                layerCom.requestState[1][1] == RQ_READY) {
                layerCom.sentRequestUICommand = false;
            }

            setCSLine(layerCom.receiveLayer, layerCom.receiveChip, GPIO_PIN_SET);

            layerCom.decodeCurrentInBuffer();
            layerCom.spi->callRxComplete();
        }
    }
    else if (hspi == spiBusEEPROM.hspi) {
        spiBusEEPROM.callRxComplete();
    }
    else if (hspi == spiBusPanel.hspi) {
        spiBusPanel.callRxComplete();
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == spiBusPanel.hspi) {
        spiBusPanel.callRxComplete();
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == spiBusPanel.hspi) {
        PolyError_Handler("spi error");
        spiBusPanel.callRxComplete();
    }
}

// EXTI Callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    if (pin & GPIO_PIN_12) { // ioExpander -> encoder
        FlagHandler::Control_Encoder_Interrupt = true;
        FlagHandler::Control_Encoder_Interrupt_Timer = 0;
    }
    if (pin & GPIO_PIN_11) { // EOC
        if (!HAL_GPIO_ReadPin(Panel_1_EOC_GPIO_Port, Panel_1_EOC_Pin)) {
            FlagHandler::Panel_0_EOC_Interrupt = true;
            FlagHandler::Panel_1_EOC_Interrupt = true;
        }
    }
    if (pin & GPIO_PIN_14) { // Control Touch
        FlagHandler::Control_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_2) { //  Touch
        FlagHandler::Panel_0_Touch_Interrupt = true;
    }
    if (pin & GPIO_PIN_5) { //  Touch
        FlagHandler::Panel_1_Touch_Interrupt = true;
    }

    if (pin & GPIO_PIN_3) { // Layer B Chip 0
        if (layerCom.chipState[1][0] == CHIP_DATASENT) {
            layerCom.chipState[1][0] = CHIP_READY;
            layerCom.chipStateTimeout[1][0] = 0;
        }
        else if (layerCom.chipState[1][0] == CHIP_WAITFORDATA) {
            layerCom.chipState[1][0] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[1][0] = 0;
        }
        else if (layerCom.chipState[1][0] == CHIP_NOTINIT) {
            layerCom.chipState[1][0] = CHIP_READY;
        }
    }

    if (pin & GPIO_PIN_4) { // Layer B Chip 1
        if (layerCom.chipState[1][1] == CHIP_DATASENT) {
            layerCom.chipState[1][1] = CHIP_READY;
            layerCom.chipStateTimeout[1][1] = 0;
        }
        else if (layerCom.chipState[1][1] == CHIP_WAITFORDATA) {
            layerCom.chipState[1][1] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[1][1] = 0;
        }
        else if (layerCom.chipState[1][1] == CHIP_NOTINIT) {
            layerCom.chipState[1][1] = CHIP_READY;
        }
    }

    if (pin & GPIO_PIN_7) { // Layer A Chip 1
        if (layerCom.chipState[0][1] == CHIP_DATASENT) {
            layerCom.chipState[0][1] = CHIP_READY;
            layerCom.chipStateTimeout[0][1] = 0;
        }
        else if (layerCom.chipState[0][1] == CHIP_WAITFORDATA) {
            layerCom.chipState[0][1] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[0][1] = 0;
        }
        else if (layerCom.chipState[0][1] == CHIP_NOTINIT) {
            layerCom.chipState[0][1] = CHIP_READY;
        }
    }

    if (pin & GPIO_PIN_6) { // Layer A Chip 0
        if (layerCom.chipState[0][0] == CHIP_DATASENT) {
            layerCom.chipState[0][0] = CHIP_READY;
            layerCom.chipStateTimeout[0][0] = 0;
        }
        else if (layerCom.chipState[0][0] == CHIP_WAITFORDATA) {
            layerCom.chipState[0][0] = CHIP_DATAREADY;
            layerCom.chipStateTimeout[0][0] = 0;
        }
        else if (layerCom.chipState[0][0] == CHIP_NOTINIT) {
            layerCom.chipState[0][0] = CHIP_READY;
        }
    }

    // software IRQ
    if (pin == GPIO_PIN_0) {
        EXTI->PR1 |= 0x01;
        FlagHandler::handleFlags();
    }
}

// USB Connect detection
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
    /* Prevent unused argument(s) compilation warning */
    if (hpcd->Instance == hpcd_USB_OTG_HS.Instance) { // HS Connected
        FlagHandler::USB_FS_CONNECTED = true;
    }

    if (hpcd->Instance == hpcd_USB_OTG_HS.Instance) { // FS Connected
        FlagHandler::USB_HS_CONNECTED = true;
    }
}

// TIM Callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) { // timer interrupts

    if (htim->Instance == htim5.Instance) {
        liveData.internalClockTick();
    }
    if (htim->Instance == htim4.Instance) {
        FlagHandler::readTemperature = true;
    }
    if (htim->Instance == htim16.Instance) {

        elapsedMicros timTimer;

        mididevice.read();
        liveData.serviceRoutine();
        checkLayerRequests();
        layerCom.beginSendTransmission();

        uint32_t timerVal = timTimer;

        // if (getRenderState() == RENDER_DONE) {
        //     // timer = 0;
        //     ui.Draw();
        //     // println("ui draw:", timer);
        //     renderLED();
        //     sendRequestAllUIData();
        // }

        // software IRQ
        EXTI->SWIER1 |= 0x01;
    }
}
