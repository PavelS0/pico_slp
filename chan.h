#ifndef CHAN_H
#define CHAN_H

#define MOV_AVG_SIZE 10

typedef struct 
{
    uint16_t max_continuous;
    uint16_t max;
    uint16_t avg;
    float val;
} chan_data;

typedef struct 
{
    uint16_t max_continuous;
    uint16_t max;
    uint16_t avg;
    uint16_t mavg;
    uint16_t mov_avg[MOV_AVG_SIZE];
    float val;
    uint8_t efficiency;
} chan;

typedef struct 
{
    uint16_t value;
    uint16_t index;
    chan* chan;
} current;

typedef struct handle
{
    uint16_t size;
    uint16_t chan_count;
    uint16_t current_count;
    chan* chans;
    chan** cur_chans;
    current* currents;
    uint8_t mov_avg_idx;
} chan_handle;
chan_handle* chan_init(uint16_t size, uint16_t chan_count, uint16_t current_count);
void chan_process(chan_handle* h, uint16_t* buf);
void chan_copy_data(chan_handle* h, chan_data* d);

#endif