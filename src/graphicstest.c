#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point points[1000];  // Each line can have up to 1000 points
    int point_count;
} Line;

#define MAX_LINES 1000
Line lines[MAX_LINES];
int line_count = 0;
int is_drawing = 0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && line_count < MAX_LINES) {
            is_drawing = 1; // Start drawing
            lines[line_count].point_count = 0;  // Start a new line
        } else if (action == GLFW_RELEASE) {
            is_drawing = 0; // Stop drawing
            line_count++;   // Finished the current line, move to the next
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

void draw_lines() {
    glColor3f(0.0f, 0.0f, 0.0f);  // Set line color to black
    for (int i = 0; i < line_count; ++i) {
        if (lines[i].point_count > 1) {
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < lines[i].point_count; ++j) {
                glVertex2f(lines[i].points[j].x, lines[i].points[j].y);
            }
            glEnd();
        }
    }
    
    // Draw the current line while it's being drawn (during drag)
    if (is_drawing && lines[line_count].point_count > 1) {
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

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "Drawing Canvas", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set callbacks for mouse input
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

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
