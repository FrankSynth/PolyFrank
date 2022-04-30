#ifdef POLYCONTROL

#include "gfx.hpp"
#include "datacore/datalocation.hpp"

#include <algorithm> // std::max

volatile FRAMEBUFFER_A ALIGN_32BYTES(uint8_t FrameBufferA[FRAMEBUFFERSIZE]);
volatile FRAMEBUFFER_B ALIGN_32BYTES(uint8_t FrameBufferB[FRAMEBUFFERSIZE]);

volatile RAM1 ALIGN_32BYTES(uint32_t color_1);
volatile RAM1 ALIGN_32BYTES(uint32_t color_2);
volatile RAM1 ALIGN_32BYTES(uint32_t color_3);
volatile RAM1 ALIGN_32BYTES(uint32_t color_4);

volatile uint32_t dma2d_TransferComplete_Flag = 0;

uint8_t *pFrameBuffer;

RENDERSTATE renderState = RENDER_DONE;

// renderQueue
// std::list<renderTask> renderQueueList;

RAM1 CircularBuffer<renderTask, MAXDRAWCALLS> renderQueue;
RAM1 ALIGN_32BYTES(WaveBuffer waveBuffer);

void GFX_Init() {
    // IRQHandler();
    // set handles
    // init LTDC //
    MX_LTDC_Init();
    SwitchFrameBuffer();

    // set BG Color;
    hltdc.Init.Backcolor.Red = 0;
    hltdc.Init.Backcolor.Green = 3;
    hltdc.Init.Backcolor.Blue = 5;

    //__HAL_LTDC_RELOAD_CONFIG(&hltdc);
    HAL_LTDC_Init(&hltdc); // update config

    // init DMA2D //
    MX_DMA2D_Init();
    DMA2D_DefaultConfig(DMA2D_OUTPUT_ARGB4444);

    // clean
    drawRectangleFill(0x00000000, 0, 0, LCDWIDTH, LCDHEIGHT);

    HAL_LTDC_ProgramLineEvent(&hltdc, 0);
}

void SwitchFrameBuffer() {
    static uint32_t toggle = 0;
    LTDC_LAYER(&hltdc, 0)->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);

    if (toggle) {
        pFrameBuffer = (uint8_t *)FrameBufferA;
        LTDC_LAYER(&hltdc, 0)->CFBAR = (uint32_t)FrameBufferB;
    }
    else {
        pFrameBuffer = (uint8_t *)FrameBufferB;
        LTDC_LAYER(&hltdc, 0)->CFBAR = (uint32_t)FrameBufferA;
    }

    __HAL_LTDC_RELOAD_CONFIG(&hltdc);

    toggle = !toggle;
}

void DMA2D_DefaultConfig(int colorMode) {

    hdma2d.Init.ColorMode = colorMode;               /* DMA2D Output color mode */
    hdma2d.Init.OutputOffset = 0x0;                  /* No offset in output */
    hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the output image */
    hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the output image */

    // Register Callbacks
    hdma2d.XferCpltCallback = TransferComplete;
    hdma2d.XferErrorCallback = TransferError;
    hdma2d.Instance = DMA2D;

    /* DMA2D Initialization */
    HAL_DMA2D_Init(&hdma2d);
}

void IRQHandler(void) {
    HAL_DMA2D_IRQHandler(&hdma2d);
    HAL_LTDC_IRQHandler(&hltdc);
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc) {
    SwitchFrameBuffer();
    setRenderState(RENDER_DONE);
}
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {}

void TransferError(DMA2D_HandleTypeDef *hdma2d) {
    PolyError_Handler("ERROR | LTDC | Transfer Callback");
}

void TransferComplete(DMA2D_HandleTypeDef *hdma2d) {
    callNextTask();
}

////// Draw Functions//////

