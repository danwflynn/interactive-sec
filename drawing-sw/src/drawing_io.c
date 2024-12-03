#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "drawing_io.h"
#include "drawing_main.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void save_lines(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Failed to open file for writing: %s\n", strerror(errno));
        return;
    }

    // Write the number of lines
    fwrite(&line_count, sizeof(int), 1, file);

    // Write each line's data
    for (int i = 0; i < line_count; ++i) {
        Line* line = &lines[i];
        
        // Write the line's color
        fwrite(line->color, sizeof(float), 3, file);

        // Write the number of points
        fwrite(&line->point_count, sizeof(int), 1, file);

        // Write each point
        fwrite(line->points, sizeof(Point), line->point_count, file);
    }

    fclose(file);
}

void load_lines(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file for reading: %s\n", strerror(errno));
        return;
    }

    // Free existing line data
    for (int i = 0; i < line_count; i++) {
        free_line_memory(&lines[i]);
    }
    line_count = 0;

    // Read the number of lines
    fread(&line_count, sizeof(int), 1, file);

    // Read each line's data
    for (int i = 0; i < line_count; ++i) {
        if (line_count >= MAX_LINES) {
            fprintf(stderr, "Too many lines in file\n");
            fclose(file);
            return;
        }

        Line* line = &lines[i];

        // Allocate memory for the new line
        line->points = (Point*)malloc(1000 * sizeof(Point));  // Initial capacity
        if (line->points == NULL) {
            fprintf(stderr, "Failed to allocate memory for points\n");
            fclose(file);
            return;
        }

        // Read the line's color
        fread(line->color, sizeof(float), 3, file);

        // Read the number of points
        fread(&line->point_count, sizeof(int), 1, file);

        // Read each point
        line->point_capacity = line->point_count;
        line->points = (Point*)realloc(line->points, line->point_capacity * sizeof(Point));
        fread(line->points, sizeof(Point), line->point_count, file);
    }

    fclose(file);
}


void save_to_png(const char* filename) {
    unsigned char* image = (unsigned char*)calloc(WINDOW_WIDTH * WINDOW_HEIGHT * 3, sizeof(unsigned char));
    if (!image) {
        fprintf(stderr, "Failed to allocate memory for the image\n");
        return;
    }

    // Draw lines into the image buffer
    for (int i = 0; i < line_count; ++i) {
        Line* line = &lines[i];
        for (int j = 0; j < line->point_count - 1; ++j) {
            // Map normalized coordinates ([-1,1]) to image space ([0, IMAGE_WIDTH/HEIGHT])
            int x1 = (line->points[j].x + 1.0f) * 0.5f * WINDOW_WIDTH;
            int y1 = (line->points[j].y + 1.0f) * 0.5f * WINDOW_HEIGHT;
            int x2 = (line->points[j + 1].x + 1.0f) * 0.5f * WINDOW_WIDTH;
            int y2 = (line->points[j + 1].y + 1.0f) * 0.5f * WINDOW_HEIGHT;

            // Convert color values (0.0-1.0) to 0-255
            int r = (int)(line->color[0] * 255);
            int g = (int)(line->color[1] * 255);
            int b = (int)(line->color[2] * 255);

            // Draw the line using Bresenham's algorithm
            int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
            int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
            int err = (dx > dy ? dx : -dy) / 2, e2;

            while (1) {
                if (x1 >= 0 && x1 < WINDOW_WIDTH && y1 >= 0 && y1 < WINDOW_HEIGHT) {
                    int idx = (y1 * WINDOW_WIDTH + x1) * 3;
                    image[idx] = r;
                    image[idx + 1] = g;
                    image[idx + 2] = b;
                }
                if (x1 == x2 && y1 == y2) break;
                e2 = err;
                if (e2 > -dx) { err -= dy; x1 += sx; }
                if (e2 < dy) { err += dx; y1 += sy; }
            }
        }
    }

    // Write the image to a PNG file
    if (!stbi_write_png(filename, WINDOW_WIDTH, WINDOW_HEIGHT, 3, image, WINDOW_WIDTH * 3)) {
        fprintf(stderr, "Failed to write PNG file\n");
    }

    free(image);
}
