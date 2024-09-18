#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point points[10000];  // Each line can have up to 10000 points
    int point_count;
    float color[3];  // Store the color for each line
} Line;

#define MAX_LINES 10000
Line lines[MAX_LINES];
int line_count = 0;
int is_drawing = 0;

float colors[][3] = {
    {1.0f, 0.0f, 0.0f},  // Red
    {0.0f, 1.0f, 0.0f},  // Green
    {0.0f, 0.0f, 1.0f},  // Blue
    {1.0f, 1.0f, 0.0f},  // Yellow
    {1.0f, 0.0f, 1.0f},  // Pink
    {0.5f, 0.0f, 0.5f},  // Purple
    {0.0f, 0.0f, 0.0f}   // Black
};
int current_color_index = 0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && line_count < MAX_LINES) {
            is_drawing = 1;  // Start drawing
            lines[line_count].point_count = 0;  // Start a new line
            // Set the color of the new line to the current color
            lines[line_count].color[0] = colors[current_color_index][0];
            lines[line_count].color[1] = colors[current_color_index][1];
            lines[line_count].color[2] = colors[current_color_index][2];
        } else if (action == GLFW_RELEASE) {
            is_drawing = 0;  // Stop drawing
            line_count++;     // Finished the current line, move to the next
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (is_drawing && lines[line_count].point_count < 1000) {
        // Convert mouse coordinates to OpenGL coordinates
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x = (2.0f * xpos / width) - 1.0f;
        float y = 1.0f - (2.0f * ypos / height);

        lines[line_count].points[lines[line_count].point_count].x = x;
        lines[line_count].points[lines[line_count].point_count].y = y;
        lines[line_count].point_count++;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        // Cycle to the next color
        current_color_index = (current_color_index + 1) % (sizeof(colors) / sizeof(colors[0]));
        printf("Switched to color: R=%.1f, G=%.1f, B=%.1f\n", colors[current_color_index][0], colors[current_color_index][1], colors[current_color_index][2]);
    }
}

void draw_lines() {
    glLineWidth(6.0f);  // Set line thickness (e.g., 6.0 for thicker lines)
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

int main(void) {
    // Initialize the library
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Set window hints to disable resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1280, 960, "Drawing Canvas", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set callbacks for mouse and key input
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    // Set the background color to white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw all the lines
        draw_lines();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
