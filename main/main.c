#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp32/rom/ets_sys.h"
#include "sdkconfig.h"
#include <stdio.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_ota_ops.h"

#include "ota.h"
#include "mqtt_client.h"

#include "heating_controller.h"

#define STR_(X) #X
#define STR(X) STR_(X)

static EventGroupHandle_t wifi_event_group;
static EventGroupHandle_t mqtt_event_group;

static esp_mqtt_client_handle_t mqtt_client = NULL;
static int OTA_ongoing = HOMIE_FALSE;

const static int MQTT_NEW_CONNECT_BIT = BIT0;
const static int MQTT_CONNECTED_BIT = BIT1;

void start_ota(struct homie_handle_s *handle, int node,
                                int property, const char *data, int data_len);
const device_rom_code_t inflow_floor_heating_sensor =
    CONFIG_inflow_floor_heating_sensor;
const device_rom_code_t returnflow_floor_heating_sensor =
    CONFIG_returnflow_floor_heating_sensor;
const device_rom_code_t inflow_heater_sensor = CONFIG_inflow_heater_sensor;
const device_rom_code_t returnflow_heater_sensor =
    CONFIG_returnflow_heater_sensor;

homie_handle_t homie = {
    .deviceid = "heating-controller",
    .devicename = "Heating Controller",
    .firmware = GIT_URL,
    .firmware_version = GIT_BRANCH " " GIT_COMMIT_HASH,
    .update_interval =
        0, /* set to 0 to workaround openhab problem of taking device offline */
    .num_nodes = 6,
    .nodes =
        {
            {.id = "system",
             .name = "System",
             .type = "esp32",
             .num_properties = 1,
             .properties =
                 {
                       {
                           .id = "update",
                           .name = "update",
                           .settable = HOMIE_TRUE,
                           .retained = HOMIE_TRUE,
                           .unit = " ",
                           .datatype = HOMIE_BOOL,
                           .read_property_cbk = NULL,
                           .write_property_cbk = &start_ota,
                           .user_data = NULL,
                       },
                 }},
            {.id = "inflow-floor-heating",
             .name = "Inflow Floor Heating",
             .type = "ds18b20",
             .num_properties = 1,
             .properties =
                 {
                     {
                         .id = "temperature",
                         .name = "Temperature",
                         .settable = HOMIE_FALSE,
                         .retained = HOMIE_TRUE,
                         .unit = "°C",
                         .datatype = HOMIE_FLOAT,
                         .user_data = (void *)&inflow_floor_heating_sensor,
                         .read_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "returnflow-floor-heating",
             .name = "Return flow floor heating",
             .type = "ds18b20",
             .num_properties = 1,
             .properties =
                 {
                     {
                         .id = "temperature",
                         .name = "Temperature",
                         .settable = HOMIE_FALSE,
                         .retained = HOMIE_TRUE,
                         .unit = "°C",
                         .datatype = HOMIE_FLOAT,
                         .user_data = (void *)&returnflow_floor_heating_sensor,
                         .read_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "inflow-heater",
             .name = "Inflow Heater",
             .type = "ds18b20",
             .num_properties = 1,
             .properties =
                 {
                     {
                         .id = "temperature",
                         .name = "Temperature",
                         .settable = HOMIE_FALSE,
                         .retained = HOMIE_TRUE,
                         .unit = "°C",
                         .datatype = HOMIE_FLOAT,
                         .user_data = (void *)inflow_heater_sensor,
                         .read_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "returnflow-heater",
             .name = "Return flow heater",
             .type = "ds18b20",
             .num_properties = 1,
             .properties =
                 {
                     {
                         .id = "temperature",
                         .name = "Temperature",
                         .settable = HOMIE_FALSE,
                         .retained = HOMIE_TRUE,
                         .unit = "°C",
                         .datatype = HOMIE_FLOAT,
                         .user_data = (void *)returnflow_heater_sensor,
                         .read_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "flow-temperature-controller",
             .name = "Flow Temperature Controller",
             .type = "3-point-output",
             .num_properties = 8,
             .properties =
                 {{
                      .id = "setpoint-temperature",
                      .name = "Set Point Temperature",
                      .settable = HOMIE_TRUE,
                      .retained = HOMIE_TRUE,
                      .unit = "°C",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk =
                          &update_controller_setpoint_temperature,
                      .write_property_cbk =
                          &receive_controller_setpoint_temperature,
                  },
                  {
                      .id = "hystersis",
                      .name = "Hysteresis",
                      .settable = HOMIE_TRUE,
                      .retained = HOMIE_TRUE,
                      .unit = "°C",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_hysteresis,
                      .write_property_cbk = &receive_controller_hysteresis,
                  },
                  {
                      .id = "emergency",
                      .name = "Emergency Temperature",
                      .settable = HOMIE_TRUE,
                      .retained = HOMIE_TRUE,
                      .unit = "°C",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_emergency,
                      .write_property_cbk = &receive_controller_emergency,
                  },
                  {
                      .id = "reaction-time",
                      .name = "Reaction Time",
                      .settable = HOMIE_TRUE,
                      .retained = HOMIE_TRUE,
                      .unit = "s",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_reaction_time,
                      .write_property_cbk = &receive_controller_reaction_time,
                  },
                  {
                      .id = "step-time",
                      .name = "Step Time",
                      .settable = HOMIE_TRUE,
                      .retained = HOMIE_TRUE,
                      .unit = "s",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_step_time,
                      .write_property_cbk = &receive_controller_step_time,
                  },
                  {
                      .id = "current-value",
                      .name = "Current Value",
                      .settable = HOMIE_FALSE,
                      .retained = HOMIE_TRUE,
                      .unit = "s",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_current_value,
                  },
                  {
                      .id = "state",
                      .name = "Current State",
                      .settable = HOMIE_FALSE,
                      .retained = HOMIE_TRUE,
                      .unit = "enum",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_state,
                  },
                  {
                      .id = "max-value",
                      .name = "Max Value",
                      .settable = HOMIE_TRUE,
                      .retained = HOMIE_TRUE,
                      .unit = "s",
                      .datatype = HOMIE_FLOAT,
                      .read_property_cbk = &update_controller_max_value,
                      .write_property_cbk = &receive_controller_max_value,
                  }}},
        },
    .uptime = 0,
};

int wifi_retry_count = 0;
const int WIFI_CONNECTED_BIT = BIT0;
static const char *TAG = "heating_controller";

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        xEventGroupSetBits(mqtt_event_group, MQTT_NEW_CONNECT_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        xEventGroupClearBits(mqtt_event_group, MQTT_NEW_CONNECT_BIT);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA='%.*s'\r\n", event->data_len, event->data);
        printf("ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n",
               event->msg_id, event->total_data_len, event->data_len,
               event->current_data_offset);

        homie_handle_mqtt_incoming_event(&homie, event);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_app_start(void)
{
    char uri[256];
    sprintf(uri, "mqtt://%s:%s@%s:%d", CONFIG_MQTT_USER, CONFIG_MQTT_PASSWORD,
            CONFIG_MQTT_SERVER, CONFIG_MQTT_PORT);
    mqtt_event_group = xEventGroupCreate();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        .uri = uri,
    };
    xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);

    ESP_LOGI(TAG, "connect to mqtt uri %s", uri);
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_LOGI(TAG, "Note free memory: %d bytes", esp_get_free_heap_size());
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
            esp_err_t err;
            char hostname[33] = {0};
            snprintf(hostname, 33, "heating-control");
            ESP_LOGI(TAG, "set hostname to %s", hostname);
            if ((err = tcpip_adapter_set_hostname(WIFI_IF_STA, hostname)) != ESP_OK)
            {
                ESP_LOGE(TAG, "set hostname failed: %s", esp_err_to_name(err));
            }
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_mqtt_client_stop(mqtt_client);

        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_retry_count++;
        ESP_LOGI(TAG, "retry to connect to the AP");
        if (wifi_retry_count > 10)
        {
            ESP_LOGI(TAG, "reboot to many tries");
            fflush(stdout);
            esp_restart();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        snprintf(homie.ip, sizeof(homie.ip), "%d.%d.%d.%d",
                 IP2STR(&event->ip_info.ip));
        wifi_retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        esp_mqtt_client_start(mqtt_client);
    }
}

void connect_to_wifi()
{
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {.ssid = CONFIG_WIFI_SSID, .password = CONFIG_WIFI_PASSWORD},
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    uint8_t eth_mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(homie.mac, sizeof(homie.mac), "%02X:%02X:%02X:%02X:%02X:%02X",
             eth_mac[0], eth_mac[1], eth_mac[2], eth_mac[3], eth_mac[4],
             eth_mac[5]);

    ESP_LOGI(TAG, "homie device id %s", homie.deviceid);
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", CONFIG_WIFI_SSID,
             CONFIG_WIFI_PASSWORD);

    ESP_ERROR_CHECK(esp_wifi_start());
}

void start_ota(struct homie_handle_s *handle, int node, int property, const char *data, int data_len)
{
    if (data_len > 0)
    {
        char buf_topic[255] = {0};
        char url[255] = {0};

        snprintf(url, sizeof(url), "%.*s", data_len, data);
        
        OTA_ongoing = HOMIE_TRUE;
        ESP_LOGI(TAG, "get OTA update from %s", url);

        esp_err_t ret = execute_ota(url);

        // reset update topic to prevent inifinte update loop
        sprintf(buf_topic, "homie/%s/%s/update/set", homie.deviceid, homie.nodes[0].id);
        esp_mqtt_client_publish(homie.mqtt_client, buf_topic, "",
                            0, 1, HOMIE_TRUE);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "reset to start new image");
            fflush(stdout);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart();
        }
        OTA_ongoing = HOMIE_FALSE;
    }
}

void app_main(void)
{
    printf("starting....\n");
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    connect_to_wifi();

    mqtt_app_start();

    temperatureSensors = temperatureSensors_init(&mqtt_client);

    controller =
        controller_init(temperatureSensors, CONFIG_RELAIS1_GPIO,
                        CONFIG_RELAIS2_GPIO, inflow_floor_heating_sensor,
                        CONFIG_controller_hysteresis,      /* hysteresis */
                        CONFIG_controller_emergency_value, /* emergency */
                        CONFIG_controller_max_value,       /* max value */
                        CONFIG_controller_set_point,       /* set point */
                        CONFIG_controller_reaction_time,   /* reaction time */
                        CONFIG_controller_step_time        /* step time */
        );

    const esp_partition_t* current_partition = esp_ota_get_running_partition();
    strncpy(homie.firmware, current_partition->label, sizeof(homie.firmware));

    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    strncpy(homie.firmware_version, app_desc->version, sizeof(homie.firmware_version));
    ESP_LOGI(TAG, "running partition %s version %s", current_partition->label, app_desc->version);
    homie.mqtt_client = mqtt_client;

    for (int i = 24 * 60 * 60 / 5; i >= 0; i--)
    {
        EventBits_t uxBits;
        ESP_LOGI(TAG, "Restarting in %d seconds...\n", i * 5);

        homie.uptime += 5;

        ESP_LOGI(TAG, "test for mqtt new connect");
        uxBits = xEventGroupWaitBits(mqtt_event_group, MQTT_NEW_CONNECT_BIT, true,
                                     false, 1);
        if ((uxBits & MQTT_NEW_CONNECT_BIT) != 0)
        {
            ESP_LOGI(TAG, "homie init");
            homie_init(&homie);
            ESP_LOGI(TAG, "homie init done");
        }
        
        temperatureSensors_trigger_read(temperatureSensors);

        uxBits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false,
                                     false, 1);
        if ((uxBits & MQTT_CONNECTED_BIT) != 0)
        {
            ESP_LOGI(TAG, "homie cycle");
            homie_cycle(&homie);
        }

        controller_cycle(controller);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
