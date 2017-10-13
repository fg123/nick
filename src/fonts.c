#include <stdlib.h>
#include <string.h>
#include "fonts.h"
#include "error.h"
#include "pdf.h"
#include "util.h"

#define FONT_ENCODING_BUILT_IN "StandardEncoding"
#define FONT_ENCODING "UTF-8"

static font_node* fonts_ll = NULL;

static const char* built_in[] = {
	"Courier",
	"Courier-Bold",
	"Courier-Oblique",
	"Courier-BoldOblique",
	"Helvetica",
	"Helvetica-Bold",
	"Helvetica-Oblique",
	"Helvetica-BoldOblique",
	"Times-Roman",
	"Times-Bold",
	"Times-Italic",
	"Times-BoldItalic",
	"Symbol",
	"ZapfDingbats"
};

static void add_to_ll(char* name, HPDF_Font font, font_style style, int line);

void fonts_init() {
	add_to_ll("Courier", 
		HPDF_GetFont(pdf, built_in[0], FONT_ENCODING_BUILT_IN), STYLE_NONE, 0);
	add_to_ll("Courier", 
		HPDF_GetFont(pdf, built_in[1], FONT_ENCODING_BUILT_IN), STYLE_BOLD, 0);
	add_to_ll("Courier", 
		HPDF_GetFont(pdf, built_in[2], FONT_ENCODING_BUILT_IN), STYLE_ITALIC, 0);
	add_to_ll("Courier", 
		HPDF_GetFont(pdf, built_in[3], FONT_ENCODING_BUILT_IN), STYLE_BOLDITALIC, 0);
	add_to_ll("Helvetica", 
		HPDF_GetFont(pdf, built_in[4], FONT_ENCODING_BUILT_IN), STYLE_NONE, 0);
	add_to_ll("Helvetica", 
		HPDF_GetFont(pdf, built_in[5], FONT_ENCODING_BUILT_IN), STYLE_BOLD, 0);
	add_to_ll("Helvetica", 
		HPDF_GetFont(pdf, built_in[6], FONT_ENCODING_BUILT_IN), STYLE_ITALIC, 0);
	add_to_ll("Helvetica", 
		HPDF_GetFont(pdf, built_in[7], FONT_ENCODING_BUILT_IN), STYLE_BOLDITALIC, 0);
	add_to_ll("Times New Roman", 
		HPDF_GetFont(pdf, built_in[8], FONT_ENCODING_BUILT_IN), STYLE_NONE, 0);
	add_to_ll("Times New Roman", 
		HPDF_GetFont(pdf, built_in[9], FONT_ENCODING_BUILT_IN), STYLE_BOLD, 0);
	add_to_ll("Times New Roman", 
		HPDF_GetFont(pdf, built_in[10], FONT_ENCODING_BUILT_IN), STYLE_ITALIC, 0);
	add_to_ll("Times New Roman", 
		HPDF_GetFont(pdf, built_in[11], FONT_ENCODING_BUILT_IN), STYLE_BOLDITALIC, 0);
	add_to_ll("Symbol", 
		HPDF_GetFont(pdf, built_in[12], FONT_ENCODING_BUILT_IN), STYLE_NONE, 0);
	add_to_ll("ZapfDingbats", 
		HPDF_GetFont(pdf, built_in[13], FONT_ENCODING_BUILT_IN), STYLE_NONE, 0);
}

HPDF_Font get_font(char* name, font_style style, int line) {
	font_node* curr = fonts_ll;
	while (curr) {
		if (strcmp(curr->name, name) == 0) {
			if (style == STYLE_NONE && curr->regular) {
				return curr->regular;
			}
			else if (style == STYLE_BOLD && curr->bold) {
				return curr->bold;
			}
			else if (style == STYLE_ITALIC && curr->italic) {
				return curr->italic;
			}
			else if (style == STYLE_BOLDITALIC && curr->bold_italic) {
				return curr->bold_italic;
			}
			else {
				error(line, FONT_STYLE_NOT_FOUND, name, 
					font_style_string[style]);
			}
		}
		curr = curr->next;
	}
	error(line, FONT_NOT_FOUND, name);
}

void put_font(char* name, font_style style, char* path, int line) {
	const char* font_name = HPDF_LoadTTFontFromFile(pdf, path, HPDF_TRUE);
	HPDF_Font font = HPDF_GetFont(pdf, font_name, FONT_ENCODING);
	add_to_ll(name, font, style, line);
}

void free_fonts_ll() {
	font_node* curr = fonts_ll;
	while (curr) {
		font_node* next = curr->next;
		free(curr->name);
		free(curr);
		curr = next;
	}
}

static void add_to_ll(char* name, HPDF_Font font, font_style style, int line) {
	font_node* curr = fonts_ll;
	font_node* add_to = NULL;
	while (curr) {
		if (strcmp(curr->name, name) == 0) {
			add_to = curr; 
			break;
		}
		curr = curr->next;
	}
	if (!add_to) {
		add_to = malloc(sizeof(font_node));
		add_to->name = strdup(name);
		add_to->regular = NULL;
		add_to->bold = NULL;
		add_to->italic = NULL;
		add_to->bold_italic = NULL;
		add_to->next = fonts_ll;
		fonts_ll = add_to;
	}
	if (style == STYLE_NONE && !add_to->regular) {
		add_to->regular = font;
		return;
	}
	else if (style == STYLE_BOLD && !add_to->bold) {
		add_to->bold = font;
		return;
	}
	else if (style == STYLE_ITALIC && !add_to->italic) {
		add_to->italic = font;
		return;
	}
	else if (style == STYLE_BOLDITALIC && !add_to->bold_italic) {
		add_to->bold_italic = font;
		return;
	}
	else {
		error(line, FONT_STYLE_ALREADY_LOADED, name, 
			font_style_string[style]);
	}
}