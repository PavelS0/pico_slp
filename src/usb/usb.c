#include "usb.h"
#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
static uint8_t ready = 0;


static struct
{
  uint8_t buf[8192];
  uint16_t buf_len;
  uint16_t current_len;
  uint8_t busy;
} write = {0};

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
  ready = 1;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
  ready = 0;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
  ready = 0;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
  ready = 1;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}


// echo to either Serial0 or Serial1
// with Serial0 as all lower case, Serial1 as all upper case
static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
  for(uint32_t i=0; i<count; i++)
  {
    if (itf == 0)
    {
      // echo back 1st port as lower case
      if (isupper(buf[i])) buf[i] += 'a' - 'A';
    }
    else
    {
      // echo back additional ports as upper case
      if (islower(buf[i])) buf[i] -= 'a' - 'A';
    }

    tud_cdc_n_write_char(itf, buf[i]);

    if ( buf[i] == '\r' ) tud_cdc_n_write_char(itf, '\n');
  }
  tud_cdc_n_write_flush(itf);
}

void usb_init(void) {
    board_init();
    tusb_init();
}

uint8_t usb_ready(void) {
   return ready;
}

uint8_t usb_write_busy(void) {
   return write.busy;
}

void usb_task(void) {
   tud_task();
}
//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
  uint8_t itf;

  for (itf = 0; itf < CFG_TUD_CDC; itf++)
  {
    if ( tud_cdc_n_connected(itf) )
    {
      if ( tud_cdc_n_available(itf) )
      {
        uint8_t buf[64];

        uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

        // echo back to both serial ports
        //echo_serial_port(0, buf, count);
        //echo_serial_port(1, buf, count);
      }
    }
    uint32_t a = tud_cdc_n_write_available(0);
    if (a > 16) {
      uint32_t sum = write.current_len + a;
      if (sum < write.buf_len) {
        tud_cdc_n_write(itf, &write.buf[write.current_len], a);
        write.current_len = sum;
      } else {
        tud_cdc_n_write(itf, &write.buf[write.current_len], write.buf_len - write.current_len);
        write.current_len = 0;
        write.buf_len = 0;
        write.busy = 0;
      }
    }
  }
}

inline uint32_t usb_write(const uint8_t *buffer, uint32_t size) {
  uint16_t i = 0;
  memcpy(&write.buf[write.buf_len], buffer, size);
  write.buf_len += size;
  return size;
}

inline uint32_t usb_read(void *buffer, uint32_t size) {
    return tud_cdc_n_read(0, buffer, size);
}

inline uint32_t usb_write_flush() {
    write.busy = 1;
}