void drawRectangleFill(uint32_t color, uint32_t x, uint32_t y, int width, int height) {
    renderTask task;

    if (width < 0) {
        task.x = x + width;
        task.width = abs(width);
    }
    else {
        task.x = x;
        task.width = width;
    }

    if (height < 0) {
        task.y = y + height;
        task.height = abs(height);
    }
    else {
        task.y = y;
        task.height = height;
    }

    task.color = color;

    // Choose render task type
    task.mode = R2M;

    // check boundaries
    if ((task.x + task.width) > LCDWIDTH) {
        task.width = LCDWIDTH - task.x;
    }

    if ((task.y + task.height) > LCDHEIGHT) {
        task.height = LCDHEIGHT - task.y;
    }

    addToRenderQueue(task);
}

void drawRectangleChampfered(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t radius) {
    renderTask task;
    uint32_t tmpWidth = width;
    uint32_t tmpHeight = height;

    task.x = x;
    task.y = y;

    task.color = color;

    // Choose render task type
    task.mode = R2M;

    // check boundaries
    if ((task.x + width) > LCDWIDTH) {
        tmpWidth = LCDWIDTH - task.x;
    }

    if ((task.y + height) > LCDHEIGHT) {
        tmpHeight = LCDHEIGHT - task.y;
    }

    if (tmpWidth == 0 || tmpHeight == 0) {
        return;
    }

    int tmpRadius;

    if (2 * radius >= tmpWidth) {
        tmpRadius = tmpWidth / 4;
    }
    else if (2 * radius >= tmpHeight) {
        tmpRadius = tmpHeight / 4;
    }
    else {
        tmpRadius = radius;
    }

    // draw center

    task.x = x + tmpRadius;
    task.y = y + tmpRadius;
    task.height = tmpHeight - 2 * tmpRadius;
    task.width = tmpWidth - 2 * tmpRadius;

    if (width != 0 && task.height != 0) {
        addToRenderQueue(task);
    }

    if (task.height > 1) {
        // links
        for (int i = 0; i < tmpRadius; i++) {
            task.x = x + i;
            task.y = y + 2 * tmpRadius - i;
            task.width = 1;
            task.height = tmpHeight - 4 * tmpRadius + 2 * i;
            addToRenderQueue(task);
        }
        // rechts
        for (int i = 0; i < tmpRadius; i++) {
            task.x = x + width - i - 1;
            task.y = y + 2 * tmpRadius - i;
            task.width = 1;
            task.height = tmpHeight - 4 * tmpRadius + 2 * i;
            addToRenderQueue(task);
        }
    }
    // oben
    if (task.width > 1) {

        for (int i = 0; i < tmpRadius; i++) {
            task.x = x + 2 * tmpRadius - i;
            task.y = y + i;
            task.width = tmpWidth + 2 * i - 4 * tmpRadius;
            task.height = 1;
            addToRenderQueue(task);
        }
        // unten
        for (int i = 0; i < tmpRadius; i++) {
            task.x = x + 2 * tmpRadius - i;
            task.y = y + height - i - 1;
            task.width = tmpWidth + 2 * i - 4 * tmpRadius;
            task.height = 1;
            addToRenderQueue(task);
        }
    }
}

void drawString(std::string &text, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment) {
    renderTask task;

    task.mode = M2MTRANSPARENT_A4;  // Set DMA2D To copy M2M with Blending
    task.height = activeFont->size; // Set Font Height
    task.color = color;             // Set Font Height

    // Curser Position
    uint16_t posX = x;
    uint16_t posY = y;

    // alignment
    if (alignment != LEFT) {
        uint16_t offset = 0;

        // calculate center of current string
        for (char &c : text) {
            offset += activeFont->font[(uint8_t)c - 32].XSize;
        }
        if (alignment == CENTER) {
            posX -= offset / 2; // offset posX to Center
        }
        else if (alignment == RIGHT) {
            posX -= offset; // offset posX for right Right alignment
        }
    }

    // For each Char
    for (char &c : text) {

        task.x = posX;
        task.y = posY;

        task.width = activeFont->font[(uint8_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint8_t)c - 32].pData; // Pointer to Character

        addToRenderQueue(task); // Add Task to RenderQue

        posX += activeFont->font[(uint8_t)c - 32].XSize; // Distance to next character
    }
}

