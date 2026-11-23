#include "app_logic.h"
#include <string.h>

void app_logic_init(app_logic_ctx_t *ctx, const app_config_t *cfg)
{
    if (!ctx || !cfg) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->cfg = *cfg;
}

static uint8_t calc_fan_duty(const app_config_t *cfg, float temp_c)
{
    if (temp_c >= cfg->temp_fan_max) {
        return 100;
    }
    if (temp_c <= cfg->temp_fan_on) {
        return cfg->fan_min_pct;
    }
    float span = cfg->temp_fan_max - cfg->temp_fan_on;
    if (span <= 0.01f) {
        return 100;
    }
    float ratio = (temp_c - cfg->temp_fan_on) / span;
    float duty = (float)cfg->fan_min_pct + ratio * (float)(100 - cfg->fan_min_pct);
    if (duty > 100.0f) duty = 100.0f;
    if (duty < (float)cfg->fan_min_pct) duty = (float)cfg->fan_min_pct;
    return (uint8_t)(duty + 0.5f); // Lam tron gan nhat
}

void app_logic_step(app_logic_ctx_t *ctx, const app_input_t *in, app_output_t *out)
{
    if (!ctx || !in || !out) return;
    memset(out, 0, sizeof(*out));

    const app_config_t *c = &ctx->cfg;

    // 1. Cap nhat trang thai bao dong khoi (co hysteresis)
    if (in->smoke_valid) {
        if (!ctx->smoke_alarm_active && in->smoke_raw >= c->smoke_alarm) {
            ctx->smoke_alarm_active = true;
        } else if (ctx->smoke_alarm_active &&
                   in->smoke_raw <= (c->smoke_alarm > c->smoke_hyst ? c->smoke_alarm - c->smoke_hyst : 0)) {
            ctx->smoke_alarm_active = false;
        }
    } else {
        ctx->smoke_alarm_active = false;
    }

    // 2. Cap nhat trang thai nhiet do (co hysteresis)
    if (in->temp_valid) {
        if (!ctx->temp_alarm_active && in->temp_c >= c->temp_alarm) {
            ctx->temp_alarm_active = true;
        } else if (ctx->temp_alarm_active && in->temp_c <= (c->temp_alarm - c->temp_hyst)) {
            ctx->temp_alarm_active = false;
        }

        if (!ctx->fan_active && in->temp_c >= c->temp_fan_on) {
            ctx->fan_active = true;
        } else if (ctx->fan_active && in->temp_c <= (c->temp_fan_on - c->temp_hyst)) {
            ctx->fan_active = false;
        }
    } else {
        ctx->temp_alarm_active = false;
        ctx->fan_active = false;
    }

    // 3. Quyet dinh ngo ra theo thu tu uu tien: Khoi > Qua nhiet > Loi cam bien > Lam mat > Binh thuong
    if (ctx->smoke_alarm_active) {
        out->state = SYS_STATE_ALARM_SMOKE;
        out->fan_duty_pct = 100;
        out->buzzer_on = true;
        out->led_alarm_on = true;
    } else if (ctx->temp_alarm_active) {
        out->state = SYS_STATE_ALARM_TEMP;
        out->fan_duty_pct = 100;
        out->buzzer_on = true;
        out->led_alarm_on = true;
    } else if (!in->temp_valid) {
        out->state = SYS_STATE_SENSOR_FAULT;
        out->fan_duty_pct = 0;
        out->buzzer_on = false;
        out->led_alarm_on = true; // Bat LED bao loi, khong hu coi gay on
    } else if (ctx->fan_active) {
        out->state = SYS_STATE_COOLING;
        out->fan_duty_pct = calc_fan_duty(c, in->temp_c);
        out->buzzer_on = false;
        out->led_alarm_on = false;
    } else {
        out->state = SYS_STATE_NORMAL;
        out->fan_duty_pct = 0;
        out->buzzer_on = false;
        out->led_alarm_on = false;
    }

    // 4. Chi kich hoat gui thong bao 1 lan khi moi chuyen sang bao dong (tranh spam Telegram)
    bool any_alarm = (ctx->smoke_alarm_active || ctx->temp_alarm_active);
    out->notify_trigger = (any_alarm && !ctx->prev_any_alarm);
    ctx->prev_any_alarm = any_alarm;
}