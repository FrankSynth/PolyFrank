#include "debughelper.hpp"
#include "circularbuffer/circularbuffer.hpp"
#include "datacore/datalocation.hpp"

#if DEBUG
#ifdef POLYCONTROL
elapsedMicros USBHSTIMEOUT;
RAM1 CircularBuffer<char, 1024> consoleBuffer;

void appendBuffer(std::string *string) {
    for (uint32_t i = 0; i < string->size(); i++) {
        consoleBuffer.push_back(string->data()[i]);
    }
}
void appendBuffer(const std::string *string) {
    for (uint32_t i = 0; i < string->size(); i++) {
        consoleBuffer.push_back(string->data()[i]);
    }
}

#endif
void printViaSTLink(const char *arg) {
    std::string str;
    str.append(arg);

#ifdef POLYCONTROL
    appendBuffer(&str);
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
    appendBuffer(&str);

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
    appendBuffer(&str);

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
    appendBuffer(&str);
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
    appendBuffer(&arg);
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
    appendBuffer(&arg);
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