void copyWaveBuffer(WaveBuffer &waveBuffer, uint16_t x, uint16_t y) {
    renderTask task;

    task.mode = M2MARGB4444; // Set DMA2D To copy M2M with Blending

    task.x = x;
    task.y = y;

    task.width = waveBuffer.width;   // Character Width
    task.height = waveBuffer.height; // Character Width

    task.pSource = (uint32_t)(uint8_t **)waveBuffer.buffer; // Pointer to Character

    addToRenderQueue(task); // Add Task to RenderQue
}

void copyBitmapToBuffer(const GUI_BITMAP &image, uint32_t color, uint16_t x, uint16_t y) {
    renderTask task;

    task.mode = M2MTRANSPARENT_A4; // Set DMA2D To copy M2M with Blending

    task.x = x;
    task.y = y;

    task.width = image.BytesPerLine * 2; // Character Width
    task.height = image.YSize;           // Character Width
    task.color = color;                  // Set Font Height

    task.pSource = (uint32_t)image.pData; // Pointer to Character

    addToRenderQueue(task); // Add Task to RenderQue
}

void drawStringVertical(std::string &text, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                        FONTALIGN alignment) {
    renderTask task;

    task.mode = M2MTRANSPARENT_A4;  // Set DMA2D To copy M2M with Blending
    task.height = activeFont->size; // Set Font Height
    task.color = color;             // Set Font Height

    // Curser Position
    uint16_t posX = x;
    uint16_t posY = y;

    if (alignment == CENTER) {
        posY = y - (text.size() * (task.height - 4)) / 2;
    }
    else if (alignment == BOTTOM) {
        posY = y - (text.size() * (task.height - 4));
    }
    // For each Char
    for (char &c : text) {

        task.x = posX - activeFont->font[(uint8_t)c - 32].XSize / 2;
        task.y = posY;

        task.width = activeFont->font[(uint8_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint8_t)c - 32].pData; // Pointer to Character

        addToRenderQueue(task); // Add Task to RenderQue

        posY += task.height - 4; // Distance to next character
    }
}
void drawString(const char *charArray, uint32_t color, uint16_t x, uint16_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment) {
    renderTask task;

    std::string text = charArray;

    task.mode = M2MTRANSPARENT_A4;  // Set DMA2D To copy M2M with Blending
    task.height = activeFont->size; // Set Font Height
    task.color = color;             // Set Font Height

    // Curser Position
    uint16_t posX = x;
    uint16_t posY = y;

    // alignment
    if (alignment != LEFT) {
        uint16_t offset = 0;

        // calculate center of current string
        for (char &c : text) {
            offset += activeFont->font[(uint8_t)c - 32].XSize;
        }
        if (alignment == CENTER) {
            posX -= offset / 2; // offset posX to Center
        }
        else if (alignment == RIGHT) {
            posX -= offset; // offset posX for right Right alignment
        }
    }

    // For each Char
    for (char &c : text) {

        task.x = posX;
        task.y = posY;

        task.width = activeFont->font[(uint8_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint8_t)c - 32].pData; // Pointer to Character

        addToRenderQueue(task); // Add Task to RenderQue

        posX += activeFont->font[(uint8_t)c - 32].XSize; // Distance to next character
    }
}

uint16_t getStringWidth(std::string &text, const GUI_FONTINFO *font) {
    uint16_t offset = 0;

    for (char &c : text) {
        offset += font->font[(uint8_t)c - 32].XSize;
    }
    return offset;
}

void addToRenderQueue(renderTask &task) {

    task.outputOffset = LCDWIDTH - task.width;
    task.pTarget = (uint32_t)pFrameBuffer + (LCDWIDTH * task.y + task.x) * 2;

    if (((task.x + task.width) <= LCDWIDTH)       //
        && ((task.y + task.height) <= LCDHEIGHT)  //
        && task.width                             //
        && task.height                            //
        && (renderQueue.size() < MAXDRAWCALLS)) { // in render region and width and height not 0

        renderQueue.push_back(task);

        if (HAL_DMA2D_GetState(&hdma2d) == HAL_DMA2D_STATE_READY)
            callNextTask();
    }
}

