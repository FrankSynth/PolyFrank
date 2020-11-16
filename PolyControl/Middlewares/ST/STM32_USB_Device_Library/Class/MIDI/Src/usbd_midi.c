/**
 ******************************************************************************
 * @file    usbd_midi.c
 * @author  MCD Application Team
 * @brief   This file provides the high layer firmware functions to manage the
 *          following functionalities of the USB MIDI Class:
 *           - Initialization and Configuration of high and low layer
 *           - Enumeration as MIDI Device (and enumeration for each implemented memory interface)
 *           - OUT/IN data transfer
 *           - Command IN transfer (class requests management)
 *           - Error management
 *
 *  @verbatim
 *
 *          ===================================================================
 *                                MIDI Class Driver Description
 *          ===================================================================
 *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
 *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
 *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
 *           This driver implements the following aspects of the specification:
 *             - Device descriptor management
 *             - Configuration descriptor management
 *             - Enumeration as MIDI device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
 *             - Requests management (as described in section 6.2 in specification)
 *             - Abstract Control Model compliant
 *             - Union Functional collection (using 1 IN endpoint for control)
 *             - Data interface class
 *
 *           These aspects may be enriched or modified for a specific user application.
 *
 *            This driver doesn't implement the following aspects of the specification
 *            (but it is possible to manage these features with some modifications on this driver):
 *             - Any class-specific aspect relative to communication classes should be managed by user application.
 *             - All communication classes other than PSTN are not managed
 *
 *  @endverbatim
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                      www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */

/** @defgroup USBD_MIDI
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_MIDI_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup USBD_MIDI_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup USBD_MIDI_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_MIDI_Private_FunctionPrototypes
 * @{
 */

static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev);

static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_MIDI_GetDeviceQualifierDescriptor(uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC, USB_DESC_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00,
};

/**
 * @}
 */

/** @defgroup USBD_MIDI_Private_Variables
 * @{
 */

/* MIDI interface class callbacks structure */
USBD_ClassTypeDef USBD_MIDI = {
    USBD_MIDI_Init,
    USBD_MIDI_DeInit,
    USBD_MIDI_Setup,
    NULL, /* EP0_TxSent, */
    USBD_MIDI_EP0_RxReady,
    USBD_MIDI_DataIn,
    USBD_MIDI_DataOut,
    NULL,
    NULL,
    NULL,
    USBD_MIDI_GetHSCfgDesc,
    USBD_MIDI_GetFSCfgDesc,
    USBD_MIDI_GetOtherSpeedCfgDesc,
    USBD_MIDI_GetDeviceQualifierDescriptor,
};

