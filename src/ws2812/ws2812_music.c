#include <stdio.h>
#include <stdlib.h>

#include "hardware/pio.h"

#include "ws2812.pio.h"
#include "ws2812_common.h"
#include "ws2812_music.h"

static chan_handle* hnd; 
static uint leds;
static uint part_len;
static float tl;
static float tm;
static float th;
static uint8_t pattern[128];

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}


static void fill_pattern() {
    memset(pattern, 0, 128 * sizeof(uint8_t));
    uint8_t j = 0;
    for (uint8_t i = 32; i < 64; i++, j++) {
        pattern[i] = (j / 32.0) * 255;
    }
    pattern[64] = 1;
    j = 1;
    for (uint8_t i = 65; i < 96; i++, j++) {
        pattern[i] = (1 - (j / 32.0)) * 255;;
    }
}

void ws2812_music_init(chan_handle* h, uint led_count) {
    while(h == NULL);
    hnd = h;
    leds = led_count;
    part_len = led_count / h->chan_count;
    fill_pattern();  
    pattern[0] = 0;
}

inline float zero(float v) {
    if (v < 0) {
        v = 0;
    }
    return v;
}

inline float intr(float l, float r, float v) {
    if (v > l && v < r) {
        return (float) (r - l) / (v - l);
    }
    return 0;
}

inline float in_range(float l, float r, float v) {
    if (v > l && v < r) {
        return 1;
    }
    return 0;
}

void ws2812_music(void) {
    uint p = 0;
    uint8_t cc = hnd->chan_count;
    for (uint16_t i = 0; i < cc; i++) {
        uint16_t l = p + part_len;
        chan* ch = &hnd->chans[i];
        uint16_t v = ch->max;
        float percent =  ch->val;//(float)v / 50;
        uint16_t leden = p + part_len * percent;
        uint16_t leden1 = p + part_len * (1 - percent);
        uint8_t dir = i % 2;

        float per = (float)i / cc;
        float red = in_range(0.0, 0.5, per);
        float green = in_range( 0.2, 0.8, per);
        float blue = in_range( 0.5, 1.0, per);
        for (uint16_t k = p; k < l; k++) {            
            if (dir == 0) {
                if (k < leden) {
                    put_pixel(urgb_u32(255 * red * percent, 255 * green * percent, 255 * blue * percent));
                } else {
                    put_pixel(0);
                }
            } else {
                if (k > leden1) {
                    put_pixel(urgb_u32(255 * red * percent, 255 * green * percent, 255 * blue * percent));
                } else {
                    put_pixel(0);
                }
            }
           
        }
        p += part_len;
    }
    sleep_ms(10);
}



void ws2812_music_wave(void) {
    float r = 6.28;
    float d = 48 / 3.14;
    float d1 = 128 / r;
    chan* chl = &hnd->chans[2];
    chan* chm = &hnd->chans[5];
    chan* chh = &hnd->chans[8];
    tl = tl +  0.2;//2 * chl->val;
    tm = tm + 2 * chm->val;
    th = th + 2 * chh->val;
    for (uint i = 0; i < leds; ++i) {
        uint xl = (uint)(i + tl) % 128;
        put_pixel(urgb_u32((pattern[xl] * chl->val), 0/* (128 + 127 * sm) * chm->val */, 0 /* (128 + 127 * sh) * chh->val */));
    }
        
    sleep_ms(10);
}
/// в центре синий по краям красный
void ws2812_music_color1(void) {
    uint p = 0;
    for (uint16_t i = 0; i < hnd->chan_count; i++) {
        uint16_t l = p + part_len;
        chan* ch = &hnd->chans[i];
        uint16_t v = ch->max;
        float percent =  (float)v / 50;
        uint16_t leden = p + part_len * percent;
        uint16_t leden1 = p + part_len * (1 - percent);
        uint8_t dir = i % 2;


        for (uint16_t k = p; k < l; k++) {
            if (dir == 0) {
               
                if (k < leden) {
                    float per = (float)(k - p) / part_len;
                    float blue = intr( 0.0, 0.4, per);
                    float green = intr( 0.2, 0.65, per);
                    float red = intr(0.6, 1.0, per);
                    put_pixel(urgb_u32(255 * red, 255 * green, 255 * blue));
                } else {
                    put_pixel(0);
                }
            } else {
                if (k > leden1) {
                    float per = 1 - (float)(k - p) / part_len;
                    float blue = intr( 0.0, 0.4, per);
                    float green = intr( 0.2, 0.65, per);
                    float red = intr(0.6, 1.0, per);
                    put_pixel(urgb_u32(255 * red, 255 * green, 255 * blue));
                } else {
                    put_pixel(0);
                }
            }
           
        }
        p += part_len;
    }
    sleep_ms(10);
}