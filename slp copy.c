
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

int main() {
    stdio_init_all();

    memset(samples, 0, SAMPLING_SIZE * sizeof(uint16_t));
    memset(spectre, 0, SAMPLING_SIZE * sizeof(uint16_t));

    init_mem_dma(DMA_SAMPLES_CHAN, samples, samples_area);
    init_mem_dma(DMA_SPECTRE_CHAN, spectre, samples_area);

    lcd_init(pio0, 0);

    ColorRGBByte color;
    lcd_set_color_rgb(&color, 1, 0, 0);

    ColorRGBByte bg;
    lcd_set_color_rgb(&color, 0, 0, 1);

    lcd_prim_fill_rect(0, 0, 480, 320, bg);

    Graph g = lcd_graph_init(DISPLAY_SAMPLES, 5, 5, 470, 150, bg, 1); */
    //Graph g1 = lcd_graph_init(DISPLAY_SAMPLES, 5, 160, 470, 150, bg, 2);

    
    multicore_launch_core1(core1_main);

    while (1)
    {
        lcd_graph_draw_unsigned(&g,  samples, DISPLAY_SAMPLES, color);    

        //dma_channel_wait_for_finish_blocking(DMA_SPECTRE_CHAN);
        //lcd_graph_draw_unsigned(&g1, spectre, DISPLAY_SAMPLES, color); 
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
    
    //int16_t buffer[BUF_S + 2];
   /*  fft_cpx out[SAMPLING_SIZE + 2];
    float out_mag[SAMPLING_SIZE + 2];
    uint16_t val[SAMPLING_SIZE]; */


    fft_cfg fft_c = fft_init(SAMPLING_SIZE);
    
    dma_channel_config c = dma_channel_get_default_config(DMA_SAMPLES_CHAN);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(
        DMA_SAMPLES_CHAN,          // Channel to be configured
        &c,            // The configuration we just created
        samples,           // The initial write address
        samples_area,           // The initial read address
        0, // Number of transfers; in this case each is 1 byte.
        false           // Start immediately.
    );

    adc_ini(samples_area, SAMPLING_SIZE * 2);
    

    uint16_t frequency = 20;
    int8_t fstep = 1;

    while(1) {
            //uint16_t* s = adc_switch_buf();

            for (int n = 0; n < SAMPLING_SIZE; n++)
            {
                samples_area[n] = frequency;
            }
           // memcpy(samples, s, SAMPLING_SIZE * sizeof(uint16_t));
            if (dma_channel_is_busy(DMA_SAMPLES_CHAN)) {
                //puts("b");
            } else {
                dma_channel_transfer_from_buffer_now(DMA_SAMPLES_CHAN, samples_area, 128);
                //puts("t");
            }
            
            //dma_channel_wait_for_finish_blocking(DMA_SAMPLES_CHAN);

            int a = 0;
            a++;
            //puts('dma send');
            //
            //
            /* fft(fft_c, (int16_t*) samples, out);
            fft_mag(out, out_mag, SAMPLING_SIZE);

            for (int n = 0; n < SAMPLING_SIZE; n++)
            {
                val[n] = (uint32_t)out_mag[n] * 5;
            }

        
            dma_channel_transfer_from_buffer_now(DMA_SPECTRE_CHAN, val, SAMPLING_SIZE / 2); */
        

        if (frequency > 100) {
            fstep = -1;
        }
        
        if (frequency < 10) {
            fstep = 1;
        }

        frequency = frequency + fstep;

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