#include "debughelper.hpp"

void printViaSTLink(const char *arg) {
    // std::string str;
    // str.append(arg);
    // HAL_UART_Transmit(&huart3, (uint8_t *)str.data(), str.length(), 100);
    printf(arg);
}

void printViaSTLink(const std::string &arg) {
    // HAL_UART_Transmit(&huart3, (uint8_t *)arg.data(), arg.length(), 100);
    printf(arg.data());
}

void printViaSTLink(std::string &arg) {
    // HAL_UART_Transmit(&huart3, (uint8_t *)arg.data(), arg.length(), 100);
    printf(arg.data());
}