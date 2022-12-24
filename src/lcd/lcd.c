
#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/interp.h"
#include "lcd.h"
#include "lcd.pio.h"

#define BUF_SIZE 10000

#define PIN_DATA_BASE 7 // 7 (10) - MOSI, 8 (11) - DC, 9 (12) - CS
#define PIN_DC 11 // 5 нога лев
#define PIN_CS 9 // 4 нога лев
#define PIN_DIN 7// 10 нога лев  0 
#define PIN_CLK 6// 9 нога лев 1

#define PIN_RESET 4 // 6 нога лев
#define PIN_BL 5

#define SERIAL_CLK_DIV 1.f


// Format: cmd length (including cmd byte), post delay in units of 5 ms, then cmd payload
// Note the delays have been shortened a little
    
static const uint8_t st7789_init_seq[] = {
        5, 0, 0xF7, 0xA9, 0x51, 0x2C, 0x82, // Adjust Control 3 
        3, 0, 0xC0, 0x11, 0x09, // Power Control 1
        2, 0, 0xC1, 0x41, // Power Control 1
        4, 0, 0xC5, 0x00, 0x0A, 0x80, //  VCOM Control 
        3, 0, 0xB1, 0xB0, 0x11, //  Frame Rate Control (In Normal Mode/Full Colors) 
        2, 0, 0xB4, 0x02, // Display Inversion Control (B4h)
        3, 0, 0xB6, 0x02, 0x22, //   Display Function Control (B6h) 
        2, 0, 0xB7, 0xC6, //   Entry Mode Set
        3, 0, 0xBE, 0x00, 0x04, // HS Lanes Control
        2, 0, 0xE9, 0x00, // Set Image Function
        2, 0, 0x36, 0xA8, // Memory Access Control (MADCTL)
        2, 0, 0x3A, 0x51, // Interface Pixel Format,
        16, 0, 0xE0, 0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F, // Positive Gamma Control
        16, 0, 0xE1, 0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F, // Negative Gamma Control
        1, 120, 0x11, // sleep out
        1, 0, 0x29, // display on
        0
};

typedef union
{
    struct __attribute__((packed))
    {
        uint8_t reserved:3;

        bool d03:1;
        bool dc3:1;
        uint8_t d3:7;
        
        bool n3:1;
        bool d02:1;
        bool dc2:1;
        uint8_t d2:7;

        bool n2:1;
        bool d01:1;
        bool dc1:1;
        uint8_t d1:7;
    };
    uint32_t word;
} Packet;

static uint32_t buf[BUF_SIZE];
static uint32_t buf_len = 0;

static uint dma_chan = 0;

static volatile Packet word_buf;
static uint32_t word_buf_c = 0;
 

static inline void _start_dma(uint32_t* b, uint32_t transactions) {
    dma_channel_wait_for_finish_blocking(dma_chan);
    dma_channel_transfer_from_buffer_now(dma_chan, b, transactions);
}

static inline void _flushIsFull() {
    uint32_t half_size = BUF_SIZE / 2;
    if (buf_len == half_size) { 
        _start_dma(buf, buf_len);
    } else if (buf_len == BUF_SIZE) {
        _start_dma(&buf[half_size], buf_len - half_size);
        buf_len = 0;
    }
}

static inline void _last_word() {
    if (word_buf_c != 0) {
        buf[buf_len++] = word_buf.word;
        word_buf.word = 0;
        word_buf_c = 0;
    }
}

inline void lcd_flush() {
    uint32_t half_size = BUF_SIZE / 2;
    _last_word();
    if (buf_len <= half_size) {
        if (buf_len > 0)
            _start_dma(buf, buf_len);
    } else {
        uint32_t l = buf_len - half_size;
        if (l > 0)
            _start_dma(&buf[half_size], l);
    }
    buf_len = 0;
}

inline void lcd_write(bool dc, const uint8_t data) {
    bool d0 = data & 1; // get LSB
    uint8_t d = data >> 1;
    switch (word_buf_c)
    {
        case 0:
            word_buf.d01 = d0;
            word_buf.dc1 = dc;
            word_buf.d1 = d; 
        break;
        case 1:
            word_buf.n2 = 1;
            word_buf.d02 = d0;
            word_buf.dc2 = dc;
            word_buf.d2 = d;
        break;
        case 2:
            word_buf.n3 = 1;
            word_buf.d03 = d0;
            word_buf.dc3 = dc;
            word_buf.d3 = d;
        break;
        default:
    }
    word_buf_c++;
    if(word_buf_c == 3) {
        buf[buf_len++] = word_buf.word;
        word_buf.word = 0;
        word_buf_c = 0;
    }

    _flushIsFull();
    
   /*  printf("CNT %d, D0 %d, DC %d\n", word_buf.c, d0, dc);
    printf("DATA SRC: "
           PRINTF_BINARY_PATTERN_INT8 "\n",
           PRINTF_BYTE_TO_BINARY_INT8(data));
    printf("PAK: "
           PRINTF_BINARY_PATTERN_INT32 "\n",
           PRINTF_BYTE_TO_BINARY_INT32(word_buf.word)); */
}

inline void lcd_write_cmd(const uint8_t *cmd, size_t count) {
    lcd_write(0, *cmd++);
    if (count >= 2) {
        for (size_t i = 0; i < count - 1; ++i) {     
            lcd_write(1, *cmd++);
        }   
    }
}

inline void lcd_write_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
   /*  uint8_t cmd[] = {
        , // set x pos
        ,  // set y pos
         // strart RAM write (RAMWR)
    }; */
    //uint8_t* cmd_ptr = cmd;
    uint8_t caset[] = {0x2a, x1>>8, x1, x2>>8, x2 };
    uint8_t paset[] = {0x2b, y1>>8, y1, y2>>8, y2 };
    uint8_t ramwr[] = {0x2c};
    lcd_write_cmd(caset, 5);
    lcd_write_cmd(paset, 5);
    lcd_write_cmd(ramwr, 1);
}

inline void lcd_write_ramwr() {
    uint8_t cmd = 0x2c; // RAMWR
    lcd_write_cmd(&cmd, 1);
    lcd_flush();
}

inline void lcd_write_pixel_3b(uint16_t x, uint16_t y, ColorRGBByte c) {
    lcd_write_addr(x, y, x + 2, y + 2);
    lcd_write(1, c.color);
    lcd_write(1, c.color);
    lcd_write(1, c.color);
    lcd_write(1, c.color);
}


inline void lcd_write_color_3b(ColorRGBByte c) {
    lcd_write(1, c.color);
}


static inline void lcd_init_seq(const uint8_t *init_seq) {
    const uint8_t *cmd = init_seq;
    while (*cmd) {
        lcd_write_cmd(cmd + 2, *cmd);
        lcd_flush();
        sleep_ms(*(cmd + 1) * 5);
        cmd += *cmd + 2;
    }
}


void lcd_init(PIO pio, uint sm) {
    uint offset = pio_add_program(pio, &lcd_program);

    dma_channel_config dma_cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
    dma_channel_configure(dma_chan, &dma_cfg,
        &pio->txf[sm], // Destination pointer
        buf,  // Source pointer         
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

    lcd_init_seq(st7789_init_seq);
    gpio_put(PIN_BL, 1);
}

inline void lcd_set_color_rgb(ColorRGBByte* c, bool r, bool g, bool b) {
    c->r = r;
    c->r0 = r;
    c->g = g;
    c->g0 = g;
    c->b = b;
    c->b0 = b;
}