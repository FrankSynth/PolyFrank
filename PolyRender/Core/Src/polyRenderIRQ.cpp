#include "polyRender.hpp"
#include "render/renderAudioDef.h"

extern Layer layerA;

extern devManager deviceManager;

// InterChip Com
extern volatile uint8_t interChipDMAInBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];
extern volatile uint8_t interChipDMAOutBuffer[2 * (INTERCHIPBUFFERSIZE + 4)];

extern volatile float interchipLFOBuffer[2];

// CV DACS

extern int8_t audioSendBuffer[UPDATEAUDIOBUFFERSIZE - 1];

extern DUALBU22210 cvDac[2];

extern COMinterChip layerCom;

// AUDIO DAC
extern volatile int32_t saiBuffer[SAIDMABUFFERSIZE * 2 * AUDIOCHANNELS];
extern PCM1690 audioDacA;

void polyRenderLoop() {

    elapsedMillis timerWFI;
    bool enableWFI = false;

    while (true) {

        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);
        FlagHandler::handleFlags();

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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    huart->Instance->ICR = 0b1100;
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    huart->Instance->CR3 &= ~USART_CR3_DMAT; // clear dma register
}

elapsedMicros audiorendertimer = 0;
uint32_t audiorendercounter = 0;
uint32_t audiorendercache = 0;
elapsedMicros cvrendertimer = 0;
uint32_t cvrendercounter = 0;
uint32_t cvrendercache = 0;

// Audio Render Callbacks
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    // HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
    audiorendertimer = 0;
    renderAudio(&(saiBuffer[SAIDMABUFFERSIZE * AUDIOCHANNELS]));

    audiorendercache += audiorendertimer;
    audiorendercounter++;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    // HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
    audiorendertimer = 0;
    renderAudio(saiBuffer);

    // audiorendercache += audiorendertimer;
    // audiorendercounter++;

    // if (audiorendercounter > 10000) {
    //     println(std::to_string((float)audiorendercache / (float)audiorendercounter));
    //     audiorendercounter = 0;
    //     audiorendercache = 0;
    // }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai) {
    PolyError_Handler("SAI error callback");
}

// reception line callback
void HAL_GPIO_EXTI_Callback(uint16_t pin) {

    if (pin == SPI_CS_SOFT_Pin) {
        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);

        if (layerCom.spi->state == BUS_SEND) {
            HAL_GPIO_WritePin(SPI_READY_GPIO_Port, SPI_READY_Pin, GPIO_PIN_RESET);
            layerCom.spi->callTxComplete();
        }

        if (layerCom.spi->state == BUS_RECEIVE) {
            // disable reception line
            HAL_GPIO_WritePin(SPI_READY_GPIO_Port, SPI_READY_Pin, GPIO_PIN_RESET);
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

        // start uart
        huart1.Instance->CR3 |= USART_CR3_DMAT; // set dma register

        // cvrendercache += cvrendertimer;
        // cvrendercounter++;

        // if (cvrendercounter > 10000) {
        //     sendString(std::to_string((float)cvrendercache / (float)cvrendercounter));
        //     cvrendercounter = 0;
        //     cvrendercache = 0;
        // }

        FlagHandler::renderNewCV = false;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // if (hspi == &hspi1) {
    //     cvDac[0].busInterface->callTxComplete();
    // }
    // if (hspi == &hspi2) {
    //     cvDac[1].busInterface->callTxComplete();
    // }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    PolyError_Handler("ERROR | FATAL | SPI Error");
}

void sendDACs() {
    cvDac[0].send();
    cvDac[1].send();
}

// cv rendering timer IRQ
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

    if (htim == &htim15) {
        sendDACs();

        if (FlagHandler::renderNewCV == true) {
            println("CV rendering not done");
        }
        FlagHandler::renderNewCV = true;

        EXTI->SWIER1 |= 0x01;
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
