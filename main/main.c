#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <stdio.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "mqtt_client.h"

#include "homie.h"
#include "temperatureSensors.h"

static EventGroupHandle_t wifi_event_group;
static EventGroupHandle_t mqtt_event_group;

static esp_mqtt_client_handle_t mqtt_client = NULL;
const static int MQTT_CONNECTED_BIT = BIT0;

temperatureSensors_handle *temperatureSensors;

void update_temperatur_sensor(struct homie_handle_s *handle, int node,
                              int property);

homie_handle_t homie = {
    .deviceid = "heating-controller",
    .devicename = "Heating Controller",
    .update_interval =
        0, /* set to 0 to workaround openhab problem of taking device offline */
    .num_nodes = 4,
    .nodes =
        {
            {.id = "tempsensor1",
             .name = "Temperature Sensor 1",
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
                         .user_data = (void *)0,
                         .update_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "tempsensor2",
             .name = "Temperature Sensor 2",
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
                         .user_data = (void *)1,
                         .update_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "tempsensor3",
             .name = "Temperature Sensor 3",
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
                         .user_data = (void *)2,
                         .update_property_cbk = &update_temperatur_sensor,
                     },
                 }},
            {.id = "tempsensor4",
             .name = "Temperature Sensor 4",
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
                         .user_data = (void *)3,
                         .update_property_cbk = &update_temperatur_sensor,
                     },
                 }},
        },
    .uptime = 0,
};

int wifi_retry_count = 0;
const int WIFI_CONNECTED_BIT = BIT0;
static const char *TAG = "heating_controller";

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        // msg_id = esp_mqtt_client_subscribe(client,
        // CONFIG_EXAMPLE_SUBSCIBE_TOPIC, qos_test); ESP_LOGI(TAG, "sent
        // subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
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
        // esp_mqtt_client_handle_t client = event->client;
        // int msg_id = 0;
        // int actual_len = 0;
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        printf("ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n",
               event->msg_id, event->total_data_len, event->data_len,
               event->current_data_offset);
        /*
            if (event->topic) {
                actual_len = event->data_len;
                msg_id = event->msg_id;
            } else {
                actual_len += event->data_len;
                // check consisency with msg_id across multiple data events for
           single msg if (msg_id != event->msg_id) { ESP_LOGI(TAG, "Wrong msg_id
           in chunked message %d != %d", msg_id, event->msg_id); abort();
                }
            }
            memcpy(actual_data + event->current_data_offset, event->data,
           event->data_len); if (actual_len == event->total_data_len) { if (0 ==
           memcmp(actual_data, expected_data, expected_size)) { printf("OK!");
                    memset(actual_data, 0, expected_size);
                    actual_published ++;
                    if (actual_published == expected_published) {
                        printf("Correct pattern received exactly x times\n");
                        ESP_LOGI(TAG, "Test finished correctly!");
                    }
                } else {
                    printf("FAILED!");
                    abort();
                }
            }*/
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

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
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
    ESP_ERROR_CHECK(esp_wifi_start());

    uint8_t eth_mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(homie.mac, sizeof(homie.mac), "%02X:%02X:%02X:%02X:%02X:%02X",
             eth_mac[0], eth_mac[1], eth_mac[2], eth_mac[3], eth_mac[4],
             eth_mac[5]);

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", CONFIG_WIFI_SSID,
             CONFIG_WIFI_PASSWORD);
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

    ESP_LOGI(TAG, "connect to mqtt uri %s", uri);
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
    esp_mqtt_client_start(mqtt_client);
    ESP_LOGI(TAG, "Note free memory: %d bytes", esp_get_free_heap_size());
}

void update_temperatur_sensor(struct homie_handle_s *handle, int node,
                              int property)
{
    char value[100];
    float temp = temperatureSensors_read_temperature(
        temperatureSensors,
        (int)handle->nodes[node].properties[property].user_data);
    printf("temp %d: %.2f\n",
           (int)handle->nodes[node].properties[property].user_data, temp);
    sprintf(value, "%.2f", temp);
    homie_publish_property_value(handle, node, property, value);
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

    printf("wait for wifi connect");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true,
                        portMAX_DELAY);

    mqtt_app_start();

    printf("wait for mqtt connect");
    xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false, true,
                        portMAX_DELAY);

    temperatureSensors = temperatureSensors_init(&mqtt_client);

    homie.mqtt_client = mqtt_client;

    homie_init(&homie);

    /*
    gpio_pad_select_gpio(CONFIG_RELAIS1_GPIO);
    gpio_pad_select_gpio(CONFIG_RELAIS2_GPIO);
    gpio_set_direction(CONFIG_RELAIS1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_RELAIS2_GPIO, GPIO_MODE_OUTPUT);
     */
    for (int i = 100; i >= 0; i--)
    {
        printf("Restarting in %d seconds...\n", i * 80);

        // gpio_set_level(CONFIG_RELAIS1_GPIO, i % 2);
        // gpio_set_level(CONFIG_RELAIS2_GPIO, i % 2);

        temperatureSensors_trigger_read(temperatureSensors);

        homie.uptime += 8;

        homie_cycle(&homie);
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
