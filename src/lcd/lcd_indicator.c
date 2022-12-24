
#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/interp.h"
#include "hardware/dma.h"
#include "lcd_indicator.h"
#include "lcd_primitives.h"

Indicator lcd_indicator_init(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, ColorRGBByte bg) {
    Indicator g;
    g.x0 = x0;
    g.y0 = y0;
    g.h = h;
    g.w = w;
    g.bg = bg;
    g.val = 0.0;
    lcd_prim_fill_rect(x0, y0, x0 + w, y0 + h, bg);
    lcd_flush();
    return g;
}

void lcd_indicator_draw(Indicator* ind, float value, ColorRGBByte c) {
    if (value > 1.0) {
        value = 1.0;
    } else if (value < 0.0) {
        value = 0.0;
    }
    uint16_t h = value * ind->h;
    lcd_prim_fill_rect(ind->x0, ind->y0, ind->x0 + ind->w, ind->y0 + h, c);
    lcd_prim_fill_rect(ind->x0, ind->y0 + h, ind->x0 + ind->w, ind->y0 + ind->h, ind->bg);

    /* uint16_t h1, h2;
    ColorRGBByte fc;
    if (ind->val > value) {
        h1 = value * ind->h;
        h2 = ind->val * ind->h;
        fc = ind->bg;
    } else {
        h1 = ind->val * ind->h;
        h2 = value * ind->h;
        fc = c;
    }
    ind->val = value;
    if (h1 != h2)
        lcd_prim_fill_rect(ind->x0, ind->y0 + h1, ind->x0 + ind->w, ind->y0 + h2, fc); */
}







