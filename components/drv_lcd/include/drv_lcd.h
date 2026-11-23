#ifndef DRV_LCD_H
#define DRV_LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

esp_err_t drv_lcd_init(void);
void      drv_lcd_clear(void);
void      drv_lcd_set_backlight(bool on);
void      drv_lcd_set_cursor(uint8_t col, uint8_t row);
void      drv_lcd_print(const char *str);
void      drv_lcd_print_line(uint8_t row, const char *str); // Ghi du 20 ky tu, tu xoa ky tu thua

#endif // DRV_LCD_H