#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/interp.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "pico/multicore.h"
#include "pico/mutex.h"

#include "adc.h"
#include "chan.h"
#include "out.h"
#include "fft.h"
#include "usb.h"
#include "tusb.h"
#include "packet_mgr.h"
#include "ws2812.h"
#include "hardware/uart.h"
#include "net.h"
#include "control.h"

#define SAMPLING_SIZE 1024
#define DISPLAY_SAMPLES SAMPLING_SIZE / 4
#define SAMPLING_FREQ 16000
#define DMA_SAMPLES_CHAN 4
#define DMA_SPECTRE_CHAN 5
#define CHAN_CNT 10


uint32_t clc;
uint16_t samples_area[SAMPLING_SIZE * 2];
uint16_t samples[SAMPLING_SIZE];
float spectre[SAMPLING_SIZE];

chan_data ind_chan[CHAN_CNT];

void init_mem_dma(int dma_chan, volatile void* dst, volatile void* src) ;
void remove_dc(uint16_t* in,  int16_t* out, uint16_t n);
void core1_main();
void interp_cfg();

static PACKET_MGR paket_mgr;
static float vvv;
static uint32_t ddd;
static chan_handle* chan_hnd;
static char c;
static uint baud;
void on_uart_rx() {
    while (uart_is_readable(uart1)) {
        c = uart_getc(uart1);
        // Can we send it back?
        c = 'a';
    }
}


int main() {
    stdio_init_all();
    // todo get free sm
   
    //2.4А per 1m
    // IR =(U п-U бэ)/R базы=(0,8-0,6)/1=0,2/1=0,2А=200 мА
    // Iб = Iк / H21э = 0.05/50 = 0,001
    // Rбазы=(Uп - Uбэизм) / Iб=(5-0.7)/0,001=4300 Ом
    //
    // Iб = Iк / H21э = 6/100 = 0,06
    // Rбазы=(Uп - Uбэизм) / Iб=(5-0.7)/0.06=71 Ом
    //
    //

    memset(samples, 0, SAMPLING_SIZE * sizeof(uint16_t));
    memset(spectre, 0, SAMPLING_SIZE * sizeof(float));

    multicore_launch_core1(core1_main);

    //usb_init();
    
    while (chan_hnd == NULL);
    //ws2812_init(2, 176, chan_hnd);
    

    
    if (packet_mgr_init(&paket_mgr)) {
        packet_mgr_reg(&paket_mgr, 0, control_pack_test_packet, control_unpack_test_packet, control_recv_test_packet);
        while (1) {
            packet_mgr_pool(&paket_mgr);
        }
    }
}



void fill_test_samples(int16_t* buffer, uint16_t frequency) {
    uint16_t sampleRate = 8000;
    uint16_t amplitude = 50;
    uint16_t shift = 60;

    for (int n = 0; n < SAMPLING_SIZE; n++)
    {
        buffer[n] = (int32_t)(amplitude * sin((2 * M_PI * n * frequency) / sampleRate));
    }
}

void core1_main() {
    float fft_in[SAMPLING_SIZE + 2];
    float out_mag[SAMPLING_SIZE + 2];
    fft_cfg fft_c = fft_init(SAMPLING_SIZE);

    interp_cfg();
    //out_init(CHAN_CNT, 18);
    chan_hnd = chan_init(SAMPLING_SIZE / 2, CHAN_CNT, 30, 30);
    
    //p.chans_len = CHAN_CNT;
    //p.chans = chan_hnd->chans;

    adc_ini(samples_area, SAMPLING_SIZE * 2);
    while(1) {
        uint16_t* s = adc_switch_buf();
        for (int n = 0; n < SAMPLING_SIZE; n++)
        {
            interp1->accum[0] = s[n];
            samples[n] = interp1->peek[0];
            fft_in[n] = (float)s[n];
        }
    
        fft(fft_c, fft_in, spectre);
        out_mag[0] = 0;

        chan_process(chan_hnd, spectre);
    }
}



void init_mem_dma(int dma_chan, volatile void* dst, volatile void* src) {
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(
        dma_chan,          // Channel to be configured
        &c,            // The configuration we just created
        dst,           // The initial write address
        src,           // The initial read address
        0, // Number of transfers; in this case each is 1 byte.
        false           // Start immediately.
    );

    

    //dma_channel_set_irq0_enabled(dma_chan, true);
}

void interp_cfg() {
    interp_config cfg = interp_default_config();
    interp_config_set_clamp(&cfg, true);
    interp_config_set_shift(&cfg, 4);
    // set mask according to new position of sign bit..
    interp_config_set_mask(&cfg, 0, 27);
    // ...so that the shifted value is correctly sign extended
    interp_config_set_signed(&cfg, true);
    interp_set_config(interp1, 0, &cfg);

    interp1->base[0] = 0;
    interp1->base[1] = 255;   
}


