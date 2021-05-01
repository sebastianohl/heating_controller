#include "heating_controller.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "heating_controller";
#define STORAGE_NAMESPACE "storage"

void init_controller_params()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    ESP_LOGI(TAG, "read controler params from NVS");
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return;

    size_t s = sizeof(float);
    err = nvs_get_blob(my_handle, "setpoint", &controller->setpoint, &s);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto error;
    err = nvs_get_blob(my_handle, "hysteresis", &controller->hysteresis, &s);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto error;
    err = nvs_get_blob(my_handle, "emergency", &controller->emergency, &s);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto error;
    err = nvs_get_blob(my_handle, "reaction_time", &controller->reaction_time, &s);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto error;
    err = nvs_get_blob(my_handle, "step_time", &controller->step_time, &s);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto error;
    err = nvs_get_blob(my_handle, "max_value", &controller->max_value, &s);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto error;

error:
    // Close
    nvs_close(my_handle);
}

void write_controller_params()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    ESP_LOGI(TAG, "store controller params to NVS");
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;

    err = nvs_set_blob(my_handle, "setpoint", &controller->setpoint, sizeof(float));
    if (err != ESP_OK) { ESP_LOGE(TAG,"error setting setpoint:%d", err); goto error; }
    err = nvs_set_blob(my_handle, "hysteresis", &controller->hysteresis, sizeof(float));
    if (err != ESP_OK) { ESP_LOGE(TAG,"error setting hystersis:%d", err); goto error; }
    err = nvs_set_blob(my_handle, "emergency", &controller->emergency, sizeof(float));
    if (err != ESP_OK) { ESP_LOGE(TAG,"error setting emergency:%d", err); goto error; }
    err = nvs_set_blob(my_handle, "reaction_time", &controller->reaction_time, sizeof(float));
    if (err != ESP_OK) { ESP_LOGE(TAG,"error setting reaction time:%d", err); goto error; }
    err = nvs_set_blob(my_handle, "step_time", &controller->step_time, sizeof(float));
    if (err != ESP_OK) { ESP_LOGE(TAG,"error setting step time:%d", err); goto error; }
    err = nvs_set_blob(my_handle, "max_value", &controller->max_value, sizeof(float));
    if (err != ESP_OK) { ESP_LOGE(TAG,"error setting max value:%d", err); goto error; }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) { ESP_LOGE(TAG,"error committing:%d", err); goto error; }

error:
    // Close
    nvs_close(my_handle);
}

void update_temperatur_sensor(struct homie_handle_s *handle, int node,
                              int property)
{
    char value[100];
    float temp = temperatureSensors_read_temperature_str(
        temperatureSensors, *((device_rom_code_t *)handle->nodes[node]
                                  .properties[property]
                                  .user_data));
    ESP_LOGI(TAG,"temp %d: %.2f",
           (int)handle->nodes[node].properties[property].user_data, temp);
    sprintf(value, "%.2f", temp);
    homie_publish_property_value(handle, node, property, value);
}

void update_controller_setpoint_temperature(struct homie_handle_s *handle,
                                            int node, int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->setpoint);
    homie_publish_property_value(handle, node, property, value);
}
void receive_controller_setpoint_temperature(struct homie_handle_s *handle,
                                             int node, int property,
                                             const char *data, int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->setpoint);
    ESP_LOGI(TAG,"new setpoint '%.2f' %d len\n", controller->setpoint, strlen(data));
    write_controller_params();
}
void receive_controller_hysteresis(struct homie_handle_s *handle, int node,
                                   int property, const char *data, int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->hysteresis);
    ESP_LOGI(TAG,"new hysteresis '%.2f'", controller->hysteresis);
    write_controller_params();
}
void update_controller_hysteresis(struct homie_handle_s *handle, int node,
                                  int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->hysteresis);
    homie_publish_property_value(handle, node, property, value);
}
void receive_controller_emergency(struct homie_handle_s *handle, int node,
                                  int property, const char *data, int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->emergency);
    ESP_LOGI(TAG,"new emergency value '%.2f'", controller->emergency);
    write_controller_params();
}
void update_controller_emergency(struct homie_handle_s *handle, int node,
                                 int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->emergency);
    homie_publish_property_value(handle, node, property, value);
}
void receive_controller_reaction_time(struct homie_handle_s *handle, int node,
                                      int property, const char *data,
                                      int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->reaction_time);
    ESP_LOGI(TAG,"new reaction time '%.2f'", controller->reaction_time);
    write_controller_params();
}
void update_controller_reaction_time(struct homie_handle_s *handle, int node,
                                     int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->reaction_time);
    homie_publish_property_value(handle, node, property, value);
}
void receive_controller_step_time(struct homie_handle_s *handle, int node,
                                  int property, const char *data, int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->step_time);
    ESP_LOGI(TAG,"new step time '%.2f'", controller->step_time);
    write_controller_params();
}
void update_controller_step_time(struct homie_handle_s *handle, int node,
                                 int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->step_time);
    homie_publish_property_value(handle, node, property, value);
}
void receive_controller_max_value(struct homie_handle_s *handle, int node,
                                  int property, const char *data, int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->max_value);
    ESP_LOGI(TAG,"new max value '%.2f'", controller->max_value);
    controller_reset(controller);
    write_controller_params();
}
void update_controller_max_value(struct homie_handle_s *handle, int node,
                                 int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->max_value);
    homie_publish_property_value(handle, node, property, value);
}
void update_controller_current_value(struct homie_handle_s *handle, int node,
                                     int property)
{
    char value[100];
    sprintf(value, "%.2f", controller->current_value);
    homie_publish_property_value(handle, node, property, value);
}
void update_controller_state(struct homie_handle_s *handle, int node,
                             int property)
{
    char value[100];
    sprintf(value, "%d", controller->state);
    homie_publish_property_value(handle, node, property, value);
}
