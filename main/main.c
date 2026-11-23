#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "app_mode.h"
#include "config.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "=== BANNER | FW=%s | MODE=%d | RESET_REASON=%d ===",
             FW_VERSION, APP_MODE, (int)esp_reset_reason());
}