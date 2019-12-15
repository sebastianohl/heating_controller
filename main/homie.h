#ifndef HOMIE_H_
#define HOMIE_H_

#include "mqtt_client.h"

#define HOMIE_MAX_STRLEN (50)
#define HOMIE_MAX_NODES (5)
#define HOMIE_MAX_NODES_PROPERTIES (10)

#define HOMIE_TRUE (1)
#define HOMIE_FALSE (0)

struct homie_handle_s;

typedef enum
{
    HOMIE_INTEGER,
    HOMIE_FLOAT,
    HOMIE_BOOL,
    HOMIE_STRING,
    HOMIE_DATATYPE_MAX
} homie_node_property_type_t;

struct homie_node_property_s
{
    char id[HOMIE_MAX_STRLEN];
    char name[HOMIE_MAX_STRLEN];
    bool settable;
    bool retained;
    char unit[HOMIE_MAX_STRLEN];
    homie_node_property_type_t datatype;
    void *user_data;
    void (*read_property_cbk)(struct homie_handle_s *handle, int node,
                                int property);
    void (*write_property_cbk)(struct homie_handle_s *handle, int node,
                                int property, const char *data);
};
typedef struct homie_node_property_s homie_node_property_t;

struct homie_node_s
{
    char id[HOMIE_MAX_STRLEN];
    char name[HOMIE_MAX_STRLEN];
    char type[HOMIE_MAX_STRLEN];
    uint8_t num_properties;
    struct homie_node_property_s properties[HOMIE_MAX_NODES_PROPERTIES];
};
typedef struct homie_node_s homie_node_t;

struct homie_handle_s
{
    char deviceid[HOMIE_MAX_STRLEN];
    char devicename[HOMIE_MAX_STRLEN];
    char ip[HOMIE_MAX_STRLEN];
    char mac[HOMIE_MAX_STRLEN];
    uint32_t update_interval;
    uint8_t num_nodes;
    struct homie_node_s nodes[HOMIE_MAX_NODES];
    uint32_t uptime;
    esp_mqtt_client_handle_t mqtt_client;
};
typedef struct homie_handle_s homie_handle_t;

void homie_init(homie_handle_t *handle);
void homie_cycle(homie_handle_t *handle);
void homie_handle_mqtt_incoming_event(homie_handle_t *handle, esp_mqtt_event_handle_t event);
void homie_publish_property_value(homie_handle_t *handle, int node,
                                  int property, const char *value);

#endif
