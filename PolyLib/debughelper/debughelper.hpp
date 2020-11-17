#pragma once

#define DEBUG 1

#if DEBUG

#include <stdint.h>

#ifdef POLYCONTROL
#include "usbd_cdc_if.h"
#elif POLYRENDER
#include "main.h"
// extern "C" {
// extern uint32_t ITM_SendChar(uint32_t ch);
// }
#endif

#include <string>

#define print(...) printViaSTLink(__VA_ARGS__)
#define println(...) printlnViaSTLink(__VA_ARGS__)

template <typename T> void printViaSTLink(T &&arg) {
    std::string str;
    str.append(std::to_string(arg));

#ifdef POLYCONTROL
    while (CDC_Transmit_HS((uint8_t *)str.data(), str.length()) != USBD_OK) {
    }
#elif POLYRENDER
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
#endif
}

void printViaSTLink(const char *arg);
void printViaSTLink(char *arg);

void printViaSTLink(const std::string &arg);

void printViaSTLink(std::string &arg);

template <typename T, typename... A> void printViaSTLink(T &&arg, A &&... args) {
    printViaSTLink(arg);
    printViaSTLink(args...);
}

template <typename... T> void printlnViaSTLink(T &&... args) {
    printViaSTLink(args...);
    printViaSTLink("\r\n");
}

#else

#define print(...)
#define println(...)

#endif