void setRenderState(RENDERSTATE state) {
    renderState = state;
}
RENDERSTATE getRenderState() {
    return renderState;
}

void callNextTask() {

    // Task//
    if (renderQueue.empty()) {
        if (renderState == RENDER_WAIT) { // last render task done -> switch Buffer
            HAL_LTDC_ProgramLineEvent(&hltdc, 0);
        }
        return;
    }

    renderTask task = renderQueue.front(); // get next render task
    renderQueue.pop_front();               // remove from renderQueue

    // DMA2D//
    HAL_DMA2D_DeInit(&hdma2d); // de init dma2d

    hdma2d.Init.OutputOffset = task.outputOffset; // DMA2D OutputBuffer Offset

    // select DMA Mode
    if (task.mode == M2MTRANSPARENT_A4) { // Memory to Memory with blending

        hdma2d.Init.Mode = DMA2D_M2M_BLEND;

        /* Foreground layer Configuration */
        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputAlpha = task.color;             /* COLOR */
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A4;     /* Foreground format is A4*/
        hdma2d.LayerCfg[1].InputOffset = 0x0;                   /* No offset in input */
        hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input foreground image */
        hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input foreground image */

        /* Background layer Configuration */
        hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[0].InputAlpha = 0xFF;                   /* 255 : fully opaque */
        hdma2d.LayerCfg[0].InputColorMode = COLORMODE;          /* Background format*/
        hdma2d.LayerCfg[0].InputOffset = task.outputOffset;     /* Background input offset*/
        hdma2d.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input background image */
        hdma2d.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input background image */

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Start DMA2D Task */
        HAL_DMA2D_BlendingStart_IT(&hdma2d, task.pSource, /* Color value in Register to Memory DMA2D mode */
                                   task.pTarget,          /* DMA2D src2 */
                                   task.pTarget,          /* DMA2D output buffer */
                                   task.width,            /* width of buffer in pixels */
                                   task.height);
    }

    // select DMA Mode
    else if (task.mode == M2MRGB565) { // Memory to Memory with blending

        hdma2d.Init.Mode = DMA2D_M2M;

        /* Foreground layer Configuration */
        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputAlpha = task.color;             /* COLOR */
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565; /* Foreground format is A4*/
        hdma2d.LayerCfg[1].InputOffset = 0x0;                   /* No offset in input */
        hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input foreground image */
        hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input foreground image */

        /* Background layer Configuration */
        hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[0].InputAlpha = 0xFF;                   /* 255 : fully opaque */
        hdma2d.LayerCfg[0].InputColorMode = COLORMODE;          /* Background format*/
        hdma2d.LayerCfg[0].InputOffset = task.outputOffset;     /* Background input offset*/
        hdma2d.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input background image */
        hdma2d.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input background image */

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Start DMA2D Task */
        HAL_DMA2D_BlendingStart_IT(&hdma2d, task.pSource, /* Color value in Register to Memory DMA2D mode */
                                   task.pTarget,          /* DMA2D src2 */
                                   task.pTarget,          /* DMA2D output buffer */
                                   task.width,            /* width of buffer in pixels */
                                   task.height);
    }

    // select DMA Mode
    else if (task.mode == M2MARGB4444) { // Memory to Memory with blending

        hdma2d.Init.Mode = DMA2D_M2M;

        /* Foreground layer Configuration */
        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputAlpha = task.color;               /* COLOR */
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB4444; /* Foreground format is A4*/
        hdma2d.LayerCfg[1].InputOffset = 0x0;                     /* No offset in input */
        hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;        /* No R&B swap for the input foreground image */
        hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input foreground image */

        /* Background layer Configuration */
        hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[0].InputAlpha = 0xFF;                   /* 255 : fully opaque */
        hdma2d.LayerCfg[0].InputColorMode = COLORMODE;          /* Background format*/
        hdma2d.LayerCfg[0].InputOffset = task.outputOffset;     /* Background input offset*/
        hdma2d.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input background image */
        hdma2d.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input background image */

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Start DMA2D Task */
        HAL_DMA2D_BlendingStart_IT(&hdma2d, task.pSource, /* Color value in Register to Memory DMA2D mode */
                                   task.pTarget,          /* DMA2D src2 */
                                   task.pTarget,          /* DMA2D output buffer */
                                   task.width,            /* width of buffer in pixels */
                                   task.height);
    }

    else if (task.mode == R2M) { // Register to Memory
        hdma2d.Init.Mode = DMA2D_R2M;

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

        /* Start DMA2D Task */
        HAL_DMA2D_Start_IT(&hdma2d, task.color, /* Color value in Register to Memory DMA2D mode */
                           task.pTarget,        /* DMA2D output buffer */
                           task.width,          /* width of buffer in pixels */
                           task.height);        /* height of buffer in lines */
    }
}

