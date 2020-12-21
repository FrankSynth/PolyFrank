#pragma once

#include "circularbuffer/circularbuffer.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "dma2d.h"
#include "flagHandler/flagHandler.hpp"
#include "fonts/polyfonts.h"
#include "ltdc.h"
#include "tim.h"
#include <stdint.h>
#include <string>

#define LCDWIDTH 800u
#define LCDHEIGHT 480u
#define LCDDATASIZE 2u
#define MAXDRAWCALLS 512u

#define COLORMODE DMA2D_INPUT_ARGB4444;

#define WHITE 0xFFFFFFFF
#define BLACK 0x00FFFFFF
#define BLUE 0x000000FF

typedef enum { LEFT, CENTER, RIGHT } FONTALIGN;
typedef enum { R2M, M2MTRANSPARENT, M2MTRANSPARENT_A4 } DRAWTYPE;
typedef enum { RENDER_DONE, RENDER_PROGRESS, RENDER_WAIT } RENDERSTATE;

#define FRAMEBUFFERSIZE LCDWIDTH *LCDHEIGHT *LCDDATASIZE

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

extern CircularBuffer<renderTask, MAXDRAWCALLS> renderQueue;

// extern uint8_t FrameBuffer[BUFFERSIZE];

void setRenderState(RENDERSTATE state);
RENDERSTATE getRenderState();

void GFX_Init();
void SwitchFrameBuffer();

void DMA2D_DefaultConfig(int colorMode);
void IRQHandler(void);

// draw functions
void drawRectangleFill(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

// callbacks
void TransferError(DMA2D_HandleTypeDef *hdma2d);
void TransferComplete(DMA2D_HandleTypeDef *hdma2d);
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc);
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc);

void addToRenderQueue(renderTask &task);
void callNextTask();

void drawString(std::string &text, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment);
void drawRectangleChampfered(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t radius);
uint16_t getStringWidth(std::string &text, const GUI_FONTINFO *font);
