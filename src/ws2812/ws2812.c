/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "math.h"
#include "ws2812_music.h"

#define IS_RGBW false
#define NUM_PIXELS 176

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 2
#endif

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_rainbow(uint len, uint t) {
    uint16_t sl = 7;
    uint16_t d = 92 / sl; 
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 92;
        uint p = (((int)x - 92) / d) * -1;

        switch (p)
        {
        case 0:
            put_pixel(urgb_u32(255, 0, 0));
            break;
        case 1:
            put_pixel(urgb_u32(255, 127, 0));
            break;
        case 2:
            put_pixel(urgb_u32(255, 255, 0));
            break;
        case 3:
            put_pixel(urgb_u32(0, 255, 0));
            break;
        case 4:
            put_pixel(urgb_u32(0, 0, 255));
            break;
        case 5:
            put_pixel(urgb_u32(75, 0, 130));
            break;
        case 6:
            put_pixel(urgb_u32(238, 130, 238));
            break;
        default:
            put_pixel(0);
            break;
        }
    }
}

void pattern_wave(uint len, uint t) {
    float r = 6.28;
    float d = 420 / r;
    float d1 = 64 / r;
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 420;
        uint x1 = (i + 18 + (t >> 1)) % 64;
        uint x2 = (i + 48 + (t >> 1)) % 64;
        uint x3 = (i + 80 + (t >> 1)) % 64;
        
        float p = ((int)x - 420) / d;
        float p1 = ((int)x1 - 64) / d1;
        float p2 = ((int)x2 - 64) / d1;
        float p3 = ((int)x3 - 64) / d1;

        float s1 = sin(p1);
        float s2 = sin(p2);
        float s3 = sin(p3);

        float m = (float)(sin(p) + 1) / 2;

        put_pixel(urgb_u32((128 + 127 * s1) * m, (128 + 127 * s2) * m, (128 + 127 * s3) * m));
    }
}

void pattern_wave_night(uint len, uint t) {
    float r = 6.28;
    float d = 420 / r;
    float d1 = 64 / r;
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 420;
        uint x1 = (i + 18 + (t >> 1)) % 64;
        
        float p = ((int)x - 420) / d;
        float p1 = ((int)x1 - 64) / d1;
       
        float s1 = sin(p1);
       
        float m = (float)(sin(p) + 1) / 4;

        put_pixel(urgb_u32((70 + 50 * s1) * m, 0, (128 + 88 * s1) * m));
    }
}

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    int max = 100; // let's not draw too much current!
    t %= max;
    for (int i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_rainbow,  "Snakes!"},
        {pattern_wave,  "Random data"},
        {pattern_wave_night,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};

void ws2812_init(uint pin, uint led_count, void *data) {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, pin, 800000, false);

    ws2812_music_init(data, led_count);
}

int ws2812_animate() {
    //ws2812_music();
    int t = 0;
    while (1) {
        int pat = 2; // rand() % count_of(pattern_table);
        int dir = (rand() >> 30) & 1 ? 1 : -1;
        puts(pattern_table[pat].name);
        puts(dir == 1 ? "(forward)" : "(backward)");
        for (int i = 0; i < 1000; ++i) {
            pattern_table[pat].pat(NUM_PIXELS, t);
            sleep_ms(20);
            t += dir;
        }
    } 
}