inline void drawPixel(WaveBuffer &waveBuffer, uint16_t x, uint16_t y, uint16_t &color) {
    waveBuffer.buffer[y][x] = color;
}

inline void drawPixelBrigthness(WaveBuffer &waveBuffer, uint16_t x, uint16_t y, uint8_t brigthness, uint16_t &color) {

    waveBuffer.buffer[y][x] = ((color & 0xFFF) | ((uint16_t((255 - brigthness) & 0xF0)) << 8));
}

void drawLine(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */
        drawPixel(waveBuffer, x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        } /* e_xy+e_x > 0 */
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        } /* e_xy+e_y < 0 */
    }
}

void draw3Line(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */

        // if (dx > dy) {
        drawPixel(waveBuffer, x0, y0 - 1, color);
        drawPixel(waveBuffer, x0, y0, color);
        drawPixel(waveBuffer, x0, y0 + 1, color);
        // }
        // else {
        //     drawPixel(waveBuffer, x0 - 1, y0, color);
        //     drawPixel(waveBuffer, x0, y0, color);
        //     drawPixel(waveBuffer, x0 + 1, y0, color);
        // }

        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        } /* e_xy+e_x > 0 */
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        } /* e_xy+e_y < 0 */
    }
}

void drawLineWidth(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color, float wd) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2, x2, y2; /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (wd = (wd + 1) / 2;;) { /* pixel loop */
        drawPixelBrigthness(waveBuffer, x0, y0, std::max(0.f, 255 * (abs(err - dx + dy) / ed - wd + 1)), color);
        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            for (e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx) {
                // drawPixelBrigthness(waveBuffer, x0, y2 += sy, std::max(0.f, 255 * (abs(e2) / ed - wd + 1)), color);
                y2 += sy;
            }
            if (x0 == x1)
                break;
            e2 = err;
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            for (e2 = dx - e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy)
                drawPixelBrigthness(waveBuffer, x2 += sx, y0, std::max(0.f, 255 * (abs(e2) / ed - wd + 1)), color);
            if (y0 == y1)
                break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawLineAA(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t &color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2, x2; /* error value e_xy */
    int ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (;;) { /* pixel loop */
        drawPixelBrigthness(waveBuffer, x0, y0, 255 * abs(err - dx + dy) / ed, color);
        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            if (x0 == x1)
                break;
            if (e2 + dy < ed)
                drawPixelBrigthness(waveBuffer, x0, y0 + sy, 255 * (e2 + dy) / ed, color);
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            if (y0 == y1)
                break;
            if (dx - e2 < ed)
                drawPixelBrigthness(waveBuffer, x2 + sx, y0, 255 * (dx - e2) / ed, color);
            err += dx;
            y0 += sy;
        }
    }
}

#endif // ifdef POLYCONTROL