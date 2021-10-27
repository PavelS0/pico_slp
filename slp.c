#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/interp.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "pico/mutex.h"

#include "lcd.pio.h"
#include "lcd.h"
#include "adc.h"
#include "fft.h"
#include "lcd_graph.h"
#include "lcd_primitives.h"

#define SAMPLING_SIZE 512
#define DISPLAY_SAMPLES SAMPLING_SIZE / 4
#define SAMPLING_FREQ 16000
#define DMA_SAMPLES_CHAN 4
#define DMA_SPECTRE_CHAN 5

uint16_t samples_area[SAMPLING_SIZE * 2];
uint16_t samples[SAMPLING_SIZE];
uint16_t spectre[SAMPLING_SIZE];

void init_mem_dma(int dma_chan, volatile void* dst, volatile void* src) ;

void core1_main();
void interp_cfg();

int main() {
    stdio_init_all();

    memset(samples, 0, SAMPLING_SIZE * sizeof(uint16_t));
    memset(spectre, 0, SAMPLING_SIZE * sizeof(uint16_t));


    //init_mem_dma(DMA_SAMPLES_CHAN, samples, samples_area);
    //init_mem_dma(DMA_SPECTRE_CHAN, spectre, samples_area);

    lcd_init(pio0, 0);

    ColorRGBByte color;
    lcd_set_color_rgb(&color, 1, 0, 0);

    ColorRGBByte bg;
    lcd_set_color_rgb(&color, 0, 0, 1);

    lcd_prim_fill_rect(0, 0, 480, 320, bg);

    Graph g = lcd_graph_init(DISPLAY_SAMPLES, 5, 5, 470, 150, bg, 1);
    Graph g1 = lcd_graph_init(DISPLAY_SAMPLES, 5, 160, 470, 150, bg, 2);

    multicore_launch_core1(core1_main);

    while (1)
    {
        lcd_graph_draw_unsigned(&g,  samples, DISPLAY_SAMPLES, color);    
        lcd_graph_draw_unsigned(&g1, spectre, DISPLAY_SAMPLES, color);
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
    fft_cpx out[SAMPLING_SIZE + 2];
    float out_mag[SAMPLING_SIZE + 2];
    fft_cfg fft_c = fft_init(SAMPLING_SIZE);

    interp_cfg();
    
    adc_ini(samples_area, SAMPLING_SIZE * 2);

    while(1) {
        uint16_t* s = adc_switch_buf();

        for (int n = 0; n < SAMPLING_SIZE; n++)
        {
            interp1->accum[0] = s[n];
            samples[n] = interp1->peek[0];
            //printf("%d\t%d\n", s[n], interp1->peek[0]);
        }
        
    
        
        fft(fft_c, (int16_t*) s, out);
        fft_mag(out, out_mag, SAMPLING_SIZE);

        for (int n = 0; n < SAMPLING_SIZE; n++)
        {
            spectre[n] = (uint32_t)out_mag[n];
            //interp1->accum[0] = val[n];
            //printf("%d\t%d\n", spectre[n], 0);
        } 

        //memcpy(spectre, val, SAMPLING_SIZE * sizeof(uint16_t));
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


