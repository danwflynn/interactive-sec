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
#include <microhttpd.h>

#define BROKER "tcp://localhost:1883"  // MQTT broker address
#define CLIENTID "CoordinateReceiver"  // Client ID
#define TOPIC "drawing/coordinates"    // MQTT topic
#define QOS 1                          // Quality of Service level
#define TIMEOUT 10000L                 // Timeout for waiting for messages

#define PORT 8080

extern MQTTClient client;
extern int use_mqtt;
extern int allow_draw;

void setup_mqtt();  // Setup MQTT client
void* mqtt_thread(void* arg);  // Poll MQTT for red dot coordinates

static enum MHD_Result handle_request(void *cls, struct MHD_Connection *connection, 
                                      const char *url, const char *method, 
                                      const char *version, const char *upload_data, 
                                      size_t *upload_data_size, void **con_cls);
void* http_thread(void* arg);

#endif

#endif // CALLBACKS_H