/* USB MIDI device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgHSDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END = {
    9,       // bLength			length of this descriptor = 9 bytes
    2,       // bDescriptorType	CONFIGURATION descriptor type = 2
    0x65, 0, // wTotalLength		length of configuration block =101
             //					incl this descriptor
    2,       // bNumInterfaces    two interfaces
    1,       // bConfigurationValue ID of this configuration
    0,       // iConfiguration	index of string descriptor:unused=0
#ifdef BUSPOWERED
    0x80, // bmAttributes		BUS-powered - no remote wakeup
    49,   // MaxPower 		entry*2 = 98mA
#else
    0xC0, // bmAttributes		SELF-powered - no remote wakeup
    0,    // MaxPower			= 0 (self-powered only)
#endif
    // Standard Audio Class Interface Descriptor
    9, // bLength			length of this descriptor = 9 bytes
    4, // bDescriptorType	INTERFACE descriptor type=4
    0, // bInterfaceNumber	index of this interface
    0, // bAlternativeSetting index of this setting
    0, // bNumEndpoints		= 0 (EP0 only)
    1, // bInterfaceClass	= 0x01 (AUDIO)
    1, // bInterfaceSubClass = 0x01 (AUDIO_CONTROL)
    0, // bInterfaceProtocol = 0 (unused)
    0, // iInterface 		= 0 (unused)
    // Class-specific AudioControl Interface descriptor
    9,    // bLength			length of this descriptor = 9 bytes
    0x24, // bDescriptorTye	CS_INTERFACEdescriptor type = 0x24
    1,    // bDescriptorSubtyte HEADER subtype
    0, 1, // bcdADC			=0x0100 revision of class specific descriptors
    9, 0, // wTotalLength		= 9
    1,    // bInCollection		number of streaming interfaces
    1,    // baInterfaceNr(1)	MIDIStreaming interface no.1
          //					belongs to this Audio Control
    // Standard MIDI Streaming Interface Descriptor
    9, // bLength	        length of this descriptor = 9 bytes
    4, // bDescriptorType	INTERFACE descriptor type = 0x04
    1, // ibInterfaceNumber	index of this interface
    0, // bAlternativeSetting index of this setting
    2, // bNumENdpoints		2 endpoints
    1, // bInterfaceClass	AUDIO
    3, // bInterfaceSubclass MIDISTREAMING
    0, // bInterfaceProtocol = 0 (unused)
    0, // iInterface 		= 0 (unused)
    // Class specific MIDI Streaming Interface Descriptor
    7,          // bLength 			length of this descriptor = 7 bytes
    0x24,       // bDescriptorType	CD_INTERFACE descriptor
    1,          // bDescriptorSubtye	MS_HEADER subtype
    0, 0x01,    // bcdADC			Revision of this class specification 0x0100
    0x41, 0x00, // wTotalLength		total size of class-specific descriptors
    // MIDI IN Jack descriptor (embedded)
    6,    // bLength           length of this descriptor = 6 bytes
    0x24, // bDescriptorType	CS_INTERFACE descriptor
    2,    // bDescriptorSubtype MIDI_IN_JACK subtype
    1,    // bJackType			EMBEDDED
    1,    // bJackID			ID of this jack
    0,    // iJack 			unused
    // MIDI IN Jack descriptor (external)
    6,    // bLength           length of this descriptor = 6 bytes
    0x24, // bDescriptorType	CS_INTERFACE descriptor
    2,    // bDescriptorSubtype MIDI_IN_JACK subtype
    2,    // bJackType			EXTERNAL
    2,    // bJackID			ID of this jack
    0,    // iJack 			unused
    // MIDI OUT Jack descriptor (embedded)
    9,    // bLength           length of this descriptor = 9 bytes
    0x24, // bDescriptorType	CS_INTERFACE descriptor
    3,    // bDescriptorSubtype MIDI_OUT_JACK subtype
    1,    // bJackType			EMBEDDED
    3,    // bJackID			ID of this jack
    1,    // bNrInputPins		= 1
    2,    // BaSourceID(1)		this jack is connected to jack with ID 2
    1,    // BaSourcePin(1)	there connected to pin 1
    0,    // iJack 			unused
    // MIDI OUT Jack descriptor (external)
    9,    // length of this descriptor = 9 bytes
    0x24, // bDescriptorType	CS_INTERFACE descriptor
    3,    // bDescriptorSubtype MIDI_OUT_JACK subtype
    2,    // bJackType			EXTERNAL
    4,    // bJackID			ID of this jack
    1,    // bNrInputPins		= 1
    1,    // BaSourceID(1)		this jack is connected to jack with ID 1
    1,    // BaSourcePin(1)	there connected to pin 1
    0,    // iJack 			unused
    // Standard BULK IN endpoint descriptor
    9,          // bLength           length of this descriptor = 9 bytes
    5,          // bDescriptorType	ENDPOINT descriptor
    0x81,       // bEndpointAddress	IN endpoint no.1
    2,          // bmAttributes		BULK DATA, not shared
    0x40, 0x00, // wMaxPacketSize	64 bytes/packet
    0,          // bInterval 		ignored for Bulk = 0
    0,          // bRefresh 			unused
    0,          // bSynchAddress 	unused
    // Class-specific BULK IN endpoint descriptor
    5,    // bLength			length of this descriptor = 5 bytes
    0x25, // bDescriptorType	CS_ENDPOINT descriptor
    1,    // bDescriptorSubtype MS_GENERAL subtype
    1,    // bNumEmbMIDIJack	= 1 number of embedded MIDI OUT jacks
    3,    // BaAssocJackID(1)	associated with embedded MIDI OUT no.3
    // Standard BULK OUT endpoint descriptor
    9,          // bLength           length of this descriptor = 9 bytes
    5,          // bDescriptorType	ENDPOINT descriptor
    2,          // bEndpointAddress	OUT endpoint no.2
    2,          // bmAttributes		BULK DATA,not shared
    0x40, 0x00, // wMaxPacketSize	64 bytes/packet
    0,          // bInterval 		ignored for Bulk = 0
    0,          // bRefresh 			unused
    0,          // bSynchAddress 	unused
    // Class-specific BULK OUT endpoint descriptor
    5,    // bLength			length of this descriptor = 5 bytes
    0x25, // bDescriptorType	CS_ENDPOINT descriptor
    1,    // bDescriptorSubtype MS_GENERAL subtype
    1,    // bNumEmbMIDIJack	= 1 number of embedded MIDI IN jacks
    1     // baAssocJackID(1)	associated with embedded MIDI IN no.1
};

/* USB MIDI device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgFSDesc[] __ALIGN_END = {
    // configuration descriptor
    0x09,
    0x02,
    0x65,
    0x00,
    0x02,
    0x01,
    0x00,
    0xc0,
    0x32,

    // The Audio Interface Collection
    0x09,
    0x04,
    0x00,
    0x00,
    0x00,
    0x01,
    0x01,
    0x00,
    0x00,
    // Standard AC Interface Descriptor
    0x09,
    0x24,
    0x01,
    0x00,
    0x01,
    0x09,
    0x00,
    0x01,
    0x01,
    // Class-specific AC Interface Descriptor
    0x09,
    0x04,
    0x01,
    0x00,
    0x02,
    0x01,
    0x03,
    0x00,
    0x00,
    // MIDIStreaming Interface Descriptors
    0x07,
    0x24,
    0x01,
    0x00,
    0x01,
    0x41,
    0x00,
    // Class-Specific MS Interface Header Descriptor

    // MIDI IN JACKS
    0x06,
    0x24,
    0x02,
    0x01,
    0x01,
    0x00,
    0x06,
    0x24,
    0x02,
    0x02,
    0x02,
    0x00,

    // MIDI OUT JACKS
    0x09,
    0x24,
    0x03,
    0x01,
    0x03,
    0x01,
    0x02,
    0x01,
    0x00,
    0x09,
    0x24,
    0x03,
    0x02,
    0x06,
    0x01,
    0x01,
    0x01,
    0x00,

    // OUT endpoint descriptor
    0x09,
    0x05,
    MIDI_OUT_EP,
    0x02,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,
    0x05,
    0x25,
    0x01,
    0x01,
    0x01,

    // IN endpoint descriptor
    0x09,
    0x05,
    MIDI_IN_EP,
    0x02,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,
    0x05,
    0x25,
    0x01,
    0x01,
    0x03,
};

__ALIGN_BEGIN static uint8_t USBD_MIDI_OtherSpeedCfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09, /* bLength: Configuation Descriptor size */
    USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION, USB_MIDI_CONFIG_DESC_SIZ, 0x00, 0x02, /* bNumInterfaces: 2 interfaces */
    0x01,                                                                          /* bConfigurationValue: */
    0x04,                                                                          /* iConfiguration: */
    0xC0,                                                                          /* bmAttributes: */
    0x32,                                                                          /* MaxPower 100 mA */

    /*Interface Descriptor */
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00, /* bInterfaceNumber: Number of Interface */
    0x00, /* bAlternateSetting: Alternate setting */
    0x01, /* bNumEndpoints: One endpoints used */
    0x02, /* bInterfaceClass: Communication Interface Class */
    0x02, /* bInterfaceSubClass: Abstract Control Model */
    0x01, /* bInterfaceProtocol: Common AT commands */
    0x00, /* iInterface: */

    /* Header Functional Descriptor */
    0x05, /* bLength: Endpoint Descriptor size */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x00, /* bDescriptorSubtype: Header Func Desc */
    0x10, /* bcdMIDI: spec release number */
    0x01,

    /*Call Management Functional Descriptor*/
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x01, /* bDescriptorSubtype: Call Management Func Desc */
    0x00, /* bmCapabilities: D0+D1 */
    0x01, /* bDataInterface: 1 */

    /*ACM Functional Descriptor*/
    0x04, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x02, /* bDescriptorSubtype: Abstract Control Management desc */
    0x02, /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x06, /* bDescriptorSubtype: Union func desc */
    0x00, /* bMasterInterface: Communication class interface */
    0x01, /* bSlaveInterface0: Data Class Interface */

    /*Endpoint 2 Descriptor*/
    0x07,                                            /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                          /* bDescriptorType: Endpoint */
    MIDI_CMD_EP,                                     /* bEndpointAddress */
    0x03,                                            /* bmAttributes: Interrupt */
    LOBYTE(MIDI_CMD_PACKET_SIZE),                    /* wMaxPacketSize: */
    HIBYTE(MIDI_CMD_PACKET_SIZE), MIDI_FS_BINTERVAL, /* bInterval: */

    /*---------------------------------------------------------------------------*/

    /*Data class interface descriptor*/
    0x09,                    /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
    0x01,                    /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x02,                    /* bNumEndpoints: Two endpoints used */
    0x0A,                    /* bInterfaceClass: MIDI */
    0x00,                    /* bInterfaceSubClass: */
    0x00,                    /* bInterfaceProtocol: */
    0x00,                    /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,                   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
    MIDI_OUT_EP,            /* bEndpointAddress */
    0x02,                   /* bmAttributes: Bulk */
    0x40,                   /* wMaxPacketSize: */
    0x00, 0x00,             /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,                   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
    MIDI_IN_EP,             /* bEndpointAddress */
    0x02,                   /* bmAttributes: Bulk */
    0x40,                   /* wMaxPacketSize: */
    0x00, 0x00              /* bInterval */
};

