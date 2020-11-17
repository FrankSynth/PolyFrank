#include "debughelper.hpp"
#if DEBUG

void printViaSTLink(const char *arg) {
    std::string str;
    str.append(arg);

#ifdef POLYCONTROL
    while (CDC_Transmit_HS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
#endif
}

void printViaSTLink(const std::string &arg) {

#ifdef POLYCONTROL
    while (CDC_Transmit_HS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < arg.size(); i++) {
        ITM_SendChar(arg.data()[i]);
    }
#endif
}

void printViaSTLink(std::string &arg) {

#ifdef POLYCONTROL
    while (CDC_Transmit_HS((uint8_t *)arg.data(), arg.length()) == USBD_BUSY) {
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < arg.size(); i++) {
        ITM_SendChar(arg.data()[i]);
    }
#endif
}

#endif
