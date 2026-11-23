#include <stdio.h>
#include <assert.h>
#include "app_logic.h"

static app_config_t default_cfg(void)
{
    app_config_t cfg = {
        .temp_fan_on  = 30.0f,
        .temp_fan_max = 40.0f,
        .temp_alarm   = 45.0f,
        .temp_hyst    = 1.0f,
        .smoke_alarm  = 1500,
        .smoke_hyst   = 100,
        .fan_min_pct  = 40
    };
    return cfg;
}

int main(void)
{
    app_logic_ctx_t ctx;
    app_config_t cfg = default_cfg();
    app_input_t in;
    app_output_t out;

    app_logic_init(&ctx, &cfg);

    // Test 1: Binh thuong (25 C, khong khoi)
    in = (app_input_t){ .temp_c = 25.0f, .temp_valid = true, .smoke_raw = 300, .smoke_valid = true };
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_NORMAL);
    assert(out.fan_duty_pct == 0);
    assert(!out.buzzer_on && !out.led_alarm_on && !out.notify_trigger);

    // Test 2: Bat dau lam mat (30 C -> 40%)
    in.temp_c = 30.0f;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_COOLING);
    assert(out.fan_duty_pct == 40);

    // Test 3: Lam mat tuyen tinh (35 C -> giua 40% va 100% = 70%)
    in.temp_c = 35.0f;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_COOLING);
    assert(out.fan_duty_pct == 70);

    // Test 4: Hysteresis tat quat (giam xuong 29.5 C van chay 40%, xuong 28.9 C moi tat)
    in.temp_c = 29.5f;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_COOLING && out.fan_duty_pct == 40);

    in.temp_c = 28.9f;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_NORMAL && out.fan_duty_pct == 0);

    // Test 5: Qua nhiet (46 C -> bao dong, notify_trigger = true lan dau, lan 2 = false)
    in.temp_c = 46.0f;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_ALARM_TEMP);
    assert(out.fan_duty_pct == 100 && out.buzzer_on && out.led_alarm_on);
    assert(out.notify_trigger == true);

    app_logic_step(&ctx, &in, &out);
    assert(out.notify_trigger == false); // Khong spam

    // Test 6: Bao dong khoi uu tien cao hon (smoke = 1600)
    in.smoke_raw = 1600;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_ALARM_SMOKE);
    assert(out.fan_duty_pct == 100 && out.buzzer_on);

    // Test 7: Loi cam bien nhiet do (khi het khoi)
    in.smoke_raw = 300;
    in.temp_valid = false;
    app_logic_step(&ctx, &in, &out);
    assert(out.state == SYS_STATE_SENSOR_FAULT);
    assert(out.fan_duty_pct == 0 && !out.buzzer_on && out.led_alarm_on);

    printf("ALL 7 APP_LOGIC TESTS PASSED!\n");
    return 0;
}