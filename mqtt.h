#ifndef _MQTT_H
#define _MQTT_H

#include <string>
#include <unordered_map>
#include <mqtt_client.h>

typedef void (*MESSAGE_HANDLER)(void *message, size_t size);

class MqttPetoiClient {
public:
    MqttPetoiClient(const char *brokerURI, int brokerPort=1883):
        _brokerURI(brokerURI),
        _brokerPort(brokerPort),
        _client(nullptr)
    {
    }
    void connect();

    void publish(const char *topic, void *data, size_t size);

    void subscribe(const char *topic, MESSAGE_HANDLER MESSAGE_HANDLER);

    MESSAGE_HANDLER find_message_handler(const std::string &topic);

private:
    std::unordered_map<std::string, MESSAGE_HANDLER> _handlers;
    esp_mqtt_client_handle_t _client;
    std::string _brokerURI;
    int _brokerPort;
};

#endif