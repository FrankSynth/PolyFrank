#ifdef POLYCONTROL

#include "gfx.hpp"
#include "datacore/datalocation.hpp"

#include <algorithm> // std::max

volatile FRAMEBUFFER_A ALIGN_32BYTES(uint8_t FrameBufferA[FRAMEBUFFERSIZE]);
volatile FRAMEBUFFER_B ALIGN_32BYTES(uint8_t FrameBufferB[FRAMEBUFFERSIZE]);

// volatile RAM1 ALIGN_32BYTES(uint32_t color_1);
// volatile RAM1 ALIGN_32BYTES(uint32_t color_2);
// volatile RAM1 ALIGN_32BYTES(uint32_t color_3);
// volatile RAM1 ALIGN_32BYTES(uint32_t color_4);

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

    //__HAL_LTDC_RELOAD_CONFIG(&hltdc);
    HAL_LTDC_Init(&hltdc); // update config

    // init DMA2D //
    MX_DMA2D_Init();
    DMA2D_DefaultConfig(DMA2D_OUTPUT_RGB565);

    // clean
    drawRectangleFill(0xFF000000, 0, 0, LCDWIDTH, LCDHEIGHT);

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

    /* Background layer Configuration */
    hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[0].InputColorMode = colorMode;          /* Background format*/
    hdma2d.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input background image */
    hdma2d.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input background image */

    /*Foreground layer Configuration */
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[0].InputColorMode = colorMode;          /* Background format*/
    hdma2d.LayerCfg[1].InputOffset = 0x0;                   /* No offset in input */
    hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;      /* No R&B swap for the input foreground image */
    hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No alpha inversion for the input foreground image */

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

    task.color = color;

    // Choose render task type
    task.mode = R2M;

    // check boundaries
    if ((x + width) > LCDWIDTH) {
        tmpWidth = LCDWIDTH - x;
    }

    if ((y + height) > LCDHEIGHT) {
        tmpHeight = LCDHEIGHT - y;
    }

    if (tmpWidth == 0 || tmpHeight == 0) {
        return;
    }

    if (tmpWidth < 3 || tmpHeight < 3) {
        task.x = x;
        task.y = y;
        task.height = tmpHeight;
        task.width = tmpWidth;

        addToRenderQueue(task);
        return;
    }

    uint32_t tmpRadius = radius;

    if (tmpRadius >= tmpWidth / 2) {
        tmpRadius = (tmpWidth - 1) / 2u;
    }
    if (tmpRadius >= tmpHeight / 2) {
        tmpRadius = (tmpHeight - 1) / 2u;
    }

    // draw center

    uint32_t innerHeight = tmpHeight - 2 * tmpRadius;
    uint32_t innerWidth = tmpWidth - 2 * tmpRadius;

    task.x = x + tmpRadius;
    task.y = y + tmpRadius;
    task.height = innerHeight;
    task.width = innerWidth;

    addToRenderQueue(task);

    // links
    for (uint32_t i = 0; i < tmpRadius; i++) {
        task.x = x + i;
        task.y = y - i + tmpRadius;
        task.width = 1;
        task.height = innerHeight + i * 2;
        addToRenderQueue(task);
    }
    // rechts
    for (uint32_t i = 0; i < tmpRadius; i++) {
        task.x = x + width - 1 - i;
        task.y = y - i + tmpRadius;
        task.width = 1;
        task.height = innerHeight + i * 2;
        addToRenderQueue(task);
    }

    // oben
    for (uint32_t i = 0; i < tmpRadius; i++) {
        task.x = x + tmpRadius - i;
        task.y = y + i;
        task.width = innerWidth + i * 2;
        task.height = 1;
        addToRenderQueue(task);
    }
    // unten
    for (uint32_t i = 0; i < tmpRadius; i++) {
        task.x = x + tmpRadius - i;
        task.y = y + height - i - 1;
        task.width = innerWidth + i * 2;
        task.height = 1;
        addToRenderQueue(task);
    }
}

