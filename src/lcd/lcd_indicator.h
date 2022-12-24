#ifndef LCD_INDICATOR_H
#define LCD_INDICATOR_H

#include <stdio.h>
#include <math.h>
#include "lcd.h"

typedef struct 
{
    uint16_t x0;
    uint16_t y0;
    uint16_t h;
    uint16_t w;
    float val;
    ColorRGBByte bg;

} Indicator;

Indicator lcd_indicator_init(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, ColorRGBByte bg);
void lcd_indicator_draw(Indicator* ind, float value, ColorRGBByte c);
#endif

