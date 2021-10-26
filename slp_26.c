
#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/interp.h"

#include "lcd.pio.h"
#include "raspberry_256x256_rgb565.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480
#define IMAGE_SIZE 256
#define LOG_IMAGE_SIZE 8
#define BUF_SIZE 10000

#define PIN_DATA_BASE 7 // 7 (10) - MOSI, 8 (11) - DC, 9 (12) - CS
#define PIN_DC 11 // 5 нога лев
#define PIN_CS 9 // 4 нога лев
#define PIN_DIN 7// 10 нога лев  0 
#define PIN_CLK 6// 9 нога лев 1

#define PIN_RESET 4 // 6 нога лев
#define PIN_BL 5

#define SERIAL_CLK_DIV 1.f


#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64    \
    PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)

// Format: cmd length (including cmd byte), post delay in units of 5 ms, then cmd payload
// Note the delays have been shortened a little
    
static const uint8_t st7789_init_seq[] = {
        5, 0, 0xF7, 0xA9, 0x51, 0x2C, 0x82,
        3, 0, 0xC0, 0x11, 0x09,
        2, 0, 0xC1, 0x41,
        4, 0, 0xC5, 0x00, 0x0A, 0x80,
        3, 0, 0xB1, 0xB0, 0x11,
        2, 0, 0xB4, 0x02,
        3, 0, 0xB6, 0x02, 0x22,
        2, 0, 0xB7, 0xC6,
        3, 0, 0xBE, 0x00, 0x04,
        2, 0, 0xE9, 0x00,
        2, 0, 0x36, 0x08,
        2, 0, 0x3A, 0x66,
        16, 0, 0xE0, 0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F,
        16, 0, 0xE1, 0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F,
        1, 120, 0x11,
        1, 0, 0x29,
        0
};
/* static const uint8_t st7789_init_seq[] = {
        1, 20, 0x01,                         // Software reset
        1, 10, 0x11,                         // Exit sleep mode
        2, 2, 0x3a, 0x55,                   // Set colour mode to 16 bit
        2, 0, 0x36, 0x00,                   // Set MADCTL: row then column, refresh is bottom to top ????
        //5, 0, 0x2a, 0x00, 0x00, 0x00, 0xf0, // CASET: column addresses from 0 to 240 (f0)
        //5, 0, 0x2b, 0x00, 0x00, 0x00, 0xf0, // RASET: row addresses from 0 to 240 (f0)
        1, 2, 0x21,                         // Inversion on, then 10 ms delay (supposedly a hack?)
        1, 2, 0x13,                         // Normal display on, then 10 ms delay
        1, 2, 0x29,                         // Main screen turn on, then wait 500 ms
        0                                     // Terminate list
}; */

static uint16_t lcd_buf[BUF_SIZE];
static uint32_t current_size = 0;

static uint dma_chan = 0;

static inline bool lcd_flush() {
    if (current_size != 0) {
        if(!dma_channel_is_busy(dma_chan)){
        dma_channel_transfer_from_buffer_now(dma_chan, lcd_buf, current_size); // set transfer count and start NOW
        current_size = 0;
        return true;
        }
        return false;
    }
    return true;
}

static inline void lcd_flush_blocking() {
    if (current_size != 0) {
        dma_channel_transfer_from_buffer_now(dma_chan, lcd_buf, current_size);
        dma_channel_wait_for_finish_blocking(dma_chan);
        current_size = 0;
    }
}


static inline void lcd_write(bool dc, bool cs, const uint8_t data) {
    uint16_t val = 0;
    int i; int j = 0;
    for(i = 0; i < 15; i += 2 , j++) {
        if (j == 0) {
            val |= (!!dc << (i + 1)) | (((data >> j) & 1U) << i);
        } else {
            val |= (((data >> j) & 1U) << i);
        }
        
        // начиная c lsb каждый чётный бит - данные, не четный бит - dc/cx
    }
    lcd_buf[current_size++] = val;
    
    printf("BYTE :  %c-%c-%c-%c-%c-%c-%c-%c\n",
           PRINTF_BYTE_TO_BINARY_INT8(data));
    printf("HWORD: "
           PRINTF_BINARY_PATTERN_INT16 "\n",
           PRINTF_BYTE_TO_BINARY_INT16(val));
}

static inline void lcd_write_cmd(const uint8_t *cmd, size_t count) {
    if (count >= 2) {
        lcd_write(0, 0, *cmd++);
        size_t lastIdx = count - 2;
        for (size_t i = 0; i < count - 1; ++i) {
            if (i == lastIdx) {
                lcd_write(1, 1, *cmd++);
            } else {
                lcd_write(1, 0, *cmd++);
            }
        }   
    } else {
        lcd_write(0, 1, *cmd++);
    }
}