/**
 * @}
 */

/** @defgroup USBD_MIDI_Private_Functions
 * @{
 */

/**
 * @brief  USBD_MIDI_Init
 *         Initialize the MIDI interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
    UNUSED(cfgidx);
    USBD_MIDI_HandleTypeDef *hmidi;

    hmidi = USBD_malloc(sizeof(USBD_MIDI_HandleTypeDef));

    if (hmidi == NULL) {
        pdev->pClassData = NULL;
        return (uint8_t)USBD_EMEM;
    }

    pdev->pClassData = (void *)hmidi;

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Open EP IN */
        (void)USBD_LL_OpenEP(pdev, MIDI_IN_EP, USBD_EP_TYPE_BULK, 0x40);

        pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 1U;

        /* Open EP OUT */
        (void)USBD_LL_OpenEP(pdev, MIDI_OUT_EP, USBD_EP_TYPE_BULK, MIDI_DATA_HS_OUT_PACKET_SIZE);

        pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 1U;

        /* Set bInterval for MIDI CMD Endpoint */
        pdev->ep_in[MIDI_CMD_EP & 0xFU].bInterval = MIDI_HS_BINTERVAL;
    }
    else {
        /* Open EP IN */
        (void)USBD_LL_OpenEP(pdev, MIDI_IN_EP, USBD_EP_TYPE_BULK, 0x40);

        pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 1U;

        /* Open EP OUT */
        (void)USBD_LL_OpenEP(pdev, MIDI_OUT_EP, USBD_EP_TYPE_BULK, 0x40);

        pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 1U;

        /* Set bInterval for CMD Endpoint */
        pdev->ep_in[MIDI_CMD_EP & 0xFU].bInterval = MIDI_FS_BINTERVAL;
    }

    /* Open Command IN EP */
    (void)USBD_LL_OpenEP(pdev, MIDI_CMD_EP, USBD_EP_TYPE_INTR, MIDI_CMD_PACKET_SIZE);
    pdev->ep_in[MIDI_CMD_EP & 0xFU].is_used = 1U;

    /* Init  physical Interface components */
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->Init();

    /* Init Xfer states */
    hmidi->TxState = 0U;
    hmidi->RxState = 0U;

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->RxBuffer, MIDI_DATA_HS_OUT_PACKET_SIZE);
    }
    else {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->RxBuffer, MIDI_DATA_FS_OUT_PACKET_SIZE);
    }

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_Init
 *         DeInitialize the MIDI layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
    UNUSED(cfgidx);
    uint8_t ret = 0U;

    /* Close EP IN */
    (void)USBD_LL_CloseEP(pdev, MIDI_IN_EP);
    pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 0U;

    /* Close EP OUT */
    (void)USBD_LL_CloseEP(pdev, MIDI_OUT_EP);
    pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 0U;

    /* Close Command IN EP */
    (void)USBD_LL_CloseEP(pdev, MIDI_CMD_EP);
    pdev->ep_in[MIDI_CMD_EP & 0xFU].is_used = 0U;
    pdev->ep_in[MIDI_CMD_EP & 0xFU].bInterval = 0U;

    /* DeInit  physical Interface components */
    if (pdev->pClassData != NULL) {
        ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->DeInit();
        (void)USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }

    return ret;
}

