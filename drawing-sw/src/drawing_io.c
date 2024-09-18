#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "drawing_io.h"
#include "drawing_main.h"

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
