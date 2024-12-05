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
#include <microhttpd.h>

MQTTClient client;
int use_mqtt = 0;
int allow_draw = 0;

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
            
            float xpos, ypos;
            int num_parsed = sscanf(payload, "%f %f", &xpos, &ypos);  // Parses two floating-point numbers
	    xpos *= 2;
	    ypos *= 2;
            
        if (num_parsed == 2) {
            if (allow_draw && lines[line_count].point_count < lines[line_count].point_capacity) {
                // Convert mouse coordinates to OpenGL coordinates
                int width = WINDOW_WIDTH;
                int height = WINDOW_HEIGHT;
                float x = (2.0f * xpos / width) - 1.0f;
                float y = 1.0f - (2.0f * ypos / height);

                lines[line_count].points[lines[line_count].point_count].x = x;
                lines[line_count].points[lines[line_count].point_count].y = y;
                lines[line_count].point_count++;
            } else if (allow_draw && lines[line_count].point_count >= lines[line_count].point_capacity) {
                // Increase capacity
                lines[line_count].point_capacity *= 2;
                lines[line_count].points = (Point*)realloc(lines[line_count].points, lines[line_count].point_capacity * sizeof(Point));
                if (lines[line_count].points == NULL) {
                    fprintf(stderr, "Failed to reallocate memory for points\n");
                    return 1;
                }
            }
        } else {
            printf("Failed to parse floating point numbers from payload\n");
        }

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

static enum MHD_Result handle_request(void *cls, struct MHD_Connection *connection, 
                                      const char *url, const char *method, 
                                      const char *version, const char *upload_data, 
                                      size_t *upload_data_size, void **con_cls) {
    static int dummy;
    const char *response_str = "Message received!";
    struct MHD_Response *response;

    // Only handle POST requests
    if (strcmp(method, "POST") != 0) {
        return MHD_NO; // Reject non-POST requests
    }

    if (*con_cls == NULL) {
        *con_cls = &dummy;
        return MHD_YES;
    }

    if (*upload_data_size > 0) {
        printf("Received message: %s\n", upload_data);

        //Button logic
        if (strcmp(upload_data, "{\"button\": 1,\"state\": \"pressed\"}") == 0 && allow_draw == 0) {
            //start drawing
            allow_draw = 1; // Start drawing when button 1 is pressed
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
        } else if (strcmp(upload_data, "{\"button\": 1,\"state\": \"released\"}") == 0 && allow_draw == 1) {
            //finish drawing
            allow_draw = 0; // Stop drawing when button 1 is released
            line_count++;
        } else if (strcmp(upload_data, "{\"button\": 2,\"state\": \"pressed\"}") == 0) {
            //change colors
            current_color_index = (current_color_index + 1) % (sizeof(colors) / sizeof(colors[0]));
            printf("Switched to color: R=%.1f, G=%.1f, B=%.1f\n", colors[current_color_index][0], colors[current_color_index][1], colors[current_color_index][2]);
        } else if (strcmp(upload_data, "{\"button\": 3,\"state\": \"pressed\"}") == 0) {
            //undo
            if (line_count > 0) {
            line_count--;
            free_line_memory(&lines[line_count]);
            printf("Last line undone. Remaining lines: %d\n", line_count);
            } else {
                printf("No lines to undo.\n");
            }
        }

        *upload_data_size = 0; // Signal that we've processed this data
        return MHD_YES;
    }

    // Send response
    response = MHD_create_response_from_buffer(strlen(response_str), 
                                               (void *)response_str, 
                                               MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return (ret == MHD_YES) ? MHD_YES : MHD_NO; // Ensure a valid MHD_Result is returned
}

void* http_thread(void* arg) {
    struct MHD_Daemon *daemon;

    // Start the HTTP server
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, 
                              &handle_request, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        fprintf(stderr, "Failed to start HTTP server\n");
    }

    printf("HTTP server running on port %d\n", PORT);

    // Run server indefinitely
    getchar();

    // Stop the server
    MHD_stop_daemon(daemon);
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
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        save_to_png("drawing.png");
        printf("Drawing saved to drawing.png\n");
    } else if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        // Undo the last line
        if (line_count > 0) {
            line_count--;
            free_line_memory(&lines[line_count]);
            printf("Last line undone. Remaining lines: %d\n", line_count);
        } else {
            printf("No lines to undo.\n");
        }
    }

    // Handle spacebar key press and release
    if (key == GLFW_KEY_SPACE) {
        #ifdef __linux__
        if (action == GLFW_PRESS && allow_draw == 0) {
            allow_draw = 1; // Start drawing when spacebar is pressed
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
        } else if (action == GLFW_RELEASE && allow_draw == 1) {
            allow_draw = 0; // Stop drawing when spacebar is released
            line_count++;
        }
        #endif
    }
}
