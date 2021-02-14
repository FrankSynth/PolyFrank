#include "debughelper.hpp"

#if DEBUG

elapsedMicros USBHSTIMEOUT;

void printViaSTLink(const char *arg) {
    std::string str;
    str.append(arg);

#ifdef POLYCONTROL
    if (FlagHandler::USB_HS_CONNECTED) {

        USBHSTIMEOUT = 0;
        while (CDC_Transmit_HS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 5000) {
                FlagHandler::USB_HS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
#endif
}

void printViaSTLink(const unsigned char *arg) {
    std::string str;
    str.append((const char *)arg);

#ifdef POLYCONTROL
    if (FlagHandler::USB_HS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_HS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 5000) {
                FlagHandler::USB_HS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
#endif
}

void printViaSTLink(char *arg) {
    std::string str;
    str.append(arg);

#ifdef POLYCONTROL
    if (FlagHandler::USB_HS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_HS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 5000) {
                FlagHandler::USB_HS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
#endif
}

void printViaSTLink(unsigned char *arg) {
    std::string str;
    str.append((char *)arg);

#ifdef POLYCONTROL
    if (FlagHandler::USB_HS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_HS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 5000) {
                FlagHandler::USB_HS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
#endif
}

void printViaSTLink(const std::string &arg) {

#ifdef POLYCONTROL
    if (FlagHandler::USB_HS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_HS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 5000) {
                FlagHandler::USB_HS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < arg.size(); i++) {
        ITM_SendChar(arg.data()[i]);
    }
#endif
}

void printViaSTLink(std::string &arg) {

#ifdef POLYCONTROL
    if (FlagHandler::USB_HS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_HS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 5000) {
                FlagHandler::USB_HS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < arg.size(); i++) {
        ITM_SendChar(arg.data()[i]);
    }
#endif
}

#endif
