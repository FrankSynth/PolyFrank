#pragma once

#define RAM1 __attribute__((section(".ram_D1_buffer")))
#define FRAMEBUFFER __attribute__((section(".frame_buffer")))
// #define SDRAM __attribute__((section(".sdram_buffer")))
#define RAM2 __attribute__((section(".ram_D2_buffer")))
#define RAM2_DMA __attribute__((section(".ram_D2_DMA_buffer")))
#define RAM3 __attribute__((section(".ram_D3_buffer")))
