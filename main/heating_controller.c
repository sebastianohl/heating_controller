#include "heating_controller.h"

static const char *TAG = "heating_controller";

void update_temperatur_sensor(struct homie_handle_s *handle, int node,
                              int property)
{
    char value[100];
    float temp = temperatureSensors_read_temperature_str(
        temperatureSensors, *((device_rom_code_t *)handle->nodes[node]
                                  .properties[property]
                                  .user_data));
    printf("temp %d: %.2f\n",
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
    printf("new setpoint '%.2f' %d len\n", controller->setpoint, strlen(data));
}
void receive_controller_hysteresis(struct homie_handle_s *handle, int node,
                                   int property, const char *data, int data_len)
{
    char tmp[100] = {0};
    strncpy(tmp, data, data_len);
    sscanf(tmp, "%f", &controller->hysteresis);
    printf("new hysteresis '%.2f'\n", controller->hysteresis);
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
    printf("new emergency value '%.2f'\n", controller->emergency);
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
    printf("new reaction time '%.2f'\n", controller->reaction_time);
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
    printf("new step time '%.2f'\n", controller->step_time);
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
    printf("new max value '%.2f'\n", controller->max_value);
    controller_reset(controller);
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
