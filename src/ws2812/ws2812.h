#ifndef WS2812_H
#define WS2812_H

void ws2812_init(uint pin, uint led_count, void *data);
void ws2812_animate(void);
#endif