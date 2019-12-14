#include "temperatureSensors.h"

#include <string.h>

temperatureSensors_handle *temperatureSensors_init()
{
    temperatureSensors_handle *handle =
        malloc(sizeof(temperatureSensors_handle));
    memset(handle, 0, sizeof(temperatureSensors_handle));

    handle->owb =
        owb_rmt_initialize(&handle->rmt_driver_info, CONFIG_ONE_WIRE_GPIO,
                           RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(handle->owb, true); // enable CRC check for ROM code

    // Find all connected devices
    printf("Find devices:\n");
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    owb_search_first(handle->owb, &search_state, &found);
    while (found)
    {
        owb_string_from_rom_code(
            search_state.rom_code,
            handle->devices[handle->num_devices].device_rom_code_str,
            sizeof(handle->devices[handle->num_devices].device_rom_code_str));
        printf("  %d : %s\n", handle->num_devices,
               handle->devices[handle->num_devices].device_rom_code_str);
        handle->devices[handle->num_devices].device_rom_code =
            search_state.rom_code;
        ++handle->num_devices;
        owb_search_next(handle->owb, &search_state, &found);
    }

    printf("Found %d devices\n", handle->num_devices);

    // Create DS18B20 devices on the 1-Wire bus
    for (int i = 0; i < handle->num_devices; ++i)
    {
        DS18B20_Info *ds18b20_info = ds18b20_malloc(); // heap allocation
        handle->devices[i].info = ds18b20_info;

        ds18b20_init(ds18b20_info, handle->owb,
                     handle->devices[i]
                         .device_rom_code); // associate with bus and device

        ds18b20_use_crc(ds18b20_info,
                        true); // enable CRC check for temperature readings
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);
    }

    return handle;
}

void temperatureSensors_trigger_read(temperatureSensors_handle *handle)
{
    assert(handle->num_devices > 0);
    ds18b20_convert_all(handle->owb);
    ds18b20_wait_for_conversion(handle->devices[0].info);
}

float temperatureSensors_read_temperature_idx(temperatureSensors_handle *handle,
                                          uint8_t sensor)
{
    assert(sensor < handle->num_devices);
    float temp = 42;
    ds18b20_read_temp(handle->devices[sensor].info, &temp);

    return temp;
}

float temperatureSensors_read_temperature_str(temperatureSensors_handle *handle,
                                          device_rom_code_t sensor)
{
	for (int i = 0; i < handle->num_devices; ++i)
	{
		if (strcmp(handle->devices[i].device_rom_code_str, sensor) == 0)
		{
			return temperatureSensors_read_temperature_idx(handle, i);
		}
	}
	return 666;
}
