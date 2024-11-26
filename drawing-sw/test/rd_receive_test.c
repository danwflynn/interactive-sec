#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqtt/MQTTClient.h>

#define BROKER "tcp://localhost:1883"  // MQTT broker address
#define CLIENTID "CoordinateReceiver"  // Client ID
#define TOPIC "drawing/coordinates"    // MQTT topic
#define QOS 1                          // Quality of Service level
#define TIMEOUT 10000L                 // Timeout for waiting for messages

void messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    if (message->payloadlen) {
        // Print the received coordinates
        printf("Received message on topic %s: %s\n", topicName, (char *)message->payload);
    } else {
        printf("Received empty message\n");
    }
    // Free the message payload after it's processed
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
}

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message message = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    // Initialize the MQTT client
    int rc = MQTTClient_create(&client, BROKER, CLIENTID,
                               MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // Set the callback function for message arrival
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    // Set connection options
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Connect to the broker
    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // Subscribe to the topic
    rc = MQTTClient_subscribe(client, TOPIC, QOS);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribed to topic: %s\n", TOPIC);

    // Start the message loop
    while (1) {
        // The loop will handle incoming messages in the background
        usleep(100000);  // Sleep for a short time to avoid busy-waiting
    }

    // Clean up and disconnect from the broker
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return 0;
}
