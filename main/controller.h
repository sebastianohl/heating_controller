/*
 * controller.h
 *
 *  Created on: Dec 14, 2019
 *      Author: ohli
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "temperatureSensors.h"

typedef enum
{
	CONTROLLER_OPENFLOW = 0,
	CONTROLLER_CLOSEFLOW = 1,
	CONTROLLER_KEEPFLOW = 2,
	CONTROLLER_RESET = 3,
	CONTROLLER_WAIT = 4,
	CONTROLLER_MAX
} controller_state_t;

struct controller_handle_s
{
	float setpoint; /* [°C] */
	float hysteresis; /* [°C] */
	float emergency; /* [°C] */
	float reaction_time; /* [s] */
	float step_time; /* [s] */
	float current_value; /* [s] */
	float max_value; /* [s] */

	uint32_t last_cycle; /* [ms] */
	uint32_t last_state_change; /* [ms] */
	controller_state_t state;

	device_rom_code_t inflow_sensor;

	uint8_t open_flow_pin;
	uint8_t close_flow_pin;

	temperatureSensors_handle *temperatureSensors;
};
typedef struct controller_handle_s controller_handle_t;

controller_handle_t * controller_init(temperatureSensors_handle *temperatureSensors, uint8_t open_flow_pin,
		uint8_t close_flow_pin,  device_rom_code_t const inflow_sensor, float hysteresis, float emergency,
		float max_value, float setpoint, float reaction_time, float step_time);
void controller_cycle(controller_handle_t *handle);
void controller_reset(controller_handle_t *handle);

#endif /* CONTROLLER_H_ */
