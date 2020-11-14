#pragma once

#include <stdint.h>

#define GUI_CONST_STORAGE const

typedef struct {
    uint32_t XSize;
    uint32_t XDist;
    uint32_t BytesPerLine;
    const unsigned char *pData;
} GUI_CHARINFO;

typedef struct {
    uint32_t size;
    GUI_CONST_STORAGE GUI_CHARINFO *font;
} GUI_FONTINFO;

extern GUI_CONST_STORAGE GUI_FONTINFO GUI_FontBahnschrift12_FontInfo;

extern GUI_CONST_STORAGE GUI_FONTINFO GUI_FontBahnschrift24_FontInfo;
extern GUI_CONST_STORAGE GUI_FONTINFO GUI_FontBahnschrift32_FontInfo;
extern GUI_CONST_STORAGE GUI_FONTINFO GUI_FontBahnschriftSemiBold48_FontInfo;
extern GUI_CONST_STORAGE GUI_FONTINFO GUI_FontBahnschriftSemiBold28_FontInfo;