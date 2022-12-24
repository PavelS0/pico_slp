#ifndef CHAN_H
#define CHAN_H

#define MOV_AVG_SIZE 10

#define CHAN_DIR_ATTACK 1
#define CHAN_DIR_RELEASE 0

typedef struct 
{
    uint16_t max_continuous;
    uint16_t max;
    uint16_t avg;
    float val;
} chan_data;

typedef struct 
{
    float max_continuous;
    float max;
    float avg;
    uint16_t deadzone;
    float target;
    float val;
    float abs_val;
    uint8_t dir;
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
    uint16_t attack;
    uint16_t release;
    chan* chans;
    uint32_t last_call_time;
} chan_handle;
chan_handle* chan_init(uint16_t size, uint16_t chan_count, uint16_t attack, uint16_t release);
void chan_process(chan_handle* h, float* buf);
void chan_copy_data(chan_handle* h, chan_data* d);

#endif