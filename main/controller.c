/*
 * controller.c
 *
 *  Created on: Dec 14, 2019
 *      Author: ohli
 */

#include "controller.h"
#include <string.h>
#include <math.h>
#include "freertos/task.h"

void controller_keep_flow(controller_handle_t *handle)
{
    gpio_set_level(handle->open_flow_pin, 1);
    gpio_set_level(handle->close_flow_pin, 1);

    handle->state = CONTROLLER_KEEPFLOW;
    printf ("keep flow\n");
}

void controller_close_flow(controller_handle_t *handle)
{
	if (handle->current_value <= 0)
	{
		controller_keep_flow(handle);
		return;
	}
    gpio_set_level(handle->open_flow_pin, 1);
    gpio_set_level(handle->close_flow_pin, 0);

    handle->state = CONTROLLER_CLOSEFLOW;
    printf ("close flow\n");
}

void controller_open_flow(controller_handle_t *handle)
{
	if (handle->current_value >= handle->max_value)
	{
		controller_keep_flow(handle);
		return;
	}

	gpio_set_level(handle->open_flow_pin, 0);
    gpio_set_level(handle->close_flow_pin, 1);

    handle->state = CONTROLLER_OPENFLOW;
    printf ("open flow\n");
}

void controller_reset(controller_handle_t *handle)
{
	controller_close_flow(handle);

	handle->state = CONTROLLER_RESET;
	handle->current_value = handle->max_value;
	printf("controller reset\n");
}

controller_handle_t * controller_init(temperatureSensors_handle *temperatureSensors, uint8_t open_flow_pin,
		uint8_t close_flow_pin, device_rom_code_t const inflow_sensor, float hysteresis, float emergency,
		float max_value, float setpoint, float reaction_time, float step_time)
{
	controller_handle_t *handle = malloc(sizeof(controller_handle_t));
	memset(handle, 0, sizeof(controller_handle_t));

	handle->hysteresis = hysteresis;
	handle->emergency = emergency;
	handle->max_value = max_value;
	handle->setpoint = setpoint;
	handle->reaction_time = reaction_time;
	handle->step_time = step_time;

	handle->close_flow_pin = close_flow_pin;
	handle->open_flow_pin = open_flow_pin;
	handle->temperatureSensors = temperatureSensors;

	strcpy(handle->inflow_sensor, inflow_sensor);


    gpio_pad_select_gpio(handle->close_flow_pin);
    gpio_pad_select_gpio(handle->open_flow_pin);
    gpio_set_direction(handle->close_flow_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(handle->open_flow_pin, GPIO_MODE_OUTPUT);

    controller_reset(handle);

	return handle;
}

void controller_cycle(controller_handle_t *handle)
{
	controller_state_t lastState = handle->state;
	float currentTemp = temperatureSensors_read_temperature_str(handle->temperatureSensors, handle->inflow_sensor);
	printf ("current temp %f\n", currentTemp);
	printf ("current state %d\n", handle->state);
	printf ("current setpoint %.2f\n", handle->setpoint);

	if ((xTaskGetTickCount() * portTICK_PERIOD_MS) < handle->last_cycle) /* timer wrap ignore one cycle*/
	{
		printf("timer wrap\n");
		handle->last_cycle = xTaskGetTickCount() * portTICK_PERIOD_MS;
		handle->last_state_change = handle->last_cycle;
		return;
	}

	float timeSinceLast = (xTaskGetTickCount() * portTICK_PERIOD_MS)-handle->last_cycle;
	printf("time since last cycle %f\n", timeSinceLast);
	switch (handle->state)
	{
	case CONTROLLER_CLOSEFLOW: handle->current_value -= (float)timeSinceLast / 1000.0; break;
	case CONTROLLER_OPENFLOW: handle->current_value += (float)timeSinceLast / 1000.0; break;
	case CONTROLLER_RESET:
		{
			handle->current_value -= (float)timeSinceLast / 1000.0;
			break;
		}
	default: break;
	};

	handle->current_value = fmin(fmax(handle->current_value, 0), handle->max_value);

	printf("current value %f\n", handle->current_value);
	handle->last_cycle = xTaskGetTickCount() * portTICK_PERIOD_MS;

	printf("last state change %d\n", handle->last_cycle-handle->last_state_change);

	if (currentTemp > handle->emergency)
	{
		controller_reset(handle);
		handle->last_state_change = handle->last_cycle;
		return;
	}

	if (handle->state == CONTROLLER_RESET)
	{
		if (handle->last_cycle-handle->last_state_change > handle->max_value * 1000 * 1.2)
		{
			handle->current_value = 0;
			controller_keep_flow(handle);
		} else {
			controller_close_flow(handle);
			handle->state = CONTROLLER_RESET;
			return;
		}
	}

	if (handle->last_cycle-handle->last_state_change > handle->reaction_time*1000)
	{
		printf("error value %f <=> %f\n", currentTemp-handle->setpoint, handle->hysteresis);
		float error_value = currentTemp-handle->setpoint;
		if (fabs(error_value) > handle->hysteresis)
		{
			if (error_value > 0) /* too hot */
			{
				controller_close_flow(handle);
			} else { /* too cold */
				controller_open_flow(handle);
			}
		} else { /* within range */
			controller_keep_flow(handle);
		}
	} else {
		if (handle->last_cycle-handle->last_state_change > handle->step_time*1000)
		{
			switch (handle->state)
			{
				case CONTROLLER_CLOSEFLOW: controller_keep_flow(handle); break;
				case CONTROLLER_OPENFLOW: controller_keep_flow(handle); break;
				default: break;
			};
		}
	}

	if (handle->state != lastState)
	{
		handle->last_state_change = handle->last_cycle;
	}
}
