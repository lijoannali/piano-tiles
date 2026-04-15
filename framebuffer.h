#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

#define COLOR_OFF     0x00
#define COLOR_DIM_RED 0x01
#define COLOR_DIM_BLUE 0x10
#define COLOR_DIM_GREEN 0x11

void ClearFramebuffer(void);
void DrawRow(int row, uint8_t color);
void FlushFramebuffer(void);

#endif
