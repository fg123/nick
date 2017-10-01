#ifndef LAYOUT_H
#define LAYOUT_H

#include "view.h"

typedef enum {
	MS_UNSPECIFIED,
	MS_AT_MOST
} measure_spec;

// Generates a list of views to draw from the original tree
void measure_and_layout(view* tree);

#endif