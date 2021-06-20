#pragma once

#define DEBUG 1

#if DEBUG

#include "datacore/dataHelperFunctions.hpp"
#ifdef POLYCONTROL
#include "flagHandler/flagHandler.hpp"
#include "usbd_cdc_if.hpp"

extern elapsedMicros USBHSTIMEOUT;

#elif POLYRENDER
#include "main.h"
#endif

#include <stdint.h>
#include <string>

#define print(...) printViaSTLink(__VA_ARGS__)
#define println(...) printlnViaSTLink(__VA_ARGS__)

// std::string printString;

template <typename T> void printViaSTLink(T &&arg) {
    std::string str;
    str.append(std::to_string(arg));

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

void printViaSTLink(const char *arg);
void printViaSTLink(const unsigned char *arg);
void printViaSTLink(char *arg);
void printViaSTLink(unsigned char *arg);

void printViaSTLink(const std::string &arg);

void printViaSTLink(std::string &arg);

template <typename T, typename... A> void printViaSTLink(T &&arg, A &&...args) {
    printViaSTLink(arg);
    printViaSTLink(args...);
}

template <typename... T> void printlnViaSTLink(T &&...args) {
    printViaSTLink(args...);
    printViaSTLink("\r\n");
}

#else

#define print(...)
#define println(...)

#endif