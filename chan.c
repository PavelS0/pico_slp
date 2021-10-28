#include <stdio.h>
#include "pico/stdlib.h"
#include "chan.h"

#define MAX_EFF 255




chan_handle* chan_init(uint16_t size, uint16_t chan_count, uint16_t current_count) {
    chan_handle* c = malloc(sizeof(chan_handle));
    c->size = size;
    c->chan_count = chan_count;
    c->current_count = current_count;
    c->chans = malloc(chan_count * sizeof(chan));
    if (current_count > 0) {
        c->currents = malloc(current_count * sizeof(chan));
    }
    c->mov_avg_idx = 0;
    return c;
}


uint16_t calc_mov_avg(uint16_t* data, uint16_t size) {
    uint16_t i = size;
    uint32_t sum = 0;
    while(i > 0) {
        i--;
        sum += data[i];
    }
        
    return sum / size;
}

void calc_chan(chan* c, uint16_t avg, uint16_t max, uint8_t mov_avg_index) {
    c->avg = avg;
    c->max = max;
    if (c->max_continuous > 4000) {
        c->max_continuous = 0;
    }
    if (c->max_continuous < max) {
        c->max_continuous = max;
        c->val = 1.0;
    } else {
        uint16_t mov_avg = calc_mov_avg(c->mov_avg, MOV_AVG_SIZE);
        if(c->max > mov_avg) {
            c->val = (float) (c->max - mov_avg) / (c->max_continuous - mov_avg);
        } else {
            c->val = 0.0;
        }
        c->mavg = mov_avg;
    }
    
    c->mov_avg[mov_avg_index] = c->max; // скользящее среднее
    c->max_continuous--;
}



void process_chans(chan_handle* c, uint16_t* buf) {
    uint16_t size = c->size;
    chan* chans = c->chans;
    uint16_t ccnt = c->chan_count;
    uint16_t cstep = c->size / ccnt;
    uint8_t end;
    uint16_t j = 0;
    uint16_t ci = 0;  
    uint32_t sum = 0;
    uint16_t max = 0;

    for (uint16_t i = 0; i < size; i++, j++) {
        if (j == cstep) {
            uint16_t avg = sum / cstep;
            if (ci == 2) {
                end = 1;
            }
            calc_chan(&chans[ci], avg, max, c->mov_avg_idx);
            
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

    c->mov_avg_idx++;
    if(c->mov_avg_idx == MOV_AVG_SIZE) {
        c->mov_avg_idx = 0;
    }

}

void chan_copy_data(chan_handle* h, chan_data* d) {
    chan* chans = h->chans;
    for (int i = 0; i < h->chan_count; i++) {
        chan* c = &chans[i];
        d[i].max_continuous = c->max_continuous;
        d[i].max = c->max;
        d[i].avg = c->mavg;
        d[i].val = c->val;
    }
}

/* void sort_chans(chan* chans, uint16_t cnt, uint8_t* sorted_idx)
{
    uint16_t j = 0;
    for(uint16_t i = 0; i < cnt; i++){ {
        j = i;
        for(uint16_t k = i; k < cnt; k++){
            if(chans[j].efficiency > chans[k].efficiency){
                j = k;
            }
        }
        sorted_idx[i] = j;
        chans[j] = chans[i];
    }
} */

void switch_chan_if_can(chan_handle* c, chan* n, uint8_t index) {
    current *cur = &c->currents[index];
    if (cur->chan != n) {
        //TODO: swicth_channels
    }
}


void select_chans(chan_handle* c) {
    chan *chans = c->chans;
    uint8_t range = c->chan_count / c->current_count;
    //sort_chans(c->chans, ccnt, c->sorted_idx);
    uint8_t j = 0;
    uint8_t max_idx = 0;
    uint8_t cur_idx = 0;
    for (uint16_t i = 0; i < c->chan_count; i++) {
        if (j == range) {
            switch_chan_if_can(c, &chans[max_idx], cur_idx );
            j = 0;
            max_idx = 0;
            cur_idx++;
        }
        if (chans[max_idx].efficiency < chans[i].efficiency) {
            max_idx = i;
        }

        // выбрать канала, при чём, так, чтобы они были расположены друг за другом на расстоянии 2
    }
}

void chan_process(chan_handle* h, uint16_t* buf) {
    process_chans(h, buf);
    if (h->current_count > 0) {
        select_chans(h);
    }
}



