idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        driver
        esp_adc
        esp_timer
        freertos
        log
        spi_flash
    PRIV_REQUIRES
        esp_driver_gpio
        esp_driver_spi
)
