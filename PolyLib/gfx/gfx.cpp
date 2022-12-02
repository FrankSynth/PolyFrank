#ifdef POLYCONTROL

#include "gfx.hpp"
#include "datacore/datalocation.hpp"
#include <algorithm>

volatile FRAMEBUFFER_A ALIGN_32BYTES(uint8_t FrameBufferA[FRAMEBUFFERSIZE]);
volatile FRAMEBUFFER_B ALIGN_32BYTES(uint8_t FrameBufferB[FRAMEBUFFERSIZE]);

volatile uint32_t dma2d_TransferComplete_Flag = 0;

uint8_t *pFrameBuffer;

RENDERSTATE renderState = RENDER_DONE;

RAM1 CircularBuffer<renderTask, MAXDRAWCALLS> renderQueue;
RAM1 ALIGN_32BYTES(uint16_t waveformBuffer[WAVEFORMHEIGHT][LCDWIDTH - 50]);
RAM2 ALIGN_32BYTES(uint16_t waveformQuickBuffer[WAVEFORMQUICKHEIGHT][LCDWIDTH - 50]);

WaveBuffer waveBuffer;
WaveBuffer waveQuickBuffer;

void GFX_Init() {
    // set handles
    MX_LTDC_Init();
    SwitchFrameBuffer();

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

////////////GFX////////////

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

void drawRectangleCentered(uint32_t color, uint32_t radius, uint32_t x, uint32_t y) {
    if (radius == 0)
        return;

    uint32_t xpos = x - radius;
    uint32_t ypos = y - radius;

    drawRectangleFill(color, xpos, ypos, radius * 2, radius * 2);
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

void drawStringSoftware(const std::string &text, uint32_t x, uint32_t y, const GUI_FONTINFO *activeFont) {
    renderTask task;
    task.mode = SOFT_M2M; // Set DMA2D To copy M2M with Blending

    task.height = activeFont->size; // Set Font Height

    // Curser Position
    uint32_t posX = x;
    uint32_t posY = y;

    // For each Char
    for (const char &c : text) {

        task.x = posX;
        task.y = posY;

        task.width = activeFont->font[(uint32_t)c - 32].BytesPerLine * 2;  // Character Width
        task.pSource = (uint32_t)activeFont->font[(uint32_t)c - 32].pData; // Pointer to Character

        renderSoftware(task); // Add Task to RenderQue

        posX += activeFont->font[(uint32_t)c - 32].XSize; // Distance to next character
    }
}

void clearFrameBuffer() {
    int color = 0x00000000;
    memset((int32_t *)FrameBufferA, color, (LCDWIDTH * LCDHEIGHT) * 2);
    memset((int32_t *)FrameBufferB, color, (LCDWIDTH * LCDHEIGHT) * 2);
};

void renderSoftware(renderTask &task) {
    task.outputOffset = LCDWIDTH - task.width;
    task.pTarget = (uint32_t)pFrameBuffer + (LCDWIDTH * task.y + task.x) * 2;

    if (((task.x + task.width) <= LCDWIDTH)       //
        && ((task.y + task.height) <= LCDHEIGHT)  //
        && task.width                             //
        && task.height                            //
        && (renderQueue.size() < MAXDRAWCALLS)) { // in render region and width and height not 0

        if (task.mode == SOFT_M2M) {
            for (uint32_t y = 0; y < task.height; y++) {

                for (uint32_t x = 0; x < task.width; x++) {
                    uint32_t indexSource = y * task.width / 2 + x / 2;
                    uint32_t indexTarget = y * LCDWIDTH + x;

                    if ((((uint8_t *)task.pSource)[indexSource] & 0x0F) > 0x8 ||
                        (((uint8_t *)task.pSource)[indexSource] & 0xF0) > 0x80) {
                        ((uint16_t *)task.pTarget)[indexTarget] = 0xFFFF;
                    }
                }
            }
        }
    }
}

void copyWaveBuffer(WaveBuffer &buffer, uint32_t x, uint32_t y) {
    renderTask task;

    task.mode = M2MARGB4444; // Set DMA2D To copy M2M with Blending

    task.x = x;
    task.y = y;

    task.width = buffer.width;   // Character Width
    task.height = buffer.height; // Character Width

    task.pSource = (uint32_t)(uint8_t **)(*buffer.buffer); // Pointer to Character

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

////////////DMA2D////////////

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

    // // DMA2D//
    // HAL_DMA2D_DeInit(&hdma2d); // de init dma2d

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

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

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

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

        /* Start DMA2D Task */
        HAL_DMA2D_BlendingStart_IT(&hdma2d, task.pSource, /* Color value in Register to Memory DMA2D mode */
                                   task.pTarget,          /* DMA2D src2 */
                                   task.pTarget,          /* DMA2D output buffer */
                                   task.width,            /* width of buffer in pixels */
                                   task.height);
    }

    else if (task.mode == R2M) { // Register to Memoryif (lastDrawType != task.mode) {
        hdma2d.Init.Mode = DMA2D_M2M_BLEND_FG;

        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;

        hdma2d.LayerCfg[0].InputOffset = task.outputOffset; /* Background input offset*/

        /* Apply DMA2D Background configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 0);

        /* Apply DMA2D Foreground configuration */
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        /* Init DMA2D */
        HAL_DMA2D_Init(&hdma2d);

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

////////////Software GFX ////////////

inline void drawPixel(WaveBuffer &buffer, uint32_t x, uint32_t y, uint16_t color) {
    if (x < buffer.width && y < buffer.height) // check boundaries
        (*buffer.buffer)[y][x] = color;        // simple "blend"
}

inline void drawPixelAlpha(WaveBuffer &buffer, uint32_t x, uint32_t y, uint16_t color, uint32_t alpha) {
    if (x < buffer.width && y < buffer.height)                              // check boundaries
        (*buffer.buffer)[y][x] = (color & 0x0FFF) | ((alpha & 0x0F) << 12); // simple "blend"
}

inline void drawPixelBlend(WaveBuffer &buffer, uint32_t x, uint32_t y, uint16_t color) {
    if (x < buffer.width && y < buffer.height) { // check boundaries

        uint8_t newA = (color >> 12) & 0x000F;
        uint8_t newR = (color >> 8) & 0x000F;
        uint8_t newG = (color >> 4) & 0x000F;
        uint8_t newB = color & 0x000F;

        uint8_t oldA = ((*buffer.buffer)[y][x] >> 12) & 0x000F;
        uint8_t oldR = ((*buffer.buffer)[y][x] >> 8) & 0x000F;
        uint8_t oldG = ((*buffer.buffer)[y][x] >> 4) & 0x000F;
        uint8_t oldB = (*buffer.buffer)[y][x] & 0x000F;

        uint8_t rOut = (newR * newA / 16) + (oldR * (16 - newA) / 16);
        uint8_t gOut = (newG * newA / 16) + (oldG * (16 - newA) / 16);
        uint8_t bOut = (newB * newA / 16) + (oldB * (16 - newA) / 16);
        uint8_t aOut = newA + (oldA * (16 - newA) / 16);
        uint16_t mixedColor = (aOut & 0x000F) << 12 | (rOut & 0x000F) << 8 | (gOut & 0x000F) << 4 | (bOut & 0x000F);

        (*buffer.buffer)[y][x] = mixedColor;
    }
}

void drawLine(WaveBuffer &buffer, int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */
        drawPixel(buffer, x0, y0, color);
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

void drawLineAA(WaveBuffer &buffer, int x0, int y0, int x1, int y1, uint16_t color) {

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2, x2; /* error value e_xy */
    int ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (;;) { /* pixel loop */
        drawPixelAlpha(buffer, x0, y0, color, 15 - 15 * abs(err - dx + dy) / ed);
        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            if (x0 == x1)
                break;
            if (e2 + dy < ed)
                drawPixelAlpha(buffer, x0, y0 + sy, color, 15 - 15 * (e2 + dy) / ed);
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            if (y0 == y1)
                break;
            if (dx - e2 < ed)
                drawPixelAlpha(buffer, x2 + sx, y0, color, 15 - 15 * (dx - e2) / ed);
            err += dx;
            y0 += sy;
        }
    }
}

// Bresenham's line algorithm
void drawLineThick(WaveBuffer &buffer, int x0, int y0, int x1, int y1, uint16_t color) {

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */

        drawPixelThick(buffer, x0, y0, color);

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

void plotQuadBezierSeg(WaveBuffer &buffer, int x0, int y0, int x1, int y1, int x2, int y2,
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
            drawPixelThick(buffer, x0, y0, color); /* plot curve */
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
    drawLineThick(buffer, x0, y0, x2, y2, color); /* plot remaining part to end */
}

void drawQuadBezier(WaveBuffer &buffer, int x0, int y0, int x1, int y1, int x2, int y2,
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
        plotQuadBezierSeg(buffer, x0, y0, x, floor(r + 0.5), x, y, color);
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
        plotQuadBezierSeg(buffer, x0, y0, floor(r + 0.5), y, x, y, color);
        r = (x1 - x2) * (t - y2) / (y1 - y2) + x2; /* intersect P7 | P1 P2 */
        x0 = x;
        x1 = floor(r + 0.5);
        y0 = y1 = y; /* P0 = P6, P1 = P7 */
    }
    plotQuadBezierSeg(buffer, x0, y0, x1, y1, x2, y2, color); /* remaining part */
}

// irgendwas stimmt da nicht...
void drawLineWidth(WaveBuffer &buffer, int x0, int y0, int x1, int y1, float wd,
                   uint16_t color) { /* plot an anti-aliased line of width wd */
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2, x2, y2; /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (wd = (wd + 1) / 2;;) { /* pixel loop */
        drawPixelAlpha(buffer, x0, y0, color, 15 - std::max(0.f, 15 * (abs(err - dx + dy) / ed - wd + 1)));

        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            for (e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx)
                drawPixelAlpha(buffer, x0, y2 += sy, color, 15 - std::max(0.f, 15 * (abs(e2) / ed - wd + 1)));

            if (x0 == x1)
                break;
            e2 = err;
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            for (e2 = dx - e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy)
                drawPixelAlpha(buffer, x2 += sx, y0, color, 15 - std::max(0.f, 15 * (abs(e2) / ed - wd + 1)));

            if (y0 == y1)
                break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawFilledCircle(WaveBuffer &buffer, int x0, int y0, uint16_t color, float r) {
    uint16_t colorAA;
    for (float y = 0; y < 2.0 * r; y++) {
        for (float x = 0; x < 2 * r; x++) {
            float deltaX = r - x;
            float deltaY = r - y;
            float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            distance = std::clamp(r - distance, 0.f, 1.f);
            colorAA = (uint16_t((color >> 12) * distance) << 12) | (color & 0x0FFF);

            drawPixelBlend(buffer, x + x0 - r, y + y0 - r, colorAA);
        }
    }
}

void plotQuadBezier(WaveBuffer &buffer, int x0, int y0, int x1, int y1, int x2, int y2,
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
        plotQuadBezierSeg(buffer, x0, y0, x, floor(r + 0.5), x, y, color);
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
        plotQuadBezierSeg(buffer, x0, y0, floor(r + 0.5), y, x, y, color);
        r = (x1 - x2) * (t - y2) / (y1 - y2) + x2; /* intersect P7 | P1 P2 */
        x0 = x;
        x1 = floor(r + 0.5);
        y0 = y1 = y; /* P0 = P6, P1 = P7 */
    }
    plotQuadBezierSeg(buffer, x0, y0, x1, y1, x2, y2, color); /* remaining part */
}

#endif // ifdef POLYCONTROL