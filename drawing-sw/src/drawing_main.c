#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drawing_main.h"
#include "drawing_io.h"
#include "callbacks.h"

#ifdef __Linux__
#include <pthread.h>
#endif

// Initialize the global variables
Line lines[MAX_LINES];
int line_count = 0;
int is_drawing = 0;
int current_color_index = 0;

void draw_lines() {
    glLineWidth(6.0f);  // Set line thickness
    for (int i = 0; i < line_count; ++i) {
        if (lines[i].point_count > 1) {
            // Set the color of the line
            glColor3f(lines[i].color[0], lines[i].color[1], lines[i].color[2]);
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < lines[i].point_count; ++j) {
                glVertex2f(lines[i].points[j].x, lines[i].points[j].y);
            }
            glEnd();
        }
    }

    // Draw the current line while it's being drawn (during drag)
    if (is_drawing && lines[line_count].point_count > 1) {
        // Set the color of the current line
        glColor3f(lines[line_count].color[0], lines[line_count].color[1], lines[line_count].color[2]);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < lines[line_count].point_count; ++j) {
            glVertex2f(lines[line_count].points[j].x, lines[line_count].points[j].y);
        }
        glEnd();
    }
}

void free_line_memory(Line* line) {
    free(line->points);
    line->points = NULL;
}

int main(void) {
    // Initialize the library
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Set window hints to disable resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Drawing Canvas", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set the background color to white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Enable anti-aliasing for smoother lines
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Set callbacks for mouse and key input
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    // Initialize MQTT (Linux)
    #ifdef __linux__
    pthread_t mqtt_thread_id;
    pthread_create(&mqtt_thread_id, NULL, mqtt_thread, NULL);
    setup_mqtt();  // Setup MQTT
    pthread_t http_thread_id;
    pthread_create(&http_thread_id, NULL, http_thread, NULL);
    #endif

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw all the lines on top of the background
        draw_lines();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up allocated memory
    for (int i = 0; i < line_count; i++) {
        free_line_memory(&lines[i]);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    #ifdef __linux__
    // Clean up MQTT client if used
    if (use_mqtt) {
        MQTTClient_disconnect(client, 1000);
        MQTTClient_destroy(&client);
    }
    #endif

    return 0;
}
