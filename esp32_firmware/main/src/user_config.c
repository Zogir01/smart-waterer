#include "../inc/user_config.h"
#include "nvs.h"

#define CONFIG_NAMESPACE "usercfg"

esp_err_t config_save(const user_config_t *config) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    nvs_set_i32(handle, "wat-time", config->watering_time);
    nvs_set_i32(handle, "samp-cnt", config->sample_count);
    nvs_set_i32(handle, "rd-delay", config->read_delay);
    nvs_set_i32(handle, "dry-thrd", config->dry_threshold);

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

esp_err_t config_load(user_config_t *config) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;
    
    nvs_get_i32(handle, "wat-time", &config->watering_time);
    nvs_get_i32(handle, "samp-cnt", &config->sample_count);
    nvs_get_i32(handle, "rd-delay", &config->read_delay);
    nvs_get_i32(handle, "dry-thrd", &config->dry_threshold);

    nvs_close(handle);
    return ESP_OK;
}