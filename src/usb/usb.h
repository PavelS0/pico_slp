#ifndef USB_H_
#define USB_H_

#include <stdio.h>

void led_blinking_task(void);
void cdc_task(void);
void usb_init(void);
void usb_task(void);
uint8_t usb_ready(void);
uint32_t usb_write(const uint8_t *buffer, uint32_t size);
uint32_t usb_read(void *buffer, uint32_t size);
uint32_t usb_write_flush();

#endif /* USB_H_ */
