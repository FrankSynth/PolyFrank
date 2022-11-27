#pragma once

#define DFUMODE 126 // activate DFU MODE
#define ISPMODE 124 // activate SPI MODE
#define DATAVALID 0x01
#define DATAINVALID 0x02
#define BLOCKVALID 0x03
#define ENTERMODE 0x04
#define ERASEDONE 0x05
#define UPDATEFINISH 0x06
#define STARTERASE 0x07
#define COMFAILURE 0x08

// #define HELP 126 // activate DFU MODE

void jumpToBootloader();
void rebootToBooloader();

void flashRenderMCUSPI();