/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usb_device.c
 * @version        : v1.0_Cube
 * @brief          : This file implements the USB Device
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

#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.hpp"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_midi.h"
#include "usbd_midi_if.hpp"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;
USBD_HandleTypeDef hUsbDeviceFS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * Init USB device Library, add supported class and start the library
 * @retval None
 */
void MX_USB_DEVICE_Init(void) {
    /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */

    /* USER CODE END USB_DEVICE_Init_PreTreatment */

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_Init HS Failed");
    }
    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_RegisterClass HS Failed");
    }
    if (USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_CDC_RegisterInterface HS Failed");
    }
    if (USBD_Start(&hUsbDeviceHS) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_Start HS Failed");
    }

    /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */
    SCB_InvalidateICache();
    // SCB_InvalidateDCache();

    /* USER CODE END USB_DEVICE_Init_PreTreatment */

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_Init FS Failed");
    }
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_RegisterClass FS Failed");
    }
    if (USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_MIDI_fops_FS) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_MIDI_RegisterInterface FS Failed");
    }
    if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
        PolyError_Handler("ERROR | INIT | USBD_Start FS Failed");
    }

    /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

    /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
