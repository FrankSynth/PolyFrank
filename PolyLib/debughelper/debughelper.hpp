#pragma once

#ifdef POLYCONTROL
#define DEBUG 1
#elif POLYRENDER
#define DEBUG 1
#endif

#if DEBUG

#include "datacore/dataHelperFunctions.hpp"
#ifdef POLYCONTROL
#include "flagHandler/flagHandler.hpp"
#include "usbd_cdc_if.hpp"

extern elapsedMicros USBHSTIMEOUT;

#elif POLYRENDER
uint8_t sendString(const char *message);
uint8_t sendString(const std::string &message);
uint8_t sendString(std::string &&message);
#endif

#include <stdint.h>
#include <string>

#define print(...) printViaSTLink(__VA_ARGS__)
#define println(...) printlnViaSTLink(__VA_ARGS__)

#ifdef POLYCONTROL
void appendBuffer(const std::string &string);
#endif

template <typename T> void printViaSTLink(T &&arg) {
    std::string str;
    str.append(std::to_string(arg));

#ifdef POLYCONTROL
    appendBuffer(str);

    USBHSTIMEOUT = 0;
    while (CDC_Transmit_FS((uint8_t *)str.data(), str.length()) == USBD_BUSY) {
        if (USBHSTIMEOUT > 5000) {
            FlagHandler::USB_FS_CONNECTED = false;
            return;
        }
    }
#elif POLYRENDER
    sendString(str);
#endif
}

void printViaSTLink(const char *arg);
void printViaSTLink(const unsigned char *arg);
void printViaSTLink(char *arg);
void printViaSTLink(unsigned char *arg);

void printViaSTLink(const std::string &arg);
void printViaSTLink(std::string &arg);
void printViaSTLink(std::string &&arg);

template <typename T, typename... A> void printViaSTLink(T &&arg, A &&...args) {
    printViaSTLink(arg);
    printViaSTLink(args...);
}

template <typename... T> void printlnViaSTLink(T &&...args) {
    printViaSTLink(args...);
#ifdef POLYCONTROL
    printViaSTLink("\r\n");
#endif
}

#else

#define print(...)
#define println(...)

#endif

void jumpToBootloader();
void rebootToBooloader();