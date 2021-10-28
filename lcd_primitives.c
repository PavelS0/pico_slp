#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcd.h"
#include "pico/stdlib.h"

typedef struct {
    ColorRGBByte c;
    uint16_t x0;
    uint16_t y0;
    uint16_t x1;
    uint16_t y1;
} LineBuf;

   
static void _drawLineLow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorRGBByte c) {
    int16_t dx = x1 - x0;
    int16_t dy = y1 - y0;
    int8_t yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int32_t D = (2 * dy) - dx;
    uint16_t y = y0;

    for(uint16_t x = x0; x <= x1; x++ ) {
        lcd_write_pixel_3b(x, y, c);
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

static void _drawLineHigh(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorRGBByte c) {
    int16_t dx = x1 - x0;
    int16_t dy = y1 - y0;
    int8_t xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int32_t D = (2 * dx) - dy;
    uint16_t x = x0;

    for(uint16_t y = y0; y <= y1; y++ ) {
        lcd_write_pixel_3b(x, y, c);
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2*dx;
        }
    }
}

// By switching the x and y axis an implementation for positive or negative steep gradients can be written as
// A complete solution would need to detect whether x1 > x0 or y1 > y0 and reverse the input coordinates before drawing, thus

void lcd_prim_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorRGBByte c) {
    if (abs(y1 - y0) < abs(x1 - x0)) {
        if (x0 > x1) {
            _drawLineLow(x1, y1, x0, y0, c);
        } else {
            _drawLineLow(x0, y0, x1, y1, c);
        }
    } else {
        if (y0 > y1) {
            _drawLineHigh(x1, y1, x0, y0, c);
        } else {
            _drawLineHigh(x0, y0, x1, y1, c);
        }
    }
}

void lcd_prim_fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorRGBByte c) {
    uint16_t xs, ys;
    if (x1 > x0) {
        xs = x1 - x0;
    } else {
        xs = x0 - x1;
    }
    if (y1 > y0) {
        ys = y1 - y0;
    } else {
        ys = y0 - y1;
    }

    uint32_t r = (xs + 1) * (ys + 1); 
    lcd_write_addr(x0, y0, x1, y1);
    //printf("%d %d %d %d\n", x1, x0, y1, y0);
    while (r-- > 0) {
        lcd_write_color_3b(c);
    }

}





