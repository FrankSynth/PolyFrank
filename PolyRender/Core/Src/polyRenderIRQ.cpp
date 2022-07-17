#include "polyRender.hpp"
#include "render/renderAudioDef.h"

extern Layer layerA;

extern devManager deviceManager;

// InterChip Com
extern volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];
extern volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];

// CV DACS
extern volatile uint16_t cvDacDMABuffer[ALLDACS][4];

extern int8_t audioSendBuffer[UPDATEAUDIOBUFFERSIZE - 1];

extern COMinterChip layerCom;

extern MCP4728 cvDac[];

// AUDIO DAC
extern volatile int32_t saiBuffer[SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS];
extern PCM1690 audioDacA;

void polyRenderLoop() {
    while (true) {
        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);
        FlagHandler::handleFlags();
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {

    if (hi2c->Instance == hi2c1.Instance) {

        if (FlagHandler::cvDacStarted[0]) {
            // microsecondsDelay(rand);

            cvDac[1].setLatchPin();
            FlagHandler::cvDacStarted[0] = false;
            FlagHandler::cvDacFinished[0] = true;

            FlagHandler::cvDacStarted[1] = true;
            cvDac[1].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[1]) {
            // microsecondsDelay(rand);

            cvDac[2].setLatchPin();
            FlagHandler::cvDacStarted[1] = false;
            FlagHandler::cvDacFinished[1] = true;

            FlagHandler::cvDacStarted[2] = true;
            cvDac[2].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[2]) {
            cvDac[3].setLatchPin();
            FlagHandler::cvDacStarted[2] = false;
            FlagHandler::cvDacFinished[2] = true;

            FlagHandler::cvDacStarted[3] = true;
            cvDac[3].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[3]) {
            FlagHandler::cvDacFinished[3] = true;

            FlagHandler::cvDacLastFinished[0] = true;
        }
    }

    else if (hi2c->Instance == hi2c2.Instance) {
        if (FlagHandler::cvDacStarted[4]) {
            cvDac[5].setLatchPin();
            FlagHandler::cvDacStarted[4] = false;
            FlagHandler::cvDacFinished[4] = true;

            FlagHandler::cvDacStarted[5] = true;
            cvDac[5].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[5]) {
            cvDac[6].setLatchPin();
            FlagHandler::cvDacStarted[5] = false;
            FlagHandler::cvDacFinished[5] = true;

            FlagHandler::cvDacStarted[6] = true;
            cvDac[6].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[6]) {
            FlagHandler::cvDacLastFinished[1] = true;
        }
    }
    else if (hi2c->Instance == hi2c3.Instance) {

        if (FlagHandler::cvDacStarted[7]) {

            cvDac[8].setLatchPin();
            FlagHandler::cvDacStarted[7] = false;
            FlagHandler::cvDacFinished[7] = true;

            FlagHandler::cvDacStarted[8] = true;
            cvDac[8].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[8]) {
            // microsecondsDelay(rand);

            cvDac[9].setLatchPin();
            FlagHandler::cvDacStarted[8] = false;
            FlagHandler::cvDacFinished[8] = true;

            FlagHandler::cvDacStarted[9] = true;
            cvDac[9].fastUpdate();
        }
        else if (FlagHandler::cvDacStarted[9]) {
            FlagHandler::cvDacFinished[9] = true;

            FlagHandler::cvDacLastFinished[2] = true;
        }
    }

    if (FlagHandler::cvDacFinished[0] && FlagHandler::cvDacFinished[4] && FlagHandler::cvDacFinished[7])
        cvDac[0].resetLatchPin();

    if (FlagHandler::cvDacFinished[1] && FlagHandler::cvDacFinished[5] && FlagHandler::cvDacFinished[8])
        cvDac[1].resetLatchPin();

    if (FlagHandler::cvDacFinished[2] && FlagHandler::cvDacFinished[6] && FlagHandler::cvDacFinished[9])
        cvDac[2].resetLatchPin();

    if (FlagHandler::cvDacFinished[3])
        cvDac[3].resetLatchPin();
}

elapsedMicros audiorendertimer = 0;
uint32_t audiorendercounter = 0;
uint32_t audiorendercache = 0;
elapsedMicros cvrendertimer = 0;
uint32_t cvrendercounter = 0;
uint32_t cvrendercache = 0;

// Audio Render Callbacks
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
    audiorendertimer = 0;
    renderAudio(&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));

    audiorendercache += audiorendertimer;
    audiorendercounter++;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
    audiorendertimer = 0;
    renderAudio(saiBuffer);

    audiorendercache += audiorendertimer;
    audiorendercounter++;

    if (audiorendercounter > 10000) {
        println(std::to_string((float)audiorendercache / (float)audiorendercounter));
        audiorendercounter = 0;
        audiorendercache = 0;
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai) {
    PolyError_Handler("SAI error callback");
}