/**
 * @brief  USBD_MIDI_Setup
 *         Handle the MIDI specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
    uint8_t ifalt = 0U;
    uint16_t status_info = 0U;
    USBD_StatusTypeDef ret = USBD_OK;

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
        case USB_REQ_TYPE_CLASS:
            if (req->wLength != 0U) {
                if ((req->bmRequest & 0x80U) != 0U) {
                    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)
                        ->Control(req->bRequest, (uint8_t *)hmidi->data, req->wLength);

                    (void)USBD_CtlSendData(pdev, (uint8_t *)hmidi->data, req->wLength);
                }
                else {
                    hmidi->CmdOpCode = req->bRequest;
                    hmidi->CmdLength = (uint8_t)req->wLength;

                    (void)USBD_CtlPrepareRx(pdev, (uint8_t *)hmidi->data, req->wLength);
                }
            }
            else {
                ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest, (uint8_t *)req, 0U);
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest) {
                case USB_REQ_GET_STATUS:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                        (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
                    }
                    else {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_GET_INTERFACE:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                        (void)USBD_CtlSendData(pdev, &ifalt, 1U);
                    }
                    else {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_SET_INTERFACE:
                    if (pdev->dev_state != USBD_STATE_CONFIGURED) {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_CLEAR_FEATURE: break;

                default:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
    }

    return (uint8_t)ret;
}

/**
 * @brief  USBD_MIDI_DataIn
 *         Data sent on non-control IN endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    USBD_MIDI_HandleTypeDef *hmidi;
    PCD_HandleTypeDef *hpcd = pdev->pData;

    if (pdev->pClassData == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if ((pdev->ep_in[epnum].total_length > 0U) &&
        ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U)) {
        /* Update the packet total length */
        pdev->ep_in[epnum].total_length = 0U;

        /* Send ZLP */
        (void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
    }
    else {
        hmidi->TxState = 0U;
        ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->TransmitCplt(hmidi->TxBuffer, &hmidi->TxLength, epnum);
    }

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_DataOut
 *         Data received on non-control Out endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (pdev->pClassData == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    /* Get the received data length */
    hmidi->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

    /* USB data will be immediately processed, this allow next USB traffic being
    NAKed till the end of the application Xfer */

    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->Receive(hmidi->RxBuffer, &hmidi->RxLength);

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_EP0_RxReady
 *         Handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if ((pdev->pUserData != NULL) && (hmidi->CmdOpCode != 0xFFU)) {
        ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)
            ->Control(hmidi->CmdOpCode, (uint8_t *)hmidi->data, (uint16_t)hmidi->CmdLength);
        hmidi->CmdOpCode = 0xFFU;
    }

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_GetFSCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length) {
    *length = (uint16_t)sizeof(USBD_MIDI_CfgFSDesc);

    return USBD_MIDI_CfgFSDesc;
}

