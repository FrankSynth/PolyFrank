
#pragma once

#include <stdint.h>

typedef struct {
    uint32_t XSize;
    uint32_t YSize;
    uint32_t BytesPerLine;
    const unsigned char *pData;
} GUI_BITMAP;

#define GUI_CONST_STORAGE const

extern const GUI_BITMAP bmpUSBLogo;
extern const GUI_BITMAP bmpDINLogo;
