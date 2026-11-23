#include "drv_buttons.h"
#include "config.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BTN_COUNT           4
#define DEBOUNCE_TICKS      pdMS_TO_TICKS(30)

typedef struct {
    gpio_num_t pin;
    uint8_t    event_bit;
    bool       stable_pressed;
    bool       last_raw_pressed;
    TickType_t last_change_tick;
} btn_state_t;

static btn_state_t s_btns[BTN_COUNT] = {
    { .pin = PIN_BTN_MENU, .event_bit = BTN_EVENT_MENU },
    { .pin = PIN_BTN_UP,   .event_bit = BTN_EVENT_UP   },
    { .pin = PIN_BTN_DOWN, .event_bit = BTN_EVENT_DOWN },
    { .pin = PIN_BTN_OK,   .event_bit = BTN_EVENT_OK   },
};

esp_err_t drv_buttons_init(void)
{
    uint64_t pin_mask = (1ULL << PIN_BTN_MENU) |
                        (1ULL << PIN_BTN_UP)   |
                        (1ULL << PIN_BTN_DOWN) |
                        (1ULL << PIN_BTN_OK);

    gpio_config_t io_conf = {
        .pin_bit_mask = pin_mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) return err;

    TickType_t now = xTaskGetTickCount();
    for (int i = 0; i < BTN_COUNT; i++) {
        bool pressed = (gpio_get_level(s_btns[i].pin) == 0);
        s_btns[i].stable_pressed = pressed;
        s_btns[i].last_raw_pressed = pressed;
        s_btns[i].last_change_tick = now;
    }
    return ESP_OK;
}

uint8_t drv_buttons_poll(void)
{
    uint8_t events = BTN_EVENT_NONE;
    TickType_t now = xTaskGetTickCount();

    for (int i = 0; i < BTN_COUNT; i++) {
        bool raw_pressed = (gpio_get_level(s_btns[i].pin) == 0);

        if (raw_pressed != s_btns[i].last_raw_pressed) {
            s_btns[i].last_raw_pressed = raw_pressed;
            s_btns[i].last_change_tick = now;
        } else if ((now - s_btns[i].last_change_tick) >= DEBOUNCE_TICKS) {
            if (raw_pressed != s_btns[i].stable_pressed) {
                s_btns[i].stable_pressed = raw_pressed;
                if (raw_pressed) {
                    events |= s_btns[i].event_bit;
                }
            }
        }
    }

    return events;
}