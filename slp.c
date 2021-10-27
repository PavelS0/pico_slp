#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

#define SAMPLING_SIZE 512
#define DMA_SAMPLES_CHAN 4
#define DMA_SPECTRE_CHAN 5

uint16_t samples_area[SAMPLING_SIZE * 2];
uint16_t samples[SAMPLING_SIZE];

void core1_main();

int main() {
    stdio_init_all();

    memset(samples, 0, SAMPLING_SIZE * sizeof(uint16_t));
    
    multicore_launch_core1(core1_main);
    sleep_ms(200);
    while (1)
    {
       
    } 
}

void core1_main() {
    dma_channel_config c = dma_channel_get_default_config(DMA_SAMPLES_CHAN);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16); // uint16_t
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(
        DMA_SAMPLES_CHAN,          // Channel to be configured
        &c,            // The configuration we just created
        samples,           // The initial write address
        samples_area,           // The initial read address
        0, // Number of transfers;
        false           // Start immediately.
    );

    dma_channel_claim(DMA_SAMPLES_CHAN);

    while(1) {
        for (int n = 0; n < SAMPLING_SIZE - 10; n++)
        {
            samples_area[n] = 10;
        }
        // memcpy(samples, s, SAMPLING_SIZE * sizeof(uint16_t));
        
        if (!dma_channel_is_busy(DMA_SAMPLES_CHAN)) {
            dma_channel_transfer_from_buffer_now(DMA_SAMPLES_CHAN, samples_area, SAMPLING_SIZE);
        }
    }
}