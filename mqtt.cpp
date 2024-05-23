#include <Arduino.h>
#include "mqtt.h"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    MqttPetoiClient *ptr = (MqttPetoiClient *)handler_args;

    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        Serial.println("MQTT_EVENT_CONNECTED.");
        break;
    case MQTT_EVENT_DISCONNECTED:
        Serial.println("MQTT_EVENT_DISCONNECTED.");
        break;
    case MQTT_EVENT_DATA:
    {
        if (!ptr)
        {
            Serial.println("MQTT_EVENT_DATA, but not client instance");
            break;
        }
        std::string topic(event->topic, event->topic_len);
        MESSAGE_HANDLER handler = ptr->find_message_handler(topic);
        if (handler)
        {
            handler(event->data, event->data_len);
        }
        break;
    }
    case MQTT_EVENT_ERROR:
        Serial.println("MQTT_EVENT_ERROR");
        break;
    }
}

void MqttPetoiClient::connect()
{
    if (this->_client != nullptr)
    {
        Serial.println("MQTT broker already connected.");
        return;
    }

    Serial.println("Connecting MQTT broker");

    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = this->_brokerURI.c_str(),
        .port = this->_brokerPort,
    };

    this->_client = esp_mqtt_client_init(&mqtt_cfg);

    if (this->_client == nullptr) {
        return;
    }

    esp_mqtt_client_register_event(this->_client, MQTT_EVENT_ANY, mqtt_event_handler, this);
    esp_mqtt_client_start(this->_client);
}

void MqttPetoiClient::publish(const char *topic, void *data, size_t size)
{
    int msg_id = esp_mqtt_client_publish(this->_client, topic, (const char *)data, size, 0, 0);
    if (msg_id < 0)
    {
        Serial.println("Failed to publish message.");
    }
    else
    {
        Serial.print("Publish successful, msg_id=");
        Serial.println(msg_id);
    }
}

void MqttPetoiClient::subscribe(const char *topic, MESSAGE_HANDLER handler)
{
    this->_handlers[topic] = handler;
}

MESSAGE_HANDLER MqttPetoiClient::find_message_handler(const std::string &topic)
{
    auto it = this->_handlers.find(topic);

    if (it == this->_handlers.end())
    {
        return nullptr;
    }

    return it->second;
}