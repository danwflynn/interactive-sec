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

#define BROKER "tcp://localhost:1883"  // MQTT broker address
#define CLIENTID "CoordinateReceiver"  // Client ID
#define TOPIC "drawing/coordinates"    // MQTT topic
#define QOS 1                          // Quality of Service level
#define TIMEOUT 10000L                 // Timeout for waiting for messages

extern MQTTClient client;
//extern volatile float mqtt_x, mqtt_y;
extern int use_mqtt;
extern int allow_draw;

void setup_mqtt();  // Setup MQTT client
void* mqtt_thread(void* arg);  // Poll MQTT for red dot coordinates
#endif

#endif // CALLBACKS_H
