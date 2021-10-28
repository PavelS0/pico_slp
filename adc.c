#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "adc.h"

#define CAPTURE_CHANNEL 0

static uint16_t len = 0;
static uint16_t index = 0;
static uint16_t* buf_src;
static uint16_t* buf;

static void irq ();

void adc_ini(uint16_t* b, uint16_t size) {
       // Init GPIO for analogue use: hi-Z, no pulls, disable digital input buffer.
    assert(buf == 0);
    len = size / 2;
    buf_src = b;
    buf = buf_src;
    index = 0;

    adc_init();

    adc_gpio_init(26 + CAPTURE_CHANNEL);
    adc_select_input(CAPTURE_CHANNEL);
    adc_fifo_setup(
        true,    // Write each completed conversion to the sample FIFO
        false,    // Enable DMA data request (DREQ)
        1,       // DREQ (and IRQ) asserted when at least 1 sample present
        false,   // We won't see the ERR bit because of 8 bit reads; disable.
        false     // Shift each sample to 8 bits when pushing to FIFO
    );

    adc_set_clkdiv(1980); //~ 24 khz sampling rate
    adc_irq_set_enabled(true);
    sleep_ms(1000);

    irq_set_enabled(ADC_IRQ_FIFO, true);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, irq);
    
    adc_run(true);
}


static void irq () {
    buf[index++] = adc_fifo_get();
    
    if (index >= len) {
        adc_irq_set_enabled(false);
        adc_run(false);
        index = len;
    }
}

uint16_t* adc_switch_buf() {
    while(index < len);
    uint16_t* ret;
   
    if (buf == buf_src) {
        buf = &buf_src[len];
        ret =  buf_src;
    } else {
        buf = buf_src;
        ret = &buf_src[len];
    }
    index = 0;

    adc_fifo_drain();
    adc_irq_set_enabled(true);
    adc_run(true);
    
    return ret;
}