void drawString(const std::string &text, uint32_t color, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment) {
    renderTask task;

    task.mode = M2MTRANSPARENT_A4;  // Set DMA2D To copy M2M with Blending
    task.height = activeFont->size; // Set Font Height
    task.color = color;             // Set Font Height

    // Curser Position
    uint32_t posX = x;
    uint32_t posY = y;

    // alignment
    if (alignment != LEFT) {

        uint32_t offset = getStringWidth(text, activeFont);

        // calculate center of current string
        if (alignment == CENTER) {
            posX -= offset / 2; // offset posX to Center
        }
        else if (alignment == RIGHT) {
            posX -= offset; // offset posX for right Right alignment
        }
    }

    // For each Char
    for (const char &c : text) {

        task.x = posX;
        task.y = posY;

        task.width = activeFont->font[(uint32_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint32_t)c - 32].pData; // Pointer to Character

        addToRenderQueue(task); // Add Task to RenderQue

        posX += activeFont->font[(uint32_t)c - 32].XSize; // Distance to next character
    }
}

void copyWaveBuffer(const WaveBuffer &waveBuffer, uint32_t x, uint32_t y) {
    renderTask task;

    task.mode = M2MARGB4444; // Set DMA2D To copy M2M with Blending

    task.x = x;
    task.y = y;

    task.width = waveBuffer.width;   // Character Width
    task.height = waveBuffer.height; // Character Width

    task.pSource = (uint32_t)(uint8_t **)waveBuffer.buffer; // Pointer to Character

    addToRenderQueue(task); // Add Task to RenderQue
}

