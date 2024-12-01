#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "drawing_main.h"
#include "drawing_io.h"
#include "callbacks.h"

#ifdef __linux__  // Include MQTT libraries and variables for Raspberry Pi
#include "MQTTClient.h"

MQTTClient client;
volatile float mqtt_x = 0.0f, mqtt_y = 0.0f;
int use_mqtt = 0;

int mqtt_message_arrived(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
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

void setup_mqtt() {
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    int rc = MQTTClient_create(&client, BROKER, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTClient_setCallbacks(client, NULL, NULL, mqtt_message_arrived, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    use_mqtt = 1;

    rc = MQTTClient_subscribe(client, TOPIC, QOS);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribed to topic: %s\n", TOPIC);
}

// MQTT polling in a separate thread
void* mqtt_thread(void* arg) {
    while (1) {
        MQTTClient_yield();
        usleep(100000);  // Sleep to prevent high CPU usage
    }
}

#endif // __linux__

float colors[][3] = {
    {1.0f, 0.0f, 0.0f},  // Red
    {0.0f, 1.0f, 0.0f},  // Green
    {0.0f, 0.0f, 1.0f},  // Blue
    {1.0f, 1.0f, 0.0f},  // Yellow
    {1.0f, 0.0f, 1.0f},  // Pink
    {0.5f, 0.0f, 0.5f},  // Purple
    {0.0f, 0.0f, 0.0f}   // Black
};

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && line_count < MAX_LINES) {
            is_drawing = 1;
            lines[line_count].point_count = 0;
            lines[line_count].point_capacity = INITIAL_POINT_CAPACITY;
            lines[line_count].points = (Point*)malloc(lines[line_count].point_capacity * sizeof(Point));
            if (lines[line_count].points == NULL) {
                fprintf(stderr, "Failed to allocate memory for points\n");
                return;
            }
            lines[line_count].color[0] = colors[current_color_index][0];
            lines[line_count].color[1] = colors[current_color_index][1];
            lines[line_count].color[2] = colors[current_color_index][2];
        } else if (action == GLFW_RELEASE) {
            is_drawing = 0;
            line_count++;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (is_drawing && lines[line_count].point_count < lines[line_count].point_capacity) {
        // Convert mouse coordinates to OpenGL coordinates
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x = (2.0f * xpos / width) - 1.0f;
        float y = 1.0f - (2.0f * ypos / height);

        lines[line_count].points[lines[line_count].point_count].x = x;
        lines[line_count].points[lines[line_count].point_count].y = y;
        lines[line_count].point_count++;
    } else if (is_drawing && lines[line_count].point_count >= lines[line_count].point_capacity) {
        // Increase capacity
        lines[line_count].point_capacity *= 2;
        lines[line_count].points = (Point*)realloc(lines[line_count].points, lines[line_count].point_capacity * sizeof(Point));
        if (lines[line_count].points == NULL) {
            fprintf(stderr, "Failed to reallocate memory for points\n");
            return;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        // Cycle to the next color
        current_color_index = (current_color_index + 1) % (sizeof(colors) / sizeof(colors[0]));
        printf("Switched to color: R=%.1f, G=%.1f, B=%.1f\n", colors[current_color_index][0], colors[current_color_index][1], colors[current_color_index][2]);
    } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        // Save lines to a file
        save_lines("drawing_data.bin");
        printf("Drawing saved to drawing_data.bin\n");
    } else if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        // Load lines from a file
        load_lines("drawing_data.bin");
        printf("Drawing loaded from drawing_data.bin\n");
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        // Draw at MQTT coordinates while spacebar is held down
        #ifdef __linux__
        if (use_mqtt) {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            float x = (2.0f * mqtt_x / width) - 1.0f;
            float y = 1.0f - (2.0f * mqtt_y / height);

            if (line_count < MAX_LINES) {
                lines[line_count].points[lines[line_count].point_count].x = x;
                lines[line_count].points[lines[line_count].point_count].y = y;
                lines[line_count].point_count++;
            }
        }
        #endif
    }
}
