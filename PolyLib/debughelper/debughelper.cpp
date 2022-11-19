#include "debughelper.hpp"
#include "circularbuffer/circularbuffer.hpp"
#include "datacore/datalocation.hpp"

#if DEBUG
#ifdef POLYCONTROL
elapsedMicros USBHSTIMEOUT;
RAM1 CircularBuffer<char, 1024> consoleBuffer;

void appendBuffer(const std::string &string) {
    for (uint32_t i = 0; i < string.size(); i++) {
        consoleBuffer.push_back(string.data()[i]);
    }
}

#endif
void printViaSTLink(const char *arg) {
    std::string str;
    str.append(arg);

#ifdef POLYCONTROL
    appendBuffer(str);
    if (FlagHandler::USB_FS_CONNECTED) {

        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(str);
#endif
}

void printViaSTLink(const unsigned char *arg) {
    std::string str;
    str.append((const char *)arg);

#ifdef POLYCONTROL
    appendBuffer(str);

    if (FlagHandler::USB_FS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(str);

#endif
}

void printViaSTLink(char *arg) {
    std::string str;
    str.append(arg);

#ifdef POLYCONTROL
    appendBuffer(str);

    if (FlagHandler::USB_FS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(str);

#endif
}

void printViaSTLink(unsigned char *arg) {
    std::string str;
    str.append((char *)arg);

#ifdef POLYCONTROL
    appendBuffer(str);
    if (FlagHandler::USB_FS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(str);

#endif
}

void printViaSTLink(const std::string &arg) {

#ifdef POLYCONTROL
    appendBuffer(arg);
    if (FlagHandler::USB_FS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(arg);

#endif
}

void printViaSTLink(std::string &arg) {

#ifdef POLYCONTROL
    appendBuffer(arg);
    if (FlagHandler::USB_FS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(arg);

#endif
}

void printViaSTLink(std::string &&arg) {

#ifdef POLYCONTROL
    appendBuffer(arg);
    if (FlagHandler::USB_FS_CONNECTED) {
        USBHSTIMEOUT = 0;
        while (CDC_Transmit_FS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
            if (USBHSTIMEOUT > 1000) {
                FlagHandler::USB_FS_CONNECTED = false;
                return;
            }
        }
    }
#elif POLYRENDER
    sendString(arg);

#endif
}

#endif
