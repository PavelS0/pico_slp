
#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/interp.h"
#include "hardware/dma.h"
#include "lcd_graph.h"
#include "lcd_primitives.h"

Graph lcd_graph_init(uint16_t size, uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, ColorRGBByte bg, uint8_t dma_chan) {
    Graph g;
    g.x0 = x0;
    g.y0 = y0;
    g.h = h;
    g.w = w;
    g.bg = bg;
    
    lcd_prim_fill_rect(x0, y0, x0 + w, y0 + h, bg);
    lcd_flush();

    return g;
}

void lcd_graph_draw_unsigned(Graph* g, uint16_t* buf, uint16_t size, ColorRGBByte c) {
    bool clamp = false;
    float step;
    int16_t l_add;
    int16_t r_add;
    int16_t err = 0;
    uint8_t add;
    if (g->w > size) {
        step = (float)g->w /  size;
    } else {
        step = (float)size / g->w;
        float fp, ip, rfp;
        fp = modf(step, &ip);
        rfp = 1.0 - fp;
        add = (uint8_t)step;
        l_add = (int16_t)(fp * 1000);
        r_add = (int16_t)(rfp * 1000);
        clamp = true;

        //printf("%f %f %f %d %d %d", fp, ip, rfp, g->w, l_add, r_add);
    }

    //flip flop alg
    if(clamp) {
        uint16_t j = 0;
        uint16_t pj = 0;
        for (int i = 0; i < g->w - 1; i++)
        {
            lcd_prim_fill_rect(i, g->y0, i+1, g->y0 + g->h, g->bg);
            lcd_write_pixel_3b(i, buf[j] + g->y0, c );
            //lcd_prim_draw_line(i,, , buf[j] + g->y0, c);
            pj = j;
            if (err < 0) {
                j += add + 1;
                err += r_add;
            } else {
                j += add;
                err -= l_add;
            }
            //printf("e %i \n", err);
            if (j >= size) {
                //printf("error %i %i \n", i, j);
            }
        }
    } else {
        for (int i = 0; i < size - 1; i++)
        {
            uint16_t f = (uint16_t) (i  * step + g->x0);
            uint16_t t = (uint16_t) ((i + 1) * step + g->x0);
            lcd_prim_fill_rect(f, g->y0, t, g->y0 + g->h, g->bg);
            lcd_prim_draw_line(f, buf[i] + g->y0, t, buf[i+1] + g->y0, c);
        }
    }
}

void _drawPart(uint16_t ) {

}

void lcd_graph_draw_signed(Graph* g, int16_t* buf, uint16_t size, ColorRGBByte c) {
    float step = (float)g->w / (size);
    uint hw = g->h / 2 + g->y0;
    for (int i = 0; i < size - 1; i++)
    {
        uint16_t f = (uint16_t) (i  * step + g->x0);
        uint16_t t = (uint16_t) ((i + 1) * step + g->x0);
        lcd_prim_fill_rect(f, g->y0, t, g->y0 + g->h, g->bg);
        lcd_prim_draw_line(f, buf[i] + hw, t, buf[i+1] + hw, c);
    }
}








