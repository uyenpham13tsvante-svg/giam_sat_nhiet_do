#include "drv_lcd.h"
#include "config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "DRV_LCD";

// Bit mapping PCF8574 -> HD44780
#define LCD_BIT_RS          0x01
#define LCD_BIT_EN          0x04
#define LCD_BIT_BACKLIGHT   0x08

static i2c_master_bus_handle_t s_bus_handle = NULL;
static i2c_master_dev_handle_t s_dev_handle = NULL;
static uint8_t s_backlight = LCD_BIT_BACKLIGHT;
static bool s_ready = false;

static esp_err_t pcf8574_write(uint8_t data)
{
    if (!s_dev_handle) return ESP_ERR_INVALID_STATE;
    uint8_t buf = data | s_backlight;
    return i2c_master_transmit(s_dev_handle, &buf, 1, 50);
}

static void lcd_pulse_enable(uint8_t data)
{
    pcf8574_write(data | LCD_BIT_EN);
    esp_rom_delay_us(1);
    pcf8574_write(data & ~LCD_BIT_EN);
    esp_rom_delay_us(50);
}

static void lcd_write_4bits(uint8_t nibble_with_rs)
{
    pcf8574_write(nibble_with_rs);
    lcd_pulse_enable(nibble_with_rs);
}

static void lcd_send_byte(uint8_t val, uint8_t rs_flag)
{
    uint8_t high = (val & 0xF0) | rs_flag;
    uint8_t low  = ((val << 4) & 0xF0) | rs_flag;
    lcd_write_4bits(high);
    lcd_write_4bits(low);
}

static void lcd_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
}

static void lcd_data(uint8_t data)
{
    lcd_send_byte(data, LCD_BIT_RS);
}

esp_err_t drv_lcd_init(void)
{
    if (s_ready) return ESP_OK;

    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = PIN_I2C_SCL,
        .sda_io_num = PIN_I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Khoi tao I2C bus that bai: %s", esp_err_to_name(err));
        return err;
    }

    // Tu dong do dia chi 0x27 (hoac 0x3F neu mach khac)
    uint8_t detected_addr = 0;
    if (i2c_master_probe(s_bus_handle, LCD_I2C_ADDR, 50) == ESP_OK) {
        detected_addr = LCD_I2C_ADDR;
    } else if (i2c_master_probe(s_bus_handle, 0x3F, 50) == ESP_OK) {
        detected_addr = 0x3F;
    } else {
        ESP_LOGE(TAG, "Khong tim thay LCD I2C tai 0x%02X hoac 0x3F!", LCD_I2C_ADDR);
        i2c_del_master_bus(s_bus_handle);
        s_bus_handle = NULL;
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "Tim thay LCD I2C tai dia chi 0x%02X", detected_addr);

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = detected_addr,
        .scl_speed_hz = LCD_I2C_FREQ_HZ,
    };

    err = i2c_master_bus_add_device(s_bus_handle, &dev_cfg, &s_dev_handle);
    if (err != ESP_OK) {
        i2c_del_master_bus(s_bus_handle);
        s_bus_handle = NULL;
        return err;
    }

    // Chuoi khoi tao chuan HD44780 che do 4-bit
    vTaskDelay(pdMS_TO_TICKS(50));
    lcd_write_4bits(0x30);
    esp_rom_delay_us(4500);
    lcd_write_4bits(0x30);
    esp_rom_delay_us(4500);
    lcd_write_4bits(0x30);
    esp_rom_delay_us(150);
    lcd_write_4bits(0x20); // Chuyen sang 4-bit

    lcd_cmd(0x28); // 4-bit, 2+ dong, font 5x8
    lcd_cmd(0x0C); // Bat hien thi, tat con tro
    lcd_cmd(0x06); // Tu dong dich con tro sang phai
    drv_lcd_clear();

    s_ready = true;
    return ESP_OK;
}

void drv_lcd_clear(void)
{
    if (!s_dev_handle) return;
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void drv_lcd_set_backlight(bool on)
{
    s_backlight = on ? LCD_BIT_BACKLIGHT : 0x00;
    if (s_dev_handle) {
        pcf8574_write(0x00);
    }
}

void drv_lcd_set_cursor(uint8_t col, uint8_t row)
{
    if (!s_dev_handle) return;
    static const uint8_t row_offsets[4] = {0x00, 0x40, 0x14, 0x54};
    if (row >= LCD_ROWS) row = LCD_ROWS - 1;
    if (col >= LCD_COLS) col = LCD_COLS - 1;
    lcd_cmd(0x80 | (col + row_offsets[row]));
}

void drv_lcd_print(const char *str)
{
    if (!s_dev_handle || !str) return;
    while (*str) {
        lcd_data((uint8_t)(*str++));
    }
}

void drv_lcd_print_line(uint8_t row, const char *str)
{
    if (!s_dev_handle) return;
    char line_buf[LCD_COLS + 1];
    size_t len = str ? strlen(str) : 0;
    if (len > LCD_COLS) len = LCD_COLS;

    memset(line_buf, ' ', LCD_COLS);
    if (len > 0) {
        memcpy(line_buf, str, len);
    }
    line_buf[LCD_COLS] = '\0';

    drv_lcd_set_cursor(0, row);
    drv_lcd_print(line_buf);
}