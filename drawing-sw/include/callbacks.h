#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <GLFW/glfw3.h>

// External declaration of colors to use it in callbacks.c
extern float colors[][3];

// Function prototypes for the callbacks
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#ifdef __linux__  // Define MQTT setup and polling only for Raspberry Pi/Linux
#include "MQTTClient.h"

#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "DrawingSoftwareClient"
#define TOPIC "drawing/coordinates"
#define QOS 1

MQTTClient client;
volatile float mqtt_x = 0.0f, mqtt_y = 0.0f;
int use_mqtt = 0;

void setup_mqtt();  // Setup MQTT client
void poll_mqtt_coordinates();  // Poll MQTT for red dot coordinates
#endif

#endif // CALLBACKS_H
