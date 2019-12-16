#include "homie.h"

void homie_init(homie_handle_t *handle)
{
    char buf_value[512] = {0};
    char buf_topic[512] = {0};

    sprintf(buf_topic, "homie/%s/$homie", handle->deviceid);
    sprintf(buf_value, "3.0.1");
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$name", handle->deviceid);
    sprintf(buf_value, "%s", handle->devicename);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$state", handle->deviceid);
    sprintf(buf_value, "ready");
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$localip", handle->deviceid);
    sprintf(buf_value, "%s", handle->ip);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$mac", handle->deviceid);
    sprintf(buf_value, "%s", handle->mac);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$fw/name", handle->deviceid);
    sprintf(buf_value, "%s", handle->devicename);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$fw/version", handle->deviceid);
    sprintf(buf_value, "0.1");
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$implementation", handle->deviceid);
    sprintf(buf_value, "esp32s");
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$stats", handle->deviceid);
    sprintf(buf_value, "uptime");
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$stats/interval", handle->deviceid);
    sprintf(buf_value, "%d", handle->update_interval);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    sprintf(buf_topic, "homie/%s/$nodes", handle->deviceid);
    memset(buf_value, 0, sizeof(buf_value));
    for (int n = 0; n < handle->num_nodes; ++n)
    {
        const homie_node_t *const node = &handle->nodes[n];
        strcat(buf_value, node->id);
        if (n < handle->num_nodes - 1)
        {
            strcat(buf_value, ",");
        }
    }
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);
    for (int n = 0; n < handle->num_nodes; ++n)
    {
        const homie_node_t *const node = &handle->nodes[n];
        sprintf(buf_topic, "homie/%s/%s/$name", handle->deviceid, node->id);
        sprintf(buf_value, "%s", node->name);
        esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                strlen(buf_value), 1, 1);
        sprintf(buf_topic, "homie/%s/%s/$type", handle->deviceid, node->id);
        sprintf(buf_value, "%s", node->type);
        esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                strlen(buf_value), 1, 1);
        sprintf(buf_topic, "homie/%s/%s/$properties", handle->deviceid,
                node->id);
        memset(buf_value, 0, sizeof(buf_value));
        for (int p = 0; p < node->num_properties; ++p)
        {
            const homie_node_property_t *const prop = &node->properties[p];
            strcat(buf_value, prop->id);
            if (p < node->num_properties - 1)
            {
                strcat(buf_value, ",");
            }
        }
        esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                strlen(buf_value), 1, 1);
        for (int p = 0; p < node->num_properties; ++p)
        {
            const homie_node_property_t *const prop = &node->properties[p];
            sprintf(buf_topic, "homie/%s/%s/%s/$name", handle->deviceid,
                    node->id, prop->id);
            sprintf(buf_value, "%s", prop->name);
            esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                    strlen(buf_value), 1, 1);
            sprintf(buf_topic, "homie/%s/%s/%s/$settable", handle->deviceid,
                    node->id, prop->id);
            sprintf(buf_value, "%s",
                    (prop->settable == HOMIE_TRUE) ? "true" : "false");
            esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                    strlen(buf_value), 1, 1);
            sprintf(buf_topic, "homie/%s/%s/%s/$unit", handle->deviceid,
                    node->id, prop->id);
            sprintf(buf_value, "%s", prop->unit);
            esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                    strlen(buf_value), 1, 1);
            sprintf(buf_topic, "homie/%s/%s/%s/$retained", handle->deviceid,
                    node->id, prop->id);
            sprintf(buf_value, "%s",
                    (prop->retained == HOMIE_TRUE) ? "true" : "false");
            esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                    strlen(buf_value), 1, 1);
            sprintf(buf_topic, "homie/%s/%s/%s/$datatype", handle->deviceid,
                    node->id, prop->id);
            switch (prop->datatype)
            {
            case HOMIE_INTEGER:
                sprintf(buf_value, "%s", "integer");
                break;
            case HOMIE_FLOAT:
                sprintf(buf_value, "%s", "float");
                break;
            case HOMIE_BOOL:
                sprintf(buf_value, "%s", "boolean");
                break;
            case HOMIE_STRING:
                sprintf(buf_value, "%s", "string");
                break;
            default:
                sprintf(buf_value, "%s", "error");
                break;
            };
            esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                                    strlen(buf_value), 1, 1);

            if (prop->settable == HOMIE_TRUE)
            {
            	sprintf(buf_topic, "homie/%s/%s/%s/set", handle->deviceid,
                                node->id, prop->id);
            	printf("subscribe to %s\n", buf_topic);
            	esp_mqtt_client_subscribe(handle->mqtt_client, buf_topic, 1);
            }
        }
    }
}

void homie_cycle(homie_handle_t *handle)
{
    char buf_value[255] = {0};
    char buf_topic[255] = {0};

    handle->uptime++;

    sprintf(buf_topic, "homie/%s/$stats/uptime", handle->deviceid);
    sprintf(buf_value, "%d", handle->uptime);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, buf_value,
                            strlen(buf_value), 1, 1);

    for (int n = 0; n < handle->num_nodes; ++n)
    {
    	const homie_node_t *const node = &handle->nodes[n];
    	for (int p = 0; p < node->num_properties; ++p)
        {
        	const homie_node_property_t *const prop = &node->properties[p];
            if (prop->read_property_cbk) prop->read_property_cbk(handle, n, p);
        }
    }
}

void homie_handle_mqtt_incoming_event(homie_handle_t *handle, esp_mqtt_event_handle_t event)
{
	char * pch = strtok (event->topic," /");
	if (pch == NULL || strcmp(pch, "homie") != 0) return;
	pch = strtok (NULL, "/");
	if (pch == NULL || strcmp(pch, handle->deviceid) != 0) return;
	pch = strtok (NULL, "/");
	if (pch == NULL) return;
	for (int n = 0; n < handle->num_nodes; ++n)
	{
		const homie_node_t *const node = &handle->nodes[n];
		if (strcmp(pch, node->id) == 0)
		{
			pch = strtok (NULL, "/");
			if (pch == NULL) return;
			for (int p = 0; p < node->num_properties; ++p)
			{
				const homie_node_property_t *const prop = &node->properties[p];
				if (strcmp(pch, prop->id) == 0)
				{
					if (prop->write_property_cbk) prop->write_property_cbk(handle, n, p, event->data, event->data_len);
				}
			}
		}
	}
}

void homie_publish_property_value(homie_handle_t *handle, int node,
                                  int property, const char *value)
{
    char buf_topic[255] = {0};

    sprintf(buf_topic, "homie/%s/%s/%s", handle->deviceid,
            handle->nodes[node].id,
            handle->nodes[node].properties[property].id);
    esp_mqtt_client_publish(handle->mqtt_client, buf_topic, value,
                            strlen(value), 1,
                            handle->nodes[node].properties[property].retained);
}
