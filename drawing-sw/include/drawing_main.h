#ifndef MAIN_H
#define MAIN_H

#include <GLFW/glfw3.h>
#include <GL/gl.h>

// Define the maximum number of lines and initial point capacity
#define MAX_LINES 10000
#define INITIAL_POINT_CAPACITY 1000

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point* points;         // Pointer to dynamically allocated array of points
    int point_count;
    int point_capacity;    // Capacity of the points array
    float color[3];
} Line;

extern Line lines[MAX_LINES];
extern int line_count;
extern int is_drawing;

extern float colors[][3];
extern int current_color_index;

// Function declarations
void draw_lines();
void free_line_memory(Line* line);

#endif // MAIN_H
