#ifndef OUT_H
#define OUT_H
#include "chan.h"

void out_init(uint8_t chan_count, uint8_t gpio_base);
void out_process(chan_handle* h);

#endif