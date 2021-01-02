#ifndef _HEATING_CONTROLLER_H_
#define HEATIME_CONTROLLER_H_

#include "homie.h"
#include "temperatureSensors.h"
#include "controller.h"

temperatureSensors_handle *temperatureSensors;
controller_handle_t *controller;

void update_temperatur_sensor(struct homie_handle_s *handle, int node,
                              int property);
void update_target_flow_temperature(struct homie_handle_s *handle, int node,
                                    int property);
void receive_target_flow_temperature(struct homie_handle_s *handle, int node,
                                     int property, const char *data);

void receive_controller_setpoint_temperature(struct homie_handle_s *handle,
                                             int node, int property,
                                             const char *data, int data_len);
void update_controller_setpoint_temperature(struct homie_handle_s *handle,
                                            int node, int property);
void receive_controller_hysteresis(struct homie_handle_s *handle, int node,
                                   int property, const char *data,
                                   int data_len);
void update_controller_hysteresis(struct homie_handle_s *handle, int node,
                                  int property);
void receive_controller_emergency(struct homie_handle_s *handle, int node,
                                  int property, const char *data, int data_len);
void update_controller_emergency(struct homie_handle_s *handle, int node,
                                 int property);
void receive_controller_reaction_time(struct homie_handle_s *handle, int node,
                                      int property, const char *data,
                                      int data_len);
void update_controller_reaction_time(struct homie_handle_s *handle, int node,
                                     int property);
void receive_controller_step_time(struct homie_handle_s *handle, int node,
                                  int property, const char *data, int data_len);
void update_controller_step_time(struct homie_handle_s *handle, int node,
                                 int property);
void receive_controller_max_value(struct homie_handle_s *handle, int node,
                                  int property, const char *data, int data_len);
void update_controller_max_value(struct homie_handle_s *handle, int node,
                                 int property);
void update_controller_current_value(struct homie_handle_s *handle, int node,
                                     int property);
void update_controller_state(struct homie_handle_s *handle, int node,
                             int property);

#endif