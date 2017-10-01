#ifndef DRAW_H
#define DRAW_H

#include "libpdf/hpdf.h"
#include "view.h"

void draw(view* tree, HPDF_Page page);

#endif