#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

#define COLOR_OFF       0x00
#define COLOR_DIM_RED   0x01
#define COLOR_DIM_GREEN 0x02
#define COLOR_DIM_BLUE  0x03

void ClearFramebuffer(void);
void DrawRow(int row, uint8_t color);
void DrawVerticalSegment(int row, uint8_t color, int start_col, int length);
void DrawRectangle(int x, int y, int h, int w, uint8_t color);
void FlushFramebuffer(void);

#endif
