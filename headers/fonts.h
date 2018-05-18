#ifndef FONTS_H
#define FONTS_H

#include "hpdf.h"

typedef enum {
	STYLE_NONE,
	STYLE_BOLD,
	STYLE_ITALIC,
	STYLE_BOLDITALIC
} font_style;

static const char* font_style_string[] = {
	"NONE/REGULAR",
	"BOLD",
	"ITALIC",
	"BOLD-ITALIC"
};

typedef struct font_node {
	char* name;
	HPDF_Font regular;
	HPDF_Font bold;
	HPDF_Font italic;
	HPDF_Font bold_italic;
	struct font_node* next;
} font_node;

void fonts_init();

HPDF_Font get_font(char* name, font_style style, int line);

void put_font(char* name, font_style style, char* path, int line);

void free_fonts_ll();

#endif
