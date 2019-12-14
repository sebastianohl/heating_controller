#ifndef TEMPERATURESENSORS_H_
#define TEMPERATURESENSORS_H_

#include "ds18b20.h"
#include "owb.h"
#include "owb_rmt.h"

#define TEMPERATURE_SENSORS_MAX_DEV (4)

typedef char device_rom_code_t[17];

typedef struct
{
    DS18B20_Info *info;
    OneWireBus_ROMCode device_rom_code;
    device_rom_code_t device_rom_code_str;
} temperatureSensors_sensor;

typedef struct
{
    owb_rmt_driver_info rmt_driver_info;
    OneWireBus *owb;
    uint8_t num_devices;
    temperatureSensors_sensor devices[TEMPERATURE_SENSORS_MAX_DEV];
} temperatureSensors_handle;

temperatureSensors_handle *temperatureSensors_init();
void temperatureSensors_trigger_read(temperatureSensors_handle *handle);
float temperatureSensors_read_temperature_idx(temperatureSensors_handle *handle,
                                          uint8_t sensor);
float temperatureSensors_read_temperature_str(temperatureSensors_handle *handle,
                                          device_rom_code_t sensor);

#endif
