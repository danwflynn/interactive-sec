#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#define ADDRESS     "tcp://broker.hivemq.com:1883"  // Replace with your broker address
#define CLIENTID    "TestClient"
#define TOPIC       "test/topic"
#define QOS         1
#define TIMEOUT     10000L

void messageArrivedCallback(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    printf("Message arrived\n");
    printf("Topic: %s\n", topicName);
    printf("Message: %.*s\n", message->payloadlen, (char*)message->payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrivedCallback, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return EXIT_FAILURE;
    }

    printf("Connected to broker\n");

    MQTTClient_subscribe(client, TOPIC, QOS);
    printf("Subscribed to topic %s\n", TOPIC);

    // Keep the client running to receive messages
    printf("Press Enter to exit...\n");
    getchar();

    MQTTClient_disconnect(client, TIMEOUT);
    MQTTClient_destroy(&client);

    return EXIT_SUCCESS;
}
