#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "dma2d.h"
#include "flagHandler/flagHandler.hpp"
#include "fonts/polyfonts.h"
#include "img/polybitmap.h"
#include "ltdc.h"
#include "tim.h"
#include <stdint.h>
#include <string>

#define LCDWIDTH 800u
#define LCDHEIGHT 480u
#define LCDDATASIZE 2u
#define MAXDRAWCALLS 2500u

#define WHITE 0xFFFFFFFF
#define BLACK 0x00FFFFFF
#define BLUE 0x000000FF

typedef enum { LEFT, CENTER, RIGHT, TOP, BOTTOM } FONTALIGN;
typedef enum { R2M, M2MTRANSPARENT, M2MTRANSPARENT_A4, M2MRGB565, M2MARGB4444, UNDEFINDDRAWTYPE } DRAWTYPE;
typedef enum { RENDER_DONE, RENDER_PROGRESS, RENDER_WAIT } RENDERSTATE;

#define FRAMEBUFFERSIZE LCDWIDTH *LCDHEIGHT *LCDDATASIZE

#define WAVEFORMHEIGHT 160
#define CUSTOMINFOHEIGHT 35
#define WAVEFORMHEIGHTINFO WAVEFORMHEIGHT - CUSTOMINFOHEIGHT
#define WAVEFORMQUICKHEIGHT 180

extern uint8_t *pFrameBuffer;
// rendertask struct
typedef struct {
    uint32_t pSource;
    uint32_t pTarget;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t outputOffset;

    uint32_t color; // for Alpha Modes
    DRAWTYPE mode;

} renderTask;

extern uint16_t waveformBuffer[][LCDWIDTH - 50];
extern uint16_t waveformQuickBuffer[][LCDWIDTH - 50];

typedef struct {
    uint32_t width = LCDWIDTH - 50;
    uint32_t height = 0;
    uint16_t (*buffer)[][LCDWIDTH - 50]; // extra heigth for quickview
} WaveBuffer;

extern CircularBuffer<renderTask, MAXDRAWCALLS> renderQueue;
extern WaveBuffer waveBuffer;
extern WaveBuffer waveQuickBuffer;

////////////GFX INIT////////////

void GFX_Init();

////////////LTDC | DMA2D////////////
void DMA2D_DefaultConfig(int colorMode);
void IRQHandler(void);
void SwitchFrameBuffer();

////////////callbacks////////////
void TransferError(DMA2D_HandleTypeDef *hdma2d);
void TransferComplete(DMA2D_HandleTypeDef *hdma2d);
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc);
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc);

void setRenderState(RENDERSTATE state);
RENDERSTATE getRenderState();

void addToRenderQueue(renderTask &task);
void callNextTask();

////////////GFX////////////
void drawRectangleFill(uint32_t color, uint32_t x, uint32_t y, int width, int height);
void drawString(const std::string &text, uint32_t color, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment);
void drawString(const char *text, uint32_t color, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment);
void drawStringVertical(const std::string &text, uint32_t color, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont,
                        FONTALIGN alignment = CENTER);
void drawRectangleCentered(uint32_t color, uint32_t radius, uint32_t x, uint32_t y);
void drawRectangleChampfered(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t radius);
uint32_t getStringWidth(const std::string &text, const GUI_FONTINFO *font);
void copyWaveBuffer(WaveBuffer &buffer, uint32_t x, uint32_t y);
void copyBitmapToBuffer(const GUI_BITMAP &image, uint32_t color, uint32_t x, uint32_t y);

////////////Software GFX ////////////
void drawFilledCircle(WaveBuffer &buffer, int x0, int y0, uint16_t color, float r);

void drawLine(WaveBuffer &buffer, int x0, int y0, int x1, int y1, uint16_t color);
void drawLineThick(WaveBuffer &buffer, int x0, int y0, int x1, int y1, uint16_t color);
void drawLineWidth(WaveBuffer &buffer, int x0, int y0, int x1, int y1, float wd, uint16_t color);
void drawQuadBezier(WaveBuffer &buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);

inline void drawPixelThick(WaveBuffer &buffer, int32_t x0, int32_t y0, uint16_t color) {

    for (int32_t xOffset = -1; xOffset < 2; xOffset++) {
        int32_t x = x0 + xOffset;

        for (int32_t yOffset = -1; yOffset < 2; yOffset++) {
            int32_t y = y0 + yOffset;

            if (x >= 0 && x < (int32_t)buffer.width && y >= 0 && y < (int32_t)buffer.height) // check boundaries
                (*buffer.buffer)[y][x] = color;
        }
    }
}