// reception line callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);

    if (pin == SPI_CS_fromControl_Pin) {

        if (layerCom.spi->state == BUS_SEND) {
            HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_RESET);
            layerCom.spi->callTxComplete();
        }

        if (layerCom.spi->state == BUS_RECEIVE) {
            // disable reception line
            HAL_GPIO_WritePin(Layer_Ready_GPIO_Port, Layer_Ready_Pin, GPIO_PIN_RESET);
            layerCom.spi->stopDMA();
            layerCom.spi->callRxComplete();
            layerCom.decodeCurrentInBuffer();
        }
        else if (layerCom.spi->state == BUS_READY && layerCom.state == COM_READY) {
            if (layerCom.beginReceiveTransmission() != BUS_OK)
                PolyError_Handler("ERROR | FATAL | receive bus occuppied");
        }
    }

    if (pin == GPIO_PIN_0) {
        EXTI->PR1 |= 0x01;
        cvrendertimer = 0;
        renderCVs();
        cvrendercache += cvrendertimer;
        cvrendercounter++;

        if (cvrendercounter > 10000) {
            sendString(std::to_string((float)cvrendercache / (float)cvrendercounter));
            cvrendercounter = 0;
            cvrendercache = 0;
        }
        FlagHandler::renderNewCV = false;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    PolyError_Handler("ERROR | FATAL | SPI Error");
}

void sendDACs() {
    FlagHandler::cvDacLastFinished[0] = false;
    FlagHandler::cvDacLastFinished[1] = false;
    FlagHandler::cvDacLastFinished[2] = false;

    for (int i = 0; i < 10; i++)
        FlagHandler::cvDacFinished[0] = false;

    // just those 4 will set all as they are shared
    cvDac[0].setLatchPin();

    // out DacB and DacC gets automatially triggered by flags when transmission is done
    FlagHandler::cvDacStarted[0] = true;
    cvDac[0].fastUpdate();
    FlagHandler::cvDacStarted[4] = true;
    cvDac[4].fastUpdate();
    FlagHandler::cvDacStarted[7] = true;
    cvDac[7].fastUpdate();
}

// cv rendering timer IRQ
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

    if (htim == &htim15) {
        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
        if (FlagHandler::cvDacLastFinished[2] == false) {
            PolyError_Handler("polyRender | timerCallback | cvDacLastFinished[3] false");
            println("CV send not done");
        }

        if (FlagHandler::renderNewCV == true) {
            println("CV rendering not done");
        }
        FlagHandler::renderNewCV = true;

        EXTI->SWIER1 |= 0x01;

        sendDACs();
    }
}

uint8_t sendString(const std::string &message) {
    return layerCom.sendString(message);
}
uint8_t sendString(std::string &&message) {
    return layerCom.sendString(message);
}
uint8_t sendString(const char *message) {
    return layerCom.sendString(message);
}
uint8_t sendOutput(uint8_t modulID, uint8_t settingID, vec<VOICESPERCHIP> &amount) {
    return layerCom.sendOutput(modulID, settingID, amount);
}

uint8_t sendRenderbuffer(uint8_t modulID, uint8_t settingID, vec<VOICESPERCHIP> &amount) {
    return layerCom.sendRenderbuffer(modulID, settingID, amount);
}

void outputCollect() {

    for (Output *o : layerA.outputs) {
        sendOutput(o->moduleId, o->id, o->currentSample); // send only first Voice
    }
    for (BaseModule *m : layerA.modules) {
        for (RenderBuffer *r : m->renderBuffer) {
            if (r->sendOutViaCom)
                sendRenderbuffer(m->id, r->id, r->currentSample); // send all voices
        }
    }

    if (layerA.chipID == 0) {
        renderAudioUI(audioSendBuffer);
        layerCom.sendAudioBuffer(audioSendBuffer);
    }
}