static inline void lcd_init(const uint8_t *init_seq) {
    const uint8_t *cmd = init_seq;
    while (*cmd) {
        lcd_write_cmd(cmd + 2, *cmd);
        lcd_flush_blocking();
        sleep_ms(*(cmd + 1) * 5);
        cmd += *cmd + 2;
    }
}

static inline void lcd_addr(uint x1, uint y1, uint x2, uint y2) {
    uint8_t cmd[] = {
        0x2a, x1>>8, x1, x2>>8, x2, // set x pos
        0x2b, y1>>8, y1, y2>>8, y2,  // set y pos
        0x2c // strart RAM write (RAMWR)
    };
    uint8_t* cmd_ptr = cmd;
    lcd_write_cmd(cmd_ptr, 5);
    cmd_ptr += 5;
    lcd_write_cmd(cmd_ptr, 5);
    cmd_ptr += 5;
    lcd_write_cmd(cmd_ptr, 1);
}

static inline void lcd_start_pixels() {
    uint8_t cmd = 0x2c; // RAMWR
    lcd_write_cmd(&cmd, 1);
    lcd_flush_blocking();
}

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &lcd_program);

    dma_channel_config dma_cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    dma_channel_configure(dma_chan, &dma_cfg,
        &pio->txf[sm], // Destination pointer
        lcd_buf,  // Source pointer         
        0, // Number of transfers
        false                // Start immediately
    );

    lcd_program_init(pio, sm, offset, SERIAL_CLK_DIV, PIN_CLK, PIN_DATA_BASE);

    gpio_init(PIN_RESET);
    gpio_init(PIN_BL);
    gpio_init(PIN_CS);
    
    gpio_set_dir(PIN_RESET, GPIO_OUT);
    gpio_set_dir(PIN_BL, GPIO_OUT);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    
    gpio_put(PIN_RESET,1);
    sleep_ms(5); 
    gpio_put(PIN_RESET,0);
    sleep_ms(15);
    gpio_put(PIN_RESET,1);
    sleep_ms(15);
    gpio_put(PIN_CS, 0);

    lcd_init(st7789_init_seq);
    gpio_put(PIN_BL, 1);

    // Other SDKs: static image on screen, lame, boring
    // Raspberry Pi Pico SDK: spinning image on screen, bold, exciting

    // Lane 0 will be u coords (bits 8:1 of addr offset), lane 1 will be v
    // coords (bits 16:9 of addr offset), and we'll represent coords with
    // 16.16 fixed point. ACCUM0,1 will contain current coord, BASE0/1 will
    // contain increment vector, and BASE2 will contain image base pointer
#define UNIT_LSB 16
    interp_config lane0_cfg = interp_default_config();
    interp_config_set_shift(&lane0_cfg, UNIT_LSB - 1); // -1 because 2 bytes per pixel
    interp_config_set_mask(&lane0_cfg, 1, 1 + (LOG_IMAGE_SIZE - 1));
    interp_config_set_add_raw(&lane0_cfg, true); // Add full accumulator to base with each POP
    interp_config lane1_cfg = interp_default_config();
    interp_config_set_shift(&lane1_cfg, UNIT_LSB - (1 + LOG_IMAGE_SIZE));
    interp_config_set_mask(&lane1_cfg, 1 + LOG_IMAGE_SIZE, 1 + (2 * LOG_IMAGE_SIZE - 1));
    interp_config_set_add_raw(&lane1_cfg, true);

    interp_set_config(interp0, 0, &lane0_cfg);
    interp_set_config(interp0, 1, &lane1_cfg);
    interp0->base[2] = (uint32_t) raspberry_256x256;
    
    lcd_start_pixels();
    float theta = 0.f;
    float theta_max = 2.f * (float) M_PI;
    while (1) {
        theta += 0.02f;
        if (theta > theta_max)
            theta -= theta_max;
        int32_t rotate[4] = {
                cosf(theta) * (1 << UNIT_LSB), -sinf(theta) * (1 << UNIT_LSB),
                sinf(theta) * (1 << UNIT_LSB), cosf(theta) * (1 << UNIT_LSB)
        };
        interp0->base[0] = rotate[0];
        interp0->base[1] = rotate[2];
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            interp0->accum[0] = rotate[1] * y;
            interp0->accum[1] = rotate[3] * y;
            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                uint16_t colour = *(uint16_t *) (interp0->pop[2]);
                lcd_write(true, false, colour);
                lcd_write(true, false, colour >> 8);
                lcd_write(true, false, colour);
            }

            if(y % 10 == 0) {
                lcd_flush();
            }
        }
    }
}