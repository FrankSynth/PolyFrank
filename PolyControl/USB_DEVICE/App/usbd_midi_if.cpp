/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_midi_if.c
 * @version        : v1.0_Cube
 * @brief          : Usb device for Virtual Com Port.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi_if.hpp"
#include "midiInterface/MIDIInterface.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief Usb device library.
 * @{
 */

/** @addtogroup USBD_MIDI_IF
 * @{
 */

/** @defgroup USBD_MIDI_IF_Private_TypesDefinitions USBD_MIDI_IF_Private_TypesDefinitions
 * @brief Private types.
 * @{
 */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Private_Defines USBD_MIDI_IF_Private_Defines
 * @brief Private defines.
 * @{
 */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Private_Macros USBD_MIDI_IF_Private_Macros
 * @brief Private macros.
 * @{
 */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Private_Variables USBD_MIDI_IF_Private_Variables
 * @brief Private variables.
 * @{
 */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
volatile uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB MIDI are stored in this buffer   */
volatile uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
midiUSB::COMusb MIDIComRead(MIDI_Transmit_FS);
/* USER CODE END PRIVATE_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Exported_Variables USBD_MIDI_IF_Exported_Variables
 * @brief Public variables.
 * @{
 */

extern USBD_HandleTypeDef hUsbDeviceFS;
// extern midi::MidiInterface<midiUSB::COMusb> mididevice;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Private_FunctionPrototypes USBD_MIDI_IF_Private_FunctionPrototypes
 * @brief Private functions declaration.
 * @{
 */

static int8_t MIDI_Init_FS(void);
static int8_t MIDI_DeInit_FS(void);
static int8_t MIDI_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t MIDI_Receive_FS(uint8_t *pbuf, uint32_t *Len);
static int8_t MIDI_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
 * @}
 */

USBD_MIDI_ItfTypeDef USBD_Interface_MIDI_fops_FS = {MIDI_Init_FS, MIDI_DeInit_FS, MIDI_Control_FS, MIDI_Receive_FS,
                                                    MIDI_TransmitCplt_FS};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the MIDI media low layer over the FS USB IP
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t MIDI_Init_FS(void) {
    /* USER CODE BEGIN 3 */
    /* Set Application Buffers */
    USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, (uint8_t *)UserTxBufferFS, 0);
    USBD_MIDI_SetRxBuffer(&hUsbDeviceFS, (uint8_t *)UserRxBufferFS);
    return (USBD_OK);
    /* USER CODE END 3 */
}

/**
 * @brief  DeInitializes the MIDI media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t MIDI_DeInit_FS(void) {
    /* USER CODE BEGIN 4 */
    return (USBD_OK);
    /* USER CODE END 4 */
}

/**
 * @brief  Manage the MIDI class requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t MIDI_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length) {
    /* USER CODE BEGIN 5 */
    switch (cmd) {
        case MIDI_SEND_ENCAPSULATED_COMMAND: break;

        case MIDI_GET_ENCAPSULATED_RESPONSE: break;

        case MIDI_SET_COMM_FEATURE: break;

        case MIDI_GET_COMM_FEATURE: break;

        case MIDI_CLEAR_COMM_FEATURE:
            break;
            /*******************************************************************************/
            /* Line Coding Structure                                                       */
            /*-----------------------------------------------------------------------------*/
            /* Offset | Field       | Size | Value  | Description                          */
            /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
            /* 4      | bCharFormat |   1  | Number | Stop bits                            */
            /*                                        0 - 1 Stop bit                       */
            /*                                        1 - 1.5 Stop bits                    */
            /*                                        2 - 2 Stop bits                      */
            /* 5      | bParityType |  1   | Number | Parity                               */
            /*                                        0 - None                             */
            /*                                        1 - Odd                              */
            /*                                        2 - Even                             */
            /*                                        3 - Mark                             */
            /*                                        4 - Space                            */
            /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
            /*******************************************************************************/
        case MIDI_SET_LINE_CODING: break;

        case MIDI_GET_LINE_CODING: break;

        case MIDI_SET_CONTROL_LINE_STATE: break;

        case MIDI_SEND_BREAK: break;

        default: break;
    }

    return (USBD_OK);
    /* USER CODE END 5 */
}

/**
 * @brief  Data received over USB OUT endpoint are sent over MIDI interface
 *         through this function.
 *
 *         @note
 *         This function will issue a NAK packet on any OUT packet received on
 *         USB endpoint until exiting this function. If you exit this function
 *         before transfer is complete on MIDI interface (ie. using DMA controller)
 *         it will result in receiving more data while previous ones are still
 *         not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t MIDI_Receive_FS(uint8_t *Buf, uint32_t *Len) {
    /* USER CODE BEGIN 6 */
    USBD_MIDI_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    USBD_MIDI_ReceivePacket(&hUsbDeviceFS);
    // HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    MIDIComRead.push(Buf, *Len);
    // MIDI_Transmit_FS(Buf, *Len);
    return (USBD_OK);
    /* USER CODE END 6 */
}

/**
 * @brief  MIDI_Transmit_FS
 *         Data to send over USB IN endpoint are sent over MIDI interface
 *         through this function.
 *         @note
 *
 *
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t MIDI_Transmit_FS(uint8_t *Buf, uint16_t Len) {
    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 7 */
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (hmidi->TxState != 0) {
        return USBD_BUSY;
    }
    USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
    result = USBD_MIDI_TransmitPacket(&hUsbDeviceFS);
    /* USER CODE END 7 */
    return result;
}

/**
 * @brief  MIDI_TransmitCplt_FS
 *         Data transmited callback
 *
 *         @note
 *         This function is IN transfer complete callback used to inform user that
 *         the submitted Data is successfully sent over USB.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t MIDI_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 13 */
    UNUSED(Buf);
    UNUSED(Len);
    UNUSED(epnum);

    /* USER CODE END 13 */
    return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
