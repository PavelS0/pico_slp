#ifndef LCD_H
#define LCD_H

#include <stdio.h>
#include "hardware/pio.h"


#define LCD_SCREEN_WIDTH  320
#define LCD_SCREEN_HEIGHT 480

typedef union
{
    struct __attribute__((packed))
    {
       uint8_t color:6;
    };
    uint8_t word;
} ColorByte;


typedef union
{
    struct __attribute__((packed))
    {
        bool r0:1;
        bool g0:1;
        bool b0:1;
        bool r:1;
        bool g:1;
        bool b:1;
    };
    uint8_t color;
} ColorRGBByte;

void lcd_set_color_rgb(ColorRGBByte* c, bool r, bool g, bool b);

void lcd_flush();
void lcd_write(bool dc, const uint8_t data);
void lcd_write_cmd(const uint8_t *cmd, size_t count);
void lcd_write_color_3b(ColorRGBByte c);
void lcd_write_pixel_3b(uint16_t x, uint16_t y, ColorRGBByte c);
void lcd_init(PIO pio, uint sm);
void lcd_write_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_write_ramwr();
#endif