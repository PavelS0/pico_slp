#ifndef LCD_GRAPH_H
#define LCD_GRAPH_H

#include <stdio.h>
#include <math.h>
#include "lcd.h"

typedef struct 
{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
} GraphConfig;

typedef struct 
{
    uint16_t x0;
    uint16_t y0;
    uint16_t h;
    uint16_t w;
    ColorRGBByte bg;
} Graph;

Graph lcd_graph_init(uint16_t size, uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, ColorRGBByte bg, uint8_t dma_chan);
void lcd_graph_draw_signed(Graph* g, int16_t* buf, uint16_t size, ColorRGBByte c);
void lcd_graph_draw_unsigned(Graph* g, uint16_t* buf, uint16_t size, ColorRGBByte c);
#endif

