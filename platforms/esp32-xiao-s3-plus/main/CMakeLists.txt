idf_component_register(
    SRCS "zk_arche_app.c"
    INCLUDE_DIRS "."
    REQUIRES
        zk_arche_core
        libsodium
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        lwip
        driver
)
