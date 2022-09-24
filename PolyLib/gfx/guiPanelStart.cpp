#ifdef POLYCONTROL

#define SEGMENTSSIZE 20
#include "guiPanelStart.hpp"

void GUIPanelStart::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelStart::Draw() {

    static uint32_t segmentRadius = SEGMENTSSIZE / 2;
    static float segmentOffset[LCDHEIGHT / SEGMENTSSIZE][LCDWIDTH / SEGMENTSSIZE];
    static float randomOffset[LCDHEIGHT / SEGMENTSSIZE][LCDWIDTH / SEGMENTSSIZE];
    // static uint32_t randomColor[LCDHEIGHT / SEGMENTSSIZE][LCDWIDTH / SEGMENTSSIZE];

    static bool first = 1;

    static elapsedMillis fadeTimer = 0;

    static float fade = 0;

    static uint32_t color[3];

    static uint32_t backgroundColor = cBlack;

    if (first) {
        first = 0;
        for (uint32_t y = 0; y < LCDHEIGHT / SEGMENTSSIZE; y++) {
            for (uint32_t x = 0; x < LCDWIDTH / SEGMENTSSIZE; x++) {

                float offsetX = (float)x - (float)((LCDWIDTH / SEGMENTSSIZE) / 2) + 0.5;
                float offsetY = (float)y - (float)((LCDHEIGHT / SEGMENTSSIZE) / 2) + 0.5;

                segmentOffset[y][x] = sqrt(pow(offsetX, 2) + pow(offsetY, 2)) / (float)(LCDWIDTH / SEGMENTSSIZE);

                randomOffset[y][x] = ((float)rand() / RAND_MAX - 0.5) / 3;
            }
        }
    }

    if (fadeTimer < 500) {
        fade = (float)fadeTimer / 500.;
    }
    if (fadeTimer > 2100) {
        fade = 1 - (float)(-2100 + (int32_t)fadeTimer) / 500.;

        if (fade < 0)
            fade = 0;
    }

    if (fadeTimer > 1000) {
        int16_t fadeInt;
        uint8_t alpha;

        fadeInt = (-1000 + (int32_t)fadeTimer) / 2;
        if (fadeInt > 255)
            fadeInt = 255;

        alpha = (uint8_t)(255 - fadeInt);
        // println("  fadeInt : ", fadeInt, "  Alpha : ", alpha);

        backgroundColor = (cBackground & 0x00FFFFFF) | uint32_t(alpha) << 24;
    }

    drawRectangleFill(backgroundColor, 0, 0, LCDWIDTH, LCDHEIGHT);

    for (uint32_t y = 0; y < LCDHEIGHT / SEGMENTSSIZE; y++) {
        for (uint32_t x = 0; x < LCDWIDTH / SEGMENTSSIZE; x++) {
            int32_t radius =
                fade * segmentRadius *

                (fast_sin_f32(-(float)millis() / 2500 + 250 + segmentOffset[y][x] * 0.5 + randomOffset[y][x]) + 1) / 2;

            drawRectangleCentered(cHighlight, uint32_t(radius), x * SEGMENTSSIZE + segmentRadius,
                                  y * SEGMENTSSIZE + segmentRadius);
            /* code */
        }
    }
}

#endif