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
#define MAXDRAWCALLS 1024u

#define COLORMODE DMA2D_INPUT_ARGB4444;

#define WHITE 0xFFFFFFFF
#define BLACK 0x00FFFFFF
#define BLUE 0x000000FF

typedef enum { LEFT, CENTER, RIGHT, TOP, BOTTOM } FONTALIGN;
typedef enum { R2M, M2MTRANSPARENT, M2MTRANSPARENT_A4, M2MRGB565, M2MARGB4444 } DRAWTYPE;
typedef enum { RENDER_DONE, RENDER_PROGRESS, RENDER_WAIT } RENDERSTATE;

#define FRAMEBUFFERSIZE LCDWIDTH *LCDHEIGHT *LCDDATASIZE
#define WAVEFORMHEIGHT 150

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

typedef struct {
    uint16_t buffer[WAVEFORMHEIGHT][LCDWIDTH - 60];
    uint16_t width = LCDWIDTH - 60;
    uint16_t height = WAVEFORMHEIGHT;
} WaveBuffer;

extern CircularBuffer<renderTask, MAXDRAWCALLS> renderQueue;
extern WaveBuffer waveBuffer;

// extern uint8_t FrameBuffer[BUFFERSIZE];

void setRenderState(RENDERSTATE state);
RENDERSTATE getRenderState();

void GFX_Init();
void SwitchFrameBuffer();

void DMA2D_DefaultConfig(int colorMode);
void IRQHandler(void);

// draw functions
void drawRectangleFill(uint32_t color, uint32_t x, uint32_t y, int width, int height);

// callbacks
void TransferError(DMA2D_HandleTypeDef *hdma2d);
void TransferComplete(DMA2D_HandleTypeDef *hdma2d);
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc);
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc);

void addToRenderQueue(renderTask &task);
void callNextTask();

void drawString(std::string &text, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment);
void drawString(const char *text, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment);
void drawStringVertical(std::string &text, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                        FONTALIGN alignment = CENTER);

void drawRectangleChampfered(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t radius);
uint16_t getStringWidth(std::string &text, const GUI_FONTINFO *font);

void copyWaveBuffer(WaveBuffer &waveBuffer, uint16_t x, uint16_t y);

/////Software GFX Functions////////
void drawLine(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color);
void drawLineThick(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color);
void drawLineWidth(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, float wd, uint16_t &color);

void drawLineAA(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color);
void drawQuadBezier(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, int x2, int y2, uint16_t &color);

void drawCubicSpline(WaveBuffer &waveBuffer, int n, int x[], int y[], uint16_t &color);

void copyBitmapToBuffer(const GUI_BITMAP &image, uint32_t color, uint16_t x, uint16_t y);

void drawFilledCircle(WaveBuffer &waveBuffer, int x0, int y0, uint16_t &color, float r);

inline void drawPixelThick(WaveBuffer &waveBuffer, int16_t x0, int16_t y0, uint16_t &color) {
    // draw a circle

    uint16_t x;
    uint16_t y;

    for (int16_t xOffset = -1; xOffset < 2; xOffset++) {
        for (int16_t yOffset = -1; yOffset < 2; yOffset++) {
            x = x0 + xOffset;
            y = y0 + yOffset;

            if (x < waveBuffer.width && y < waveBuffer.height) // check boundaries
                waveBuffer.buffer[y][x] = color;               // simple "blend"
        }
    }
}