/**
 * @brief  USBD_MIDI_GetHSCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_MIDI_GetHSCfgDesc(uint16_t *length) {
    *length = (uint16_t)sizeof(USBD_MIDI_CfgHSDesc);

    return USBD_MIDI_CfgHSDesc;
}

/**
 * @brief  USBD_MIDI_GetCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length) {
    *length = (uint16_t)sizeof(USBD_MIDI_OtherSpeedCfgDesc);

    return USBD_MIDI_OtherSpeedCfgDesc;
}

/**
 * @brief  DeviceQualifierDescriptor
 *         return Device Qualifier descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
uint8_t *USBD_MIDI_GetDeviceQualifierDescriptor(uint16_t *length) {
    *length = (uint16_t)sizeof(USBD_MIDI_DeviceQualifierDesc);

    return USBD_MIDI_DeviceQualifierDesc;
}

/**
 * @brief  USBD_MIDI_RegisterInterface
 * @param  pdev: device instance
 * @param  fops: CD  Interface callback
 * @retval status
 */
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops) {
    if (fops == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    pdev->pUserData = fops;

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_SetTxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Tx Buffer
 * @retval status
 */
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    hmidi->TxBuffer = pbuff;
    hmidi->TxLength = length;

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_SetRxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Rx Buffer
 * @retval status
 */
uint8_t USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    hmidi->RxBuffer = pbuff;

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_MIDI_TransmitPacket
 *         Transmit packet on IN endpoint
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
    USBD_StatusTypeDef ret = USBD_BUSY;

    if (pdev->pClassData == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    if (hmidi->TxState == 0U) {
        /* Tx Transfer in progress */
        hmidi->TxState = 1U;

        /* Update the packet total length */
        pdev->ep_in[MIDI_IN_EP & 0xFU].total_length = hmidi->TxLength;

        /* Transmit next packet */
        (void)USBD_LL_Transmit(pdev, MIDI_IN_EP, hmidi->TxBuffer, hmidi->TxLength);

        ret = USBD_OK;
    }

    return (uint8_t)ret;
}

/**
 * @brief  USBD_MIDI_ReceivePacket
 *         prepare OUT Endpoint for reception
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_MIDI_ReceivePacket(USBD_HandleTypeDef *pdev) {
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (pdev->pClassData == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->RxBuffer, MIDI_DATA_HS_OUT_PACKET_SIZE);
    }
    else {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->RxBuffer, MIDI_DATA_FS_OUT_PACKET_SIZE);
    }

    return (uint8_t)USBD_OK;
}
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
