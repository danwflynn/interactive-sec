#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drawing_main.h"
#include "drawing_io.h"
#include "callbacks.h"

#ifdef __linux__  // Include MQTT libraries and variables for Raspberry Pi
#include "MQTTClient.h"

void setup_mqtt() {
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to connect to MQTT broker.\n");
        use_mqtt = 0;
        return;
    }
    use_mqtt = 1;
    printf("Connected to MQTT broker.\n");

    MQTTClient_subscribe(client, TOPIC, QOS);
}

void mqtt_message_arrived(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    char* payload = (char*)message->payload;
    sscanf(payload, "%f %f", &mqtt_x, &mqtt_y);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
}

void poll_mqtt_coordinates() {
    MQTTClient_setCallbacks(client, NULL, NULL, mqtt_message_arrived, NULL);
    MQTTClient_yield();
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
