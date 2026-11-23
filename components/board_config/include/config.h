#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver/gpio.h"
#include "hal/adc_types.h"
#include "hal/ledc_types.h"

// ================= 1. SO DO CHAN (ESP32-WROOM-32D) =================
#define PIN_DS18B20_DATA        GPIO_NUM_32     // Cap nguon 3.3V (KHONG cap 5V)
#define PIN_MQ2_AO              GPIO_NUM_35     // Qua cau phan ap 20k/20k (max ~2.5V)
#define MQ2_ADC_UNIT            ADC_UNIT_1
#define MQ2_ADC_CHANNEL         ADC_CHANNEL_7   // GPIO 35 = ADC1_CH7
#define MQ2_ADC_ATTEN           ADC_ATTEN_DB_12

#define PIN_I2C_SDA             GPIO_NUM_21
#define PIN_I2C_SCL             GPIO_NUM_22
#define LCD_I2C_ADDR            0x27            // Doi 0x3F neu mach khac
#define LCD_I2C_FREQ_HZ         100000
#define LCD_COLS                20
#define LCD_ROWS                4

#define PIN_BTN_MENU            GPIO_NUM_13     // INPUT_PULLUP, nhan = LOW
#define PIN_BTN_UP              GPIO_NUM_16
#define PIN_BTN_DOWN            GPIO_NUM_17
#define PIN_BTN_OK              GPIO_NUM_4

#define PIN_FAN_PWM             GPIO_NUM_25     // Qua tro 1k -> Base TIP122
#define PIN_BUZZER              GPIO_NUM_19     // Qua tro 1k -> Base 2N2222
#define PIN_LED_ALARM           GPIO_NUM_18     // Qua tro 220R/1k -> LED

// ================= 2. THONG SO PHAN CUNG =================
#define FAN_PWM_FREQ_HZ         5000            // 5 - 25 kHz cho TIP122
#define FAN_PWM_RES_BITS        LEDC_TIMER_10_BIT
#define FAN_MIN_DUTY_PCT        40              // Toc do toi thieu de quat quay

#define BUZZER_IS_PASSIVE       0               // 0: Active, 1: Passive
#define BUZZER_TONE_HZ          2500

#endif // BOARD_CONFIG_H