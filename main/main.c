#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "app_mode.h"
#include "config.h"
#include "drv_lcd.h"
#include "drv_buttons.h"

static const char *TAG = "MAIN";

#if APP_MODE == MODE_TEST_LCD_BTN
static void run_test_lcd_buttons(void)
{
    ESP_ERROR_CHECK(drv_buttons_init());

    esp_err_t lcd_err = drv_lcd_init();
    bool lcd_ok = (lcd_err == ESP_OK);

    if (lcd_ok) {
        drv_lcd_print_line(0, "TEST LCD & 4 NUT");
        drv_lcd_print_line(1, "Nut nhan: NONE");
        drv_lcd_print_line(2, "So lan nhan: 0");
        drv_lcd_print_line(3, "OK: Bat/Tat den nen");
    } else {
        ESP_LOGW(TAG, "Khong phat hien LCD, chi test nut nhan qua Serial Log");
    }

    uint32_t press_count = 0;
    bool backlight_on = true;
    char line[LCD_COLS + 1];

    while (1) {
        uint8_t ev = drv_buttons_poll();
        if (ev != BTN_EVENT_NONE) {
            const char *btn_name = "UNKNOWN";

            if (ev & BTN_EVENT_MENU) {
                btn_name = "MENU";
            } else if (ev & BTN_EVENT_UP) {
                btn_name = "UP";
            } else if (ev & BTN_EVENT_DOWN) {
                btn_name = "DOWN";
            } else if (ev & BTN_EVENT_OK) {
                btn_name = "OK";
                backlight_on = !backlight_on;
                if (lcd_ok) {
                    drv_lcd_set_backlight(backlight_on);
                }
            }

            press_count++;
            ESP_LOGI(TAG, "Nhan nut: %s | Tong=%lu | Backlight=%s",
                     btn_name, (unsigned long)press_count, backlight_on ? "ON" : "OFF");

            if (lcd_ok) {
                snprintf(line, sizeof(line), "Nut nhan: %-10s", btn_name);
                drv_lcd_print_line(1, line);

                snprintf(line, sizeof(line), "So lan nhan: %lu", (unsigned long)press_count);
                drv_lcd_print_line(2, line);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
#endif

void app_main(void)
{
    ESP_LOGI(TAG, "=== BANNER | FW=%s | MODE=%d | RESET_REASON=%d ===",
             FW_VERSION, APP_MODE, (int)esp_reset_reason());

#if APP_MODE == MODE_TEST_LCD_BTN
    run_test_lcd_buttons();
#else
    ESP_LOGW(TAG, "Che do APP_MODE=%d chua duoc trien khai", APP_MODE);
#endif
}