void copyBitmapToBuffer(const GUI_BITMAP &image, uint32_t color, uint32_t x, uint32_t y) {
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

void drawStringVertical(const std::string &text, uint32_t color, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont,
                        FONTALIGN alignment) {
    renderTask task;

    task.mode = M2MTRANSPARENT_A4;  // Set DMA2D To copy M2M with Blending
    task.height = activeFont->size; // Set Font Height
    task.color = color;             // Set Font Height

    // Curser Position
    uint32_t posX = x;
    uint32_t posY = y;

    if (alignment == CENTER) {
        posY = y - (text.size() * (task.height - 4)) / 2;
    }
    else if (alignment == BOTTOM) {
        posY = y - (text.size() * (task.height - 4));
    }
    // For each Char
    for (const char &c : text) {

        task.x = posX - activeFont->font[(uint32_t)c - 32].XSize / 2;
        task.y = posY;

        task.width = activeFont->font[(uint32_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint32_t)c - 32].pData; // Pointer to Character

        addToRenderQueue(task); // Add Task to RenderQue

        posY += task.height - 4; // Distance to next character
    }
}
void drawString(const char *charArray, uint32_t color, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont,
                FONTALIGN alignment) {
    renderTask task;

    std::string text = charArray;

    task.mode = M2MTRANSPARENT_A4;  // Set DMA2D To copy M2M with Blending
    task.height = activeFont->size; // Set Font Height
    task.color = color;             // Set Font Height

    // Curser Position
    uint32_t posX = x;
    uint32_t posY = y;

    // alignment
    if (alignment != LEFT) {
        uint32_t offset = 0;

        // calculate center of current string
        for (const char &c : text) {
            offset += activeFont->font[(uint32_t)c - 32].XSize;
        }
        if (alignment == CENTER) {
            posX -= offset / 2; // offset posX to Center
        }
        else if (alignment == RIGHT) {
            posX -= offset; // offset posX for right Right alignment
        }
    }

    // For each Char
    for (const char &c : text) {

        task.x = posX;
        task.y = posY;

        task.width = activeFont->font[(uint32_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint32_t)c - 32].pData; // Pointer to Character

        addToRenderQueue(task); // Add Task to RenderQue

        posX += activeFont->font[(uint32_t)c - 32].XSize; // Distance to next character
    }
}

uint32_t getStringWidth(const std::string &text, const GUI_FONTINFO *font) {
    uint32_t offset = 0;

    for (const char &c : text) {
        offset += font->font[(uint32_t)c - 32].XSize;
    }
    return offset;
}

// uint32_t maxCalls = 0;

void addToRenderQueue(renderTask &task) {

    task.outputOffset = LCDWIDTH - task.width;
    task.pTarget = (uint32_t)pFrameBuffer + (LCDWIDTH * task.y + task.x) * 2;

    if (((task.x + task.width) <= LCDWIDTH)       //
        && ((task.y + task.height) <= LCDHEIGHT)  //
        && task.width                             //
        && task.height                            //
        && (renderQueue.size() < MAXDRAWCALLS)) { // in render region and width and height not 0

        renderQueue.push_back(task);
        // maxCalls++;

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
        // println("max calls: ", maxCalls);
        // maxCalls = 0;
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
        hdma2d.LayerCfg[1].InputAlpha = task.color;         /* COLOR */
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A4; /* Foreground format is A4*/

        /* Background layer Configuration */
        hdma2d.LayerCfg[0].InputOffset = task.outputOffset; /* Background input offset*/

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
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565; /* Foreground format is A4*/

        /* Background layer Configuration */
        hdma2d.LayerCfg[0].InputOffset = task.outputOffset; /* Background input offset*/

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

        hdma2d.Init.Mode = DMA2D_M2M_BLEND;

        /* Foreground layer Configuration */
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB4444; /* Foreground format is A4*/

        /* Background layer Configuration */
        hdma2d.LayerCfg[0].InputOffset = task.outputOffset; /* Background input offset*/

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
        hdma2d.Init.Mode = DMA2D_M2M_BLEND_FG;

        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;

        hdma2d.LayerCfg[0].InputOffset = task.outputOffset; /* Background input offset*/

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        hdma2d.Instance->FGPFCCR = (task.color & 0xFF000000) | (hdma2d.Instance->FGPFCCR & 0x00FFFFFF);
        hdma2d.Instance->FGPFCCR = 0x10000 | (hdma2d.Instance->FGPFCCR & 0xFFFCFFFF);

        /* Start DMA2D Task */
        HAL_DMA2D_BlendingStart_IT(&hdma2d, task.color, /* Color value in Register to Memory DMA2D mode */
                                   task.pTarget,        /* DMA2D output buffer */
                                   task.pTarget,        /* DMA2D output buffer */
                                   task.width,          /* width of buffer in pixels */
                                   task.height);        /* height of buffer in lines */
    }
}

inline void drawPixel(WaveBuffer &waveBuffer, uint32_t x, uint32_t y, uint16_t color) {
    if (x < waveBuffer.width && y < waveBuffer.height) // check boundaries
        waveBuffer.buffer[y][x] = color;               // simple "blend"
}

inline void drawPixelAlpha(WaveBuffer &waveBuffer, uint32_t x, uint32_t y, uint16_t color, uint32_t alpha) {
    if (x < waveBuffer.width && y < waveBuffer.height)                       // check boundaries
        waveBuffer.buffer[y][x] = (color & 0x0FFF) | ((alpha & 0x0F) << 12); // simple "blend"
}

inline void drawPixelBlend(WaveBuffer &waveBuffer, uint32_t x, uint32_t y, uint16_t color) {
    if (x < waveBuffer.width && y < waveBuffer.height) { // check boundaries

        uint8_t newA = (color >> 12) & 0x000F;
        uint8_t newR = (color >> 8) & 0x000F;
        uint8_t newG = (color >> 4) & 0x000F;
        uint8_t newB = color & 0x000F;

        uint8_t oldA = (waveBuffer.buffer[y][x] >> 12) & 0x000F;
        uint8_t oldR = (waveBuffer.buffer[y][x] >> 8) & 0x000F;
        uint8_t oldG = (waveBuffer.buffer[y][x] >> 4) & 0x000F;
        uint8_t oldB = waveBuffer.buffer[y][x] & 0x000F;

        uint8_t rOut = (newR * newA / 16) + (oldR * (16 - newA) / 16);
        uint8_t gOut = (newG * newA / 16) + (oldG * (16 - newA) / 16);
        uint8_t bOut = (newB * newA / 16) + (oldB * (16 - newA) / 16);
        uint8_t aOut = newA + (oldA * (16 - newA) / 16);
        uint16_t mixedColor = (aOut & 0x000F) << 12 | (rOut & 0x000F) << 8 | (gOut & 0x000F) << 4 | (bOut & 0x000F);

        waveBuffer.buffer[y][x] = mixedColor;
    }
}

inline void drawPixelBlendFast(WaveBuffer &waveBuffer, uint32_t x, uint32_t y, uint16_t color) {

    if (x < waveBuffer.width && y < waveBuffer.height) // check boundaries
        waveBuffer.buffer[y][x] |= color;              // simple "blend"
}

void drawLine(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t color) {
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

void drawLineAA(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t color) {

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2, x2; /* error value e_xy */
    int ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (;;) { /* pixel loop */
        drawPixelAlpha(waveBuffer, x0, y0, color, 15 - 15 * abs(err - dx + dy) / ed);
        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            if (x0 == x1)
                break;
            if (e2 + dy < ed)
                drawPixelAlpha(waveBuffer, x0, y0 + sy, color, 15 - 15 * (e2 + dy) / ed);
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            if (y0 == y1)
                break;
            if (dx - e2 < ed)
                drawPixelAlpha(waveBuffer, x2 + sx, y0, color, 15 - 15 * (dx - e2) / ed);
            err += dx;
            y0 += sy;
        }
    }
}

// Bresenham's line algorithm
void drawLineThick(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, uint16_t color) {

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */

        drawPixelThick(waveBuffer, x0, y0, color);

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

void plotQuadBezierSeg(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, int x2, int y2,
                       uint16_t color) { /* plot a limited quadratic Bezier segment */
    int sx = x2 - x1, sy = y2 - y1;
    long xx = x0 - x1, yy = y0 - y1, xy;         /* relative values for checks */
    double dx, dy, err, cur = xx * sy - yy * sx; /* curvature */

    if (sx * (long)sx + sy * (long)sy > xx * xx + yy * yy) { /* begin with longer part */
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur; /* swap P0 P2 */
    }
    if (cur != 0) { /* no straight line */
        xx += sx;
        xx *= sx = x0 < x2 ? 1 : -1; /* x step direction */
        yy += sy;
        yy *= sy = y0 < y2 ? 1 : -1; /* y step direction */
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;                /* differences 2nd degree */
        if (cur * sx * sy < 0) { /* negated curvature? */
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }
        dx = 4.0 * sy * cur * (x1 - x0) + xx - xy; /* differences 1st degree */
        dy = 4.0 * sx * cur * (y0 - y1) + yy - xy;
        xx += xx;
        yy += yy;
        err = dx + dy + xy; /* error 1st step */
        do {
            drawPixelThick(waveBuffer, x0, y0, color); /* plot curve */
            if (x0 == x2 && y0 == y2)
                return;        /* last pixel -> curve finished */
            y1 = 2 * err < dx; /* save value for test of y step */
            if (2 * err > dy) {
                x0 += sx;
                dx -= xy;
                err += dy += yy;
            } /* x step */
            if (y1) {
                y0 += sy;
                dy -= xy;
                err += dx += xx;
            }                       /* y step */
        } while (dy < 0 && dx > 0); /* gradient negates -> algorithm fails */
    }
    drawLineThick(waveBuffer, x0, y0, x2, y2, color); /* plot remaining part to end */
}

void drawQuadBezier(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, int x2, int y2,
                    uint16_t color) { /* plot any quadratic Bezier curve */
    int x = x0 - x1, y = y0 - y1;
    double t = x0 - 2 * x1 + x2, r;

    if ((long)x * (x2 - x1) > 0) {                           /* horizontal cut at P4? */
        if ((long)y * (y2 - y1) > 0)                         /* vertical cut at P6 too? */
            if (fabs((y0 - 2 * y1 + y2) / t * x) > abs(y)) { /* which first? */
                x0 = x2;
                x2 = x + x1;
                y0 = y2;
                y2 = y + y1; /* swap points */
            }                /* now horizontal cut at P4 comes first */
        t = (x0 - x1) / t;
        r = (1 - t) * ((1 - t) * y0 + 2.0 * t * y1) + t * t * y2; /* By(t=P4) */
        t = (x0 * x2 - x1 * x1) * t / (x0 - x1);                  /* gradient dP4/dx=0 */
        x = floor(t + 0.5);
        y = floor(r + 0.5);
        r = (y1 - y0) * (t - x0) / (x1 - x0) + y0; /* intersect P3 | P0 P1 */
        plotQuadBezierSeg(waveBuffer, x0, y0, x, floor(r + 0.5), x, y, color);
        r = (y1 - y2) * (t - x2) / (x1 - x2) + y2; /* intersect P4 | P1 P2 */
        x0 = x1 = x;
        y0 = y;
        y1 = floor(r + 0.5); /* P0 = P4, P1 = P8 */
    }
    if ((long)(y0 - y1) * (y2 - y1) > 0) { /* vertical cut at P6? */
        t = y0 - 2 * y1 + y2;
        t = (y0 - y1) / t;
        r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2; /* Bx(t=P6) */
        t = (y0 * y2 - y1 * y1) * t / (y0 - y1);                  /* gradient dP6/dy=0 */
        x = floor(r + 0.5);
        y = floor(t + 0.5);
        r = (x1 - x0) * (t - y0) / (y1 - y0) + x0; /* intersect P6 | P0 P1 */
        plotQuadBezierSeg(waveBuffer, x0, y0, floor(r + 0.5), y, x, y, color);
        r = (x1 - x2) * (t - y2) / (y1 - y2) + x2; /* intersect P7 | P1 P2 */
        x0 = x;
        x1 = floor(r + 0.5);
        y0 = y1 = y; /* P0 = P6, P1 = P7 */
    }
    plotQuadBezierSeg(waveBuffer, x0, y0, x1, y1, x2, y2, color); /* remaining part */
}

// irgendwas stimmt da nicht...
void drawLineWidth(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, float wd,
                   uint16_t color) { /* plot an anti-aliased line of width wd */
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2, x2, y2; /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (wd = (wd + 1) / 2;;) { /* pixel loop */
        drawPixelAlpha(waveBuffer, x0, y0, color, 15 - std::max(0.f, 15 * (abs(err - dx + dy) / ed - wd + 1)));

        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            for (e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx)
                drawPixelAlpha(waveBuffer, x0, y2 += sy, color, 15 - std::max(0.f, 15 * (abs(e2) / ed - wd + 1)));

            if (x0 == x1)
                break;
            e2 = err;
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            for (e2 = dx - e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy)
                drawPixelAlpha(waveBuffer, x2 += sx, y0, color, 15 - std::max(0.f, 15 * (abs(e2) / ed - wd + 1)));

            if (y0 == y1)
                break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawFilledCircle(WaveBuffer &waveBuffer, int x0, int y0, uint16_t color, float r) {
    uint16_t colorAA;
    for (float y = 0; y < 2.0 * r; y++) {
        for (float x = 0; x < 2 * r; x++) {
            float deltaX = r - x;
            float deltaY = r - y;
            float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            distance = std::clamp(r - distance, 0.f, 1.f);
            colorAA = (uint16_t((color >> 12) * distance) << 12) | (color & 0x0FFF);

            drawPixelBlend(waveBuffer, x + x0 - r, y + y0 - r, colorAA);
        }
    }
}

void plotCubicBezierSeg(WaveBuffer &waveBuffer, int x0, int y0, float x1, float y1, float x2, float y2, int x3, int y3,
                        uint16_t color) { /* plot limited cubic Bezier segment */
    int f, fx, fy, leg = 1;
    int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1; /* step direction */
    float xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx * (x1 - x2), xb = sx * (x0 - x1 - x2 + x3);
    float yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy * (y1 - y2), yb = sy * (y0 - y1 - y2 + y3);
    double ab, ac, bc, cb, xx, xy, yy, dx, dy, ex, *pxy, EP = 0.01;

    if (xa == 0 && ya == 0) { /* quadratic Bezier */
        sx = floor((3 * x1 - x0 + 1) / 2);
        sy = floor((3 * y1 - y0 + 1) / 2); /* new midpoint */
        return plotQuadBezierSeg(waveBuffer, x0, y0, sx, sy, x3, y3, color);
    }
    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1; /* line lengths */
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;
    do { /* loop over both ends */
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ex = ab * (ab + ac - 3 * bc) + ac * ac; /* P0 part of self-intersection loop? */
        f = ex > 0 ? 1 : sqrt(1 + 1024 / x1);   /* calculate resolution */
        ab *= f;
        ac *= f;
        bc *= f;
        ex *= f * f; /* increase resolution */
        xy = 9 * (ab + ac + bc) / 8;
        cb = 8 * (xa - ya); /* init differences of 1st degree */
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);
        /* init differences of 2nd degree */
        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * cb)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * cb)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + cb);
        ac = ya * ya;
        cb = xa * xa;
        xy = 3 * (xy + 9 * f * (cb * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0) { /* negate values if inside self-intersection loop */
            dx = -dx;
            dy = -dy;
            xx = -xx;
            yy = -yy;
            xy = -xy;
            ac = -ac;
            cb = -cb;
        } /* init differences of 3rd degree */
        ab = 6 * ya * ac;
        ac = -6 * xa * ac;
        bc = 6 * ya * cb;
        cb = -6 * xa * cb;
        dx += xy;
        ex = dx + dy;
        dy += xy; /* error of 1st step */

        for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3;) {
            drawPixelThick(waveBuffer, x0, y0, color); /* plot curve */
            do {                                       /* move sub-steps of one pixel */
                if (dx > *pxy || dy < *pxy)
                    goto exit;      /* confusing values */
                y1 = 2 * ex - dy;   /* save value for test of y step */
                if (2 * ex >= dx) { /* x sub-step */
                    fx--;
                    ex += dx += xx;
                    dy += xy += ac;
                    yy += bc;
                    xx += ab;
                }
                if (y1 <= 0) { /* y sub-step */
                    fy--;
                    ex += dy += yy;
                    dx += xy += bc;
                    xx += ac;
                    yy += cb;
                }
            } while (fx > 0 && fy > 0); /* pixel complete? */
            if (2 * fx <= f) {
                x0 += sx;
                fx += f;
            } /* x step */
            if (2 * fy <= f) {
                y0 += sy;
                fy += f;
            } /* y step */
            if (pxy == &xy && dx < 0 && dy > 0)
                pxy = &EP; /* pixel ahead valid */
        }
    exit:
        xx = x0;
        x0 = x3;
        x3 = xx;
        sx = -sx;
        xb = -xb; /* swap legs */
        yy = y0;
        y0 = y3;
        y3 = yy;
        sy = -sy;
        yb = -yb;
        x1 = x2;
    } while (leg--);                                  /* try other end */
    drawLineThick(waveBuffer, x0, y0, x3, y3, color); /* remaining part in case of cusp or crunode */
}

void plotCubicBezier(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3,
                     uint16_t color) { /* plot any cubic Bezier curve */
    int n = 0, i = 0;
    long xc = x0 + x1 - x2 - x3, xa = xc - 4 * (x1 - x2);
    long xb = x0 - x1 - x2 + x3, xd = xb + 4 * (x1 + x2);
    long yc = y0 + y1 - y2 - y3, ya = yc - 4 * (y1 - y2);
    long yb = y0 - y1 - y2 + y3, yd = yb + 4 * (y1 + y2);
    float fx0 = x0, fx1, fx2, fx3, fy0 = y0, fy1, fy2, fy3;
    double t1 = xb * xb - xa * xc, t2, t[5];
    /* sub-divide curve at gradient sign changes */
    if (xa == 0) { /* horizontal */
        if (abs(xc) < 2 * abs(xb))
            t[n++] = xc / (2.0 * xb); /* one change */
    }
    else if (t1 > 0.0) { /* two changes */
        t2 = sqrt(t1);
        t1 = (xb - t2) / xa;
        if (fabs(t1) < 1.0)
            t[n++] = t1;
        t1 = (xb + t2) / xa;
        if (fabs(t1) < 1.0)
            t[n++] = t1;
    }
    t1 = yb * yb - ya * yc;
    if (ya == 0) { /* vertical */
        if (abs(yc) < 2 * abs(yb))
            t[n++] = yc / (2.0 * yb); /* one change */
    }
    else if (t1 > 0.0) { /* two changes */
        t2 = sqrt(t1);
        t1 = (yb - t2) / ya;
        if (fabs(t1) < 1.0)
            t[n++] = t1;
        t1 = (yb + t2) / ya;
        if (fabs(t1) < 1.0)
            t[n++] = t1;
    }
    for (i = 1; i < n; i++) /* bubble sort of 4 points */
        if ((t1 = t[i - 1]) > t[i]) {
            t[i - 1] = t[i];
            t[i] = t1;
            i = 0;
        }

    t1 = -1.0;
    t[n] = 1.0;                /* begin / end point */
    for (i = 0; i <= n; i++) { /* plot each segment separately */
        t2 = t[i];             /* sub-divide at t[i-1], t[i] */
        fx1 = (t1 * (t1 * xb - 2 * xc) - t2 * (t1 * (t1 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
        fy1 = (t1 * (t1 * yb - 2 * yc) - t2 * (t1 * (t1 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
        fx2 = (t2 * (t2 * xb - 2 * xc) - t1 * (t2 * (t2 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
        fy2 = (t2 * (t2 * yb - 2 * yc) - t1 * (t2 * (t2 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
        fx0 -= fx3 = (t2 * (t2 * (3 * xb - t2 * xa) - 3 * xc) + xd) / 8;
        fy0 -= fy3 = (t2 * (t2 * (3 * yb - t2 * ya) - 3 * yc) + yd) / 8;
        x3 = floor(fx3 + 0.5);
        y3 = floor(fy3 + 0.5); /* scale bounds to int */
        if (fx0 != 0.0) {
            fx1 *= fx0 = (x0 - x3) / fx0;
            fx2 *= fx0;
        }
        if (fy0 != 0.0) {
            fy1 *= fy0 = (y0 - y3) / fy0;
            fy2 *= fy0;
        }
        if (x0 != x3 || y0 != y3) /* segment t1 - t2 */
            plotCubicBezierSeg(waveBuffer, x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3, color);
        x0 = x3;
        y0 = y3;
        fx0 = fx3;
        fy0 = fy3;
        t1 = t2;
    }
}

void plotQuadBezier(WaveBuffer &waveBuffer, int x0, int y0, int x1, int y1, int x2, int y2,
                    uint16_t color) { /* plot any quadratic Bezier curve */
    int x = x0 - x1, y = y0 - y1;
    double t = x0 - 2 * x1 + x2, r;

    if ((long)x * (x2 - x1) > 0) {                           /* horizontal cut at P4? */
        if ((long)y * (y2 - y1) > 0)                         /* vertical cut at P6 too? */
            if (fabs((y0 - 2 * y1 + y2) / t * x) > abs(y)) { /* which first? */
                x0 = x2;
                x2 = x + x1;
                y0 = y2;
                y2 = y + y1; /* swap points */
            }                /* now horizontal cut at P4 comes first */
        t = (x0 - x1) / t;
        r = (1 - t) * ((1 - t) * y0 + 2.0 * t * y1) + t * t * y2; /* By(t=P4) */
        t = (x0 * x2 - x1 * x1) * t / (x0 - x1);                  /* gradient dP4/dx=0 */
        x = floor(t + 0.5);
        y = floor(r + 0.5);
        r = (y1 - y0) * (t - x0) / (x1 - x0) + y0; /* intersect P3 | P0 P1 */
        plotQuadBezierSeg(waveBuffer, x0, y0, x, floor(r + 0.5), x, y, color);
        r = (y1 - y2) * (t - x2) / (x1 - x2) + y2; /* intersect P4 | P1 P2 */
        x0 = x1 = x;
        y0 = y;
        y1 = floor(r + 0.5); /* P0 = P4, P1 = P8 */
    }
    if ((long)(y0 - y1) * (y2 - y1) > 0) { /* vertical cut at P6? */
        t = y0 - 2 * y1 + y2;
        t = (y0 - y1) / t;
        r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2; /* Bx(t=P6) */
        t = (y0 * y2 - y1 * y1) * t / (y0 - y1);                  /* gradient dP6/dy=0 */
        x = floor(r + 0.5);
        y = floor(t + 0.5);
        r = (x1 - x0) * (t - y0) / (y1 - y0) + x0; /* intersect P6 | P0 P1 */
        plotQuadBezierSeg(waveBuffer, x0, y0, floor(r + 0.5), y, x, y, color);
        r = (x1 - x2) * (t - y2) / (y1 - y2) + x2; /* intersect P7 | P1 P2 */
        x0 = x;
        x1 = floor(r + 0.5);
        y0 = y1 = y; /* P0 = P6, P1 = P7 */
    }
    plotQuadBezierSeg(waveBuffer, x0, y0, x1, y1, x2, y2, color); /* remaining part */
}

void drawCubicSpline(WaveBuffer &waveBuffer, int n, int x[], int y[],
                     uint16_t color) { /* plot cubic spline, destroys input arrays x,y */
#define M_MAX 6
    float mi = 0.25, m[M_MAX]; /* diagonal constants of matrix */
    int x3 = x[n - 1], y3 = y[n - 1], x4 = x[n], y4 = y[n];
    int i, x0, y0, x1, y1, x2, y2;

    x[1] = x0 = 12 * x[1] - 3 * x[0]; /* first row of matrix */
    y[1] = y0 = 12 * y[1] - 3 * y[0];

    for (i = 2; i < n; i++) { /* foreward sweep */
        if (i - 2 < M_MAX)
            m[i - 2] = mi = 0.25 / (2.0 - mi);
        x[i] = x0 = floor(12 * x[i] - 2 * x0 * mi + 0.5);
        y[i] = y0 = floor(12 * y[i] - 2 * y0 * mi + 0.5);
    }
    x2 = floor((x0 - 3 * x4) / (7 - 4 * mi) + 0.5); /* correct last row */
    y2 = floor((y0 - 3 * y4) / (7 - 4 * mi) + 0.5);
    plotCubicBezier(waveBuffer, x3, y3, (x2 + x4) / 2, (y2 + y4) / 2, x4, y4, x4, y4, color);

    if (n - 3 < M_MAX)
        mi = m[n - 3];
    x1 = floor((x[n - 2] - 2 * x2) * mi + 0.5);
    y1 = floor((y[n - 2] - 2 * y2) * mi + 0.5);
    for (i = n - 3; i > 0; i--) { /* back substitution */
        if (i <= M_MAX)
            mi = m[i - 1];
        x0 = floor((x[i] - 2 * x1) * mi + 0.5);
        y0 = floor((y[i] - 2 * y1) * mi + 0.5);
        x4 = floor((x0 + 4 * x1 + x2 + 3) / 6.0); /* reconstruct P[i] */
        y4 = floor((y0 + 4 * y1 + y2 + 3) / 6.0);
        plotCubicBezier(waveBuffer, x4, y4, floor((2 * x1 + x2) / 3 + 0.5), floor((2 * y1 + y2) / 3 + 0.5),
                        floor((x1 + 2 * x2) / 3 + 0.5), floor((y1 + 2 * y2) / 3 + 0.5), x3, y3, color);
        x3 = x4;
        y3 = y4;
        x2 = x1;
        y2 = y1;
        x1 = x0;
        y1 = y0;
    }
    x0 = x[0];
    x4 = floor((3 * x0 + 7 * x1 + 2 * x2 + 6) / 12.0); /* reconstruct P[1] */
    y0 = y[0];
    y4 = floor((3 * y0 + 7 * y1 + 2 * y2 + 6) / 12.0);
    plotCubicBezier(waveBuffer, x4, y4, floor((2 * x1 + x2) / 3 + 0.5), floor((2 * y1 + y2) / 3 + 0.5),
                    floor((x1 + 2 * x2) / 3 + 0.5), floor((y1 + 2 * y2) / 3 + 0.5), x3, y3, color);
    plotCubicBezier(waveBuffer, x0, y0, x0, y0, (x0 + x1) / 2, (y0 + y1) / 2, x4, y4, color);
}

#endif // ifdef POLYCONTROL