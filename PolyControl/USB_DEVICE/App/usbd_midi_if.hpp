/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_midi_if.h
 * @version        : v1.0_Cube
 * @brief          : Header for usbd_midi_if.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_MIDI_IF_H__
#define __USBD_MIDI_IF_H__

#include "com/com.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"

/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief For Usb device.
 * @{
 */

/** @defgroup USBD_MIDI_IF USBD_MIDI_IF
 * @brief Usb VCP device module
 * @{
 */

/** @defgroup USBD_MIDI_IF_Exported_Defines USBD_MIDI_IF_Exported_Defines
 * @brief Defines.
 * @{
 */
/* USER CODE BEGIN EXPORTED_DEFINES */
/* Define size for the receive and transmit buffer over MIDI */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE 2048
#define APP_TX_DATA_SIZE 2048

/* USER CODE END EXPORTED_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Exported_Types USBD_MIDI_IF_Exported_Types
 * @brief Types.
 * @{
 */

/* USER CODE BEGIN EXPORTED_TYPES */

/* USER CODE END EXPORTED_TYPES */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Exported_Macros USBD_MIDI_IF_Exported_Macros
 * @brief Aliases.
 * @{
 */

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Exported_Variables USBD_MIDI_IF_Exported_Variables
 * @brief Public variables.
 * @{
 */

/** MIDI Interface callback. */
extern USBD_MIDI_ItfTypeDef USBD_Interface_MIDI_fops_HS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
 * @}
 */

/** @defgroup USBD_MIDI_IF_Exported_FunctionsPrototype USBD_MIDI_IF_Exported_FunctionsPrototype
 * @brief Public functions declaration.
 * @{
 */

uint8_t MIDI_Transmit_HS(uint8_t *Buf, uint16_t Len);

/* USER CODE BEGIN EXPORTED_FUNCTIONS */

/* USER CODE END EXPORTED_FUNCTIONS */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

extern midiUSB::COMusb MIDIComRead;

#endif /* __USBD_MIDI_IF_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
