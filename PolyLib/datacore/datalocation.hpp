#pragma once

#define RAM1 __attribute__((section(".ram_D1_buffer")))
#define RAM2 __attribute__((section(".ram_D2_buffer")))
#define RAM2_DMA __attribute__((section(".ram_D2_DMA_buffer")))
#define RAM3 __attribute__((section(".ram_D3_buffer")))
#define RAM3_DMA __attribute__((section(".ram_D3_DMA_buffer")))
#ifdef POLYCONTROL
#define FRAMEBUFFER_A __attribute__((section(".frame_buffer_a")))
#define FRAMEBUFFER_B __attribute__((section(".frame_buffer_b")))
#endif