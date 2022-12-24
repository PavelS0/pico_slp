#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "out.h"


#define PERIOD 255 
static uint8_t _chan_count;
static uint8_t _gpio_base;

void out_init(uint8_t chan_count, uint8_t gpio_base) {
    /* gpio_init(gpio_base);
    gpio_set_dir(gpio_base, GPIO_OUT);
    gpio_put(gpio_base, 1); */
   /*  
    gpio_set_function(gpio_base, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    uint slice_num = pwm_gpio_to_slice_num(gpio_base);
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_config_set_wrap(&config, PERIOD);
    pwm_init(slice_num, &config, true);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 255); */

    assert(gpio_base % 2 == 0);

    uint8_t bc = gpio_base + chan_count;
    for (uint8_t i = gpio_base; i < bc; i++) {
        gpio_set_function(i, GPIO_FUNC_PWM);
    }

    _chan_count = chan_count;
    _gpio_base = gpio_base;

    bool multiple = chan_count % 2 == 0;

    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_config_set_wrap(&config, PERIOD);

    for (uint16_t i = gpio_base; i < bc; i += 2) {
        uint slice_num = pwm_gpio_to_slice_num(i);
        pwm_init(slice_num, &config, true);

        pwm_set_chan_level(slice_num, PWM_CHAN_A, 255);
        if (multiple || i != bc - 1) {
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 255);
        } 

        // Set the PWM running
        pwm_set_enabled(slice_num, true);
    } 
}

void out_process(chan_handle* h) {
    chan* chans = h->chans;
    
    bool multiple = _chan_count % 2 == 0;

    uint8_t bc = _gpio_base + _chan_count;
    uint8_t j = 0;
    for (uint8_t i = _gpio_base; i < bc; i += 2, j += 2) {
        uint slice_num = pwm_gpio_to_slice_num(i);
        pwm_set_chan_level(slice_num, PWM_CHAN_A, chans[j].val * 255);
        if (multiple || i != bc - 1) {
            pwm_set_chan_level(slice_num, PWM_CHAN_B, chans[j+1].val * 255);
        }
    }
}