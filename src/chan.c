#include <stdio.h>
#include "pico/stdlib.h"
#include "chan.h"
#include "pico/time.h"
#define MAX_EFF 255

chan_handle* chan_init(uint16_t size, uint16_t chan_count, uint16_t attack, uint16_t release) {
    chan_handle* c = malloc(sizeof(chan_handle));
    c->size = size;
    c->chan_count = chan_count;
    c->attack = attack;
    c->release = release;
    c->last_call_time = 0;
    c->chans = malloc(chan_count * sizeof(chan));
    memset(c->chans, 0, chan_count * sizeof(chan));
    return c;
}

void calc_chan(chan_handle* h, chan* c, float avg, float max, uint32_t delta) {
    if (max > c->deadzone) {
        c->max = max - c->deadzone;
    } else {
        c->max = 0;
    }
    if (avg > c->deadzone) {
        c->avg = avg - c->deadzone;
    } else {
        c->avg = 0;
    }
    if (c->max_continuous < c->max) {
        c->max_continuous = c->max * 1.15;
    }

    if (c->target < c->max) {
        //c->dir = CHAN_DIR_ATTACK;
        c->target = c->max;
    }
    
    if (c->dir == CHAN_DIR_ATTACK) {
        float attack = (delta / 400.0) * (c->max_continuous);
        c->abs_val = c->abs_val + attack;
        if (c->target < c->abs_val) { 
            c->dir = CHAN_DIR_RELEASE;
        }
    } else {
        float release = (delta / 400.0) * (c->max_continuous);
        c->abs_val = c->abs_val - release;
        c->target = c->target - release;
        if (c->target > c->abs_val) {
            c->dir = CHAN_DIR_ATTACK;
        }
    }

    float v = c->abs_val / c->max_continuous - 0.15;
    if (v > 1) {
        v = 1.0;
    } else if (v < 0) {
        v = 0.0;
    }
    c->val = v;
}

void process_chans(chan_handle* c, float* buf) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    uint16_t size = c->size;
    chan* chans = c->chans;
    uint16_t ccnt = c->chan_count;
    uint16_t cstep = c->size / ccnt - 1;
    uint8_t end;
    uint16_t j = 0;
    uint16_t ci = 0;  
    float sum = 0;
    float max = 0;

    for (uint16_t i = 0; i < size; i++, j++) {
        if (j == cstep) {
            float avg = sum / cstep;

            calc_chan(c, &chans[ci], avg, max, now - c->last_call_time);
            
            sum = 0;
            max = 0;
            ci++;
            j = 0;
        }
        
        uint16_t v = buf[i];
        sum += v;
        if (max < v) {
            max = v;
        }
    }

    c->last_call_time = now;
}

void chan_copy_data(chan_handle* h, chan_data* d) {
    chan* chans = h->chans;
    for (int i = 0; i < h->chan_count; i++) {
        chan* c = &chans[i];
        d[i].max_continuous = c->max_continuous;
        d[i].max = c->max;
        d[i].avg = c->avg;
        d[i].val = c->val;
    }
}

void chan_process(chan_handle* h, float* buf) {
    process_chans(h, buf);
}



