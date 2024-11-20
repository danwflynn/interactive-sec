#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#define ADDRESS     "tcp://broker.hivemq.com:1883"  // Replace with your broker address
#define CLIENTID    "TestClient"
#define TOPIC       "test/topic"
#define QOS         1
#define TIMEOUT     10000L

int messageArrivedCallback(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    // Null-terminate topicName for safety
    topicName[topicLen] = '\0';

    printf("Message arrived\n");
    printf("Topic: %s\n", topicName);

    // Ensure message payload is properly handled
    if (message != NULL && message->payload != NULL) {
        printf("Message: %.*s\n", message->payloadlen, (char*)message->payload);
    } else {
        printf("Received empty message payload\n");
    }

    // Do not free message or topicName as they are managed by the MQTTClient
    // MQTTClient_freeMessage(&message);   // This is not necessary
    // MQTTClient_free(topicName);         // This is not necessary
    return 1;  // Return 1 to indicate message was processed successfully
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Create MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
    // Set the callbacks, passing the messageArrivedCallback function for message arrival
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrivedCallback, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Attempt to connect to the broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return EXIT_FAILURE;
    }

    printf("Connected to broker\n");

    // Subscribe to the topic
    MQTTClient_subscribe(client, TOPIC, QOS);
    printf("Subscribed to topic %s\n", TOPIC);

    // Keep the client running to receive messages
    printf("Press Enter to exit...\n");
    getchar();

    // Disconnect and clean up
    MQTTClient_disconnect(client, TIMEOUT);
    MQTTClient_destroy(&client);

    return EXIT_SUCCESS;
}

