idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_system board_config drv_buttons drv_actuators drv_ds18b20 drv_mq2 drv_lcd app_logic
)
