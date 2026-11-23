#ifndef APP_LOGIC_H
#define APP_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SYS_STATE_NORMAL = 0,       // Binh thuong, quat tat
    SYS_STATE_COOLING,          // Dang lam mat theo nhiet do
    SYS_STATE_ALARM_TEMP,       // Bao dong qua nhiet
    SYS_STATE_ALARM_SMOKE,      // Bao dong khoi/gas
    SYS_STATE_SENSOR_FAULT      // Loi cam bien (mat ket noi DS18B20)
} sys_state_t;

typedef struct {
    float    temp_fan_on;       // Nhiet do bat dau bat quat (VD: 30.0 C)
    float    temp_fan_max;      // Nhiet do quat 100% (VD: 40.0 C)
    float    temp_alarm;        // Nguong bao dong qua nhiet (VD: 45.0 C)
    float    temp_hyst;         // Do tre hysteresis nhiet do (VD: 1.0 C)
    uint16_t smoke_alarm;       // Nguong bao dong khoi ADC (VD: 1500)
    uint16_t smoke_hyst;        // Do tre hysteresis khoi (VD: 100)
    uint8_t  fan_min_pct;       // Toc do toi thieu de quat quay (VD: 40%)
} app_config_t;

typedef struct {
    float    temp_c;
    bool     temp_valid;
    uint16_t smoke_raw;
    bool     smoke_valid;
} app_input_t;

typedef struct {
    sys_state_t state;
    uint8_t     fan_duty_pct;   // 0, hoac fan_min_pct..100
    bool        buzzer_on;
    bool        led_alarm_on;
    bool        notify_trigger; // True dung 1 lan khi moi vao trang thai bao dong
} app_output_t;

typedef struct {
    app_config_t cfg;
    bool         fan_active;
    bool         temp_alarm_active;
    bool         smoke_alarm_active;
    bool         prev_any_alarm;
} app_logic_ctx_t;

void app_logic_init(app_logic_ctx_t *ctx, const app_config_t *cfg);
void app_logic_step(app_logic_ctx_t *ctx, const app_input_t *in, app_output_t *out);

#endif // APP_LOGIC_H