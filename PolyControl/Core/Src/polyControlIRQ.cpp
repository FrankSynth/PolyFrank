
#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"

#include "midiInterface/MIDIInterface.h"
#include "usbd_midi_if.hpp"

// USB
extern midi::MidiInterface<midiUSB::COMusb> midiDeviceUSB;
extern midi::MidiInterface<COMdin> midiDeviceDIN;

extern GUI ui;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern ADC_HandleTypeDef hadc3;

extern devManager deviceManager;

// List of all busObjects
extern spiBus spiBusEEPROM;
extern spiBus spiBusPanel;

extern MAX11128 adcA;
extern MAX11128 adcB;

// Buffer for InterChip Com
extern volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];
extern volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];

// Layer
extern Layer layerA;
extern Layer layerB;

extern IS32FL3237 ledDriver[2][2];
extern IS31FL3205 ledDriverControl;

void sendRequestAllUIData();
void receiveFromRenderChip(uint8_t layer, uint8_t chip);
void checkLayerRequests();

// InterChip Com
extern COMinterChip layerCom;
extern COMdin MIDIDinRead;
extern UART_HandleTypeDef huart5;
uint8_t midiDataInBuffer;

extern elapsedMicros timerFramedraw;

void temperature();

void polyControlLoop() { // Here the party starts
    // WFI
    bool enableWFI = false;
    elapsedMillis timerWFI;
    elapsedMillis timerUIData;
    elapsedMillis timerStatusUpdate;

    HAL_UART_Receive_Stream(&huart5);
    initUsageTimer();

    while (1) {

        if (timerStatusUpdate >= 1000) {
            timerStatusUpdate = 0;
            globalSettings.controlStatus.usage.value = ReadAndResetUsageTimer();
        }

        if (timerUIData > 16) { // 120Hz ui data test
            // startUsageTimer();
            timerUIData = 0;
            sendRequestAllUIData();
            LEDRender();
        }
        if (getRenderState() == RENDER_DONE) {
            // startUsageTimer();
            ui.Draw();
        }
        if (enableWFI) {
            __disable_irq();
            stopUsageTimer();
            __DSB();
            __WFI();
            startUsageTimer();
            __enable_irq();
        }
        else {
            if (timerWFI > 5000)
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
    static float adcx = (110.0 - 30.0) / (*(unsigned short *)(0x1FF1E840) - *(unsigned short *)(0x1FF1E820));

    adc_v = HAL_ADC_GetValue(&hadc3);
    globalSettings.controlStatus.temperature.value = adcx * (float)(adc_v - *(unsigned short *)(0x1FF1E820)) + 30;

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

    static bool ErrorMessageSend = false; // block spamming

    if (layerCom.sentRequestUICommand == true) {
        if (ErrorMessageSend == false) {
            println("INFO || COM: Not all data received yet...");
            ErrorMessageSend = true;
        }
        return;
    }
    else if (ErrorMessageSend == true) {
        println("INFO || COM: ...error cleared");
        ErrorMessageSend = false;
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
            HAL_GPIO_WritePin(SPI_CS_Layer_2B_GPIO_Port, SPI_CS_Layer_2B_Pin, state);
        else
            HAL_GPIO_WritePin(SPI_CS_Layer_2A_GPIO_Port, SPI_CS_Layer_2A_Pin, state);
    }
    else {
        if (chip)
            HAL_GPIO_WritePin(SPI_CS_Layer_1B_GPIO_Port, SPI_CS_Layer_1B_Pin, state);
        else
            HAL_GPIO_WritePin(SPI_CS_Layer_1A_GPIO_Port, SPI_CS_Layer_1A_Pin, state);
    }
}

//////////////Callback////////////

void UART_RxISR_8BIT_FIFOEN_Stream(UART_HandleTypeDef *huart) {
    // startUsageTimer();

    uint16_t uhMask = huart->Mask;
    uint16_t uhdata;

    huart->Instance->ICR = 0b1000; // clear overrung error -> missing data is handled in midi

    while (huart->Instance->ISR & (0x01 << 5)) { // empty fifo

        uhdata = (uint16_t)READ_REG(huart->Instance->RDR);
        midiDataInBuffer = (uint8_t)(uhdata & (uint8_t)uhMask);
        MIDIDinRead.push(&midiDataInBuffer, 1);
    }
}

// SPI Callback

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // startUsageTimer();

    // InterChip Com
    if (hspi == layerCom.spi->hspi) {
        layerCom.spi->callTxComplete();
        // close ChipSelectLine

        if (layerA.layerState.value) {
            HAL_GPIO_WritePin(SPI_CS_Layer_1A_GPIO_Port, SPI_CS_Layer_1A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(SPI_CS_Layer_1B_GPIO_Port, SPI_CS_Layer_1B_Pin, GPIO_PIN_SET);
        }
        if (layerB.layerState.value) {
            HAL_GPIO_WritePin(SPI_CS_Layer_2A_GPIO_Port, SPI_CS_Layer_2A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(SPI_CS_Layer_2B_GPIO_Port, SPI_CS_Layer_2B_Pin, GPIO_PIN_SET);
        }
        return;
    }
    else if (hspi == spiBusEEPROM.hspi) {
        spiBusEEPROM.callTxComplete();
    }
    else if (hspi == spiBusPanel.hspi) {

        // reset both ADC

        spiBusPanel.callTxComplete();
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    // startUsageTimer();

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
    // startUsageTimer();

    if (hspi == spiBusPanel.hspi) {

        if (adcA.state == DEVICE_BUSY) {
            adcA.state = DEVICE_READY;
            HAL_GPIO_WritePin(Panel_1_CS_GPIO_Port, Panel_1_CS_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Panel_2_CS_GPIO_Port, Panel_2_CS_Pin, GPIO_PIN_SET);

            FlagHandler::Panel_0_RXTX_Interrupt = true;
        }

        else if (adcB.state == DEVICE_BUSY) {
            adcB.state = DEVICE_READY;
            HAL_GPIO_WritePin(Panel_1_CS_GPIO_Port, Panel_1_CS_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Panel_2_CS_GPIO_Port, Panel_2_CS_Pin, GPIO_PIN_SET);
            FlagHandler::Panel_1_RXTX_Interrupt = true;
        }
        else {
            println("ERROR Communcation ADC TxRx");
        }

        spiBusPanel.callRxComplete();
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == spiBusPanel.hspi) {
        PolyError_Handler("spi error");
        spiBusPanel.callRxComplete();
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // startUsageTimer();

    if (hi2c->Instance == hi2c4.Instance) {
        FlagHandler::ledDriverB_Interrupt = true;
    }
    else if (hi2c->Instance == hi2c3.Instance) {
        FlagHandler::ledDriverA_Interrupt = true;
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    println("ErrorI2c");
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
    println("AbortI2c");
}

// EXTI Callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    // startUsageTimer();

    if (pin & GPIO_PIN_12 || pin & GPIO_PIN_3 || pin & GPIO_PIN_4 || pin & GPIO_PIN_5) { // ioExpander -> encoder
        FlagHandler::Control_Encoder_Interrupt = true;
        FlagHandler::Control_Encoder_Interrupt_Timer = 0;
    }
    if (pin & Panel_1_EOC_Pin) { // EOC
        FlagHandler::Panel_0_EOC_Interrupt = true;
    }

    if (pin & Panel_2_EOC_Pin) { // EOC
        FlagHandler::Panel_1_EOC_Interrupt = true;
    }

    if (pin & Control_INT_Pin) { // Control Touch
        FlagHandler::Control_Touch_Interrupt = true;
    }
    if (pin & Panel_2_INT_Pin) { //  Touch
        FlagHandler::Panel_0_Touch_Interrupt = true;
        // println("panel1Int");
    }
    if (pin & Panel_1_INT_Pin) { //  Touch
        FlagHandler::Panel_1_Touch_Interrupt = true;
        // println("panel2Int");
    }

    if (pin & GPIO_PIN_8) {
        liveData.externalClockTick();
    }

    if (pin & SPI_READY_LAYER_2A_Pin) { // Layer B Chip 0
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

    if (pin & SPI_READY_LAYER_2B_Pin) { // Layer B Chip 1
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

    if (pin & SPI_READY_LAYER_1B_Pin) { // Layer A Chip 1
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

    if (pin & SPI_READY_LAYER_1A_Pin) { // Layer A Chip 0
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
    // startUsageTimer();

    if (htim->Instance == htim3.Instance) {
        liveData.internalClockTick();
    }
    if (htim->Instance == htim4.Instance) {
        FlagHandler::readTemperature = true;
    }
    if (htim->Instance == htim16.Instance) {

        elapsedMicros timTimer;

        if (globalSettings.midiSource.getValue() == 0) { // if midi USB
            midiDeviceUSB.read();
        }
        else { // DIN MIDI
            midiDeviceDIN.read();
        }

        liveData.serviceRoutine();
        checkLayerRequests();
        layerCom.beginSendTransmission();

        // software IRQ
        EXTI->SWIER1 |= 0x01;
    }
}
