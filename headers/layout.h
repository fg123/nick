#ifndef LAYOUT_H
#define LAYOUT_H

#include "view.h"

extern double* page_boundaries;

// Generates a list of views to draw from the original tree
int measure_and_layout(view* tree, int page_width, int page_height, int start_x, int start_y);

bool is_viewgroup(view* v);
bool is_view(view* v);
#endif