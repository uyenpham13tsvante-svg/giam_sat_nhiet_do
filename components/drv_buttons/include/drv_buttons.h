#ifndef DRV_BUTTONS_H
#define DRV_BUTTONS_H

#include <stdint.h>
#include "esp_err.h"

#define BTN_EVENT_NONE      0x00
#define BTN_EVENT_MENU      (1 << 0)
#define BTN_EVENT_UP        (1 << 1)
#define BTN_EVENT_DOWN      (1 << 2)
#define BTN_EVENT_OK        (1 << 3)

esp_err_t drv_buttons_init(void);
uint8_t   drv_buttons_poll(void); // Goi dinh ky (VD: moi 10-20ms), tra ve bitmask su kien nhan

#endif // DRV_BUTTONS_H