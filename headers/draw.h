#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>
#include "libpdf/hpdf.h"
#include "view.h"

void draw(view* tree, HPDF_Page* pages, int pagec);

#endif