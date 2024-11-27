#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>

#define BROKER "tcp://localhost:1883"  // MQTT broker address
#define CLIENTID "CoordinateReceiver"  // Client ID
#define TOPIC "drawing/coordinates"    // MQTT topic
#define QOS 1                          // Quality of Service level
#define TIMEOUT 10000L                 // Timeout for waiting for messages

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    if (message->payloadlen) {
        char *payload = (char *)malloc(message->payloadlen + 1);
        if (!payload) {
            printf("Failed to allocate memory for payload\n");
            MQTTClient_freeMessage(&message);
            MQTTClient_free(topicName);
            return 1;
        }

        memcpy(payload, message->payload, message->payloadlen);
        payload[message->payloadlen] = '\0';

        printf("Received message on topic %s: %s\n", topicName, payload);

        free(payload);
    } else {
        printf("Received empty message on topic %s\n", topicName);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1; // Indicate successful processing of the message
}

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    int rc = MQTTClient_create(&client, BROKER, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTClient_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    rc = MQTTClient_subscribe(client, TOPIC, QOS);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribed to topic: %s\n", TOPIC);

    while (1) {
        usleep(100000);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return 0;
}
