#include "debughelper.hpp"

void printViaSTLink(const char *arg) {
    std::string str;
    str.append(arg);
    // HAL_UART_Transmit(&huart3, (uint8_t *)str.data(), str.length(), 100);
    
    for (uint32_t i = 0; i < str.size(); i++) {
        ITM_SendChar(str.data()[i]);
    }
}

void printViaSTLink(const std::string &arg) {
    // HAL_UART_Transmit(&huart3, (uint8_t *)arg.data(), arg.length(), 100);
    for (uint32_t i = 0; i < arg.size(); i++) {
        ITM_SendChar(arg.data()[i]);
    }
}

void printViaSTLink(std::string &arg) {
    // HAL_UART_Transmit(&huart3, (uint8_t *)arg.data(), arg.length(), 100);
    for (uint32_t i = 0; i < arg.size(); i++) {
        ITM_SendChar(arg.data()[i]);
    }
}
