#ifndef WS2812_MUSIC_H
#define WS2812_MUSIC_H

#include <stdio.h>
#include "chan.h"

void ws2812_music_init(chan_handle* h, uint led_count);
void ws2812_music(void);

#endif