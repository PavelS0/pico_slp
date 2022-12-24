#ifndef LCD_PRIMITIVES_H
#define LCD_PRIMITIVES_H

#include <stdio.h>
#include "lcd.h"
void lcd_prim_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorRGBByte c);
void lcd_prim_fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorRGBByte c);

#endif