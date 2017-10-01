#ifndef VERIFY_H
#define VERIFY_H

#include "libpdf/hpdf.h"
#include "libxml/tree.h"
// view.h: interface for a View object as well as a ViewGroup object
//   Provides options to build a ViewTree from a XML document, as well as
//   freeing the ViewTree
 
typedef enum size_type {
	SIZE_MATCH_PARENT,
	SIZE_WRAP_CONTENT,
	SIZE_EXACT
} size_type;

static const char* size_type_string[] = {
	"MATCH_PARENT", "WRAP_CONTENT", "EXACT"
};

typedef enum type {
	// View Groups
	TYPE_FRAME_LAYOUT,
	TYPE_LINEAR_LAYOUT,

	// Singular Views
	TYPE_TEXT_VIEW,
	TYPE_IMAGE_VIEW
} view_type;

static const char* view_type_string[] = {
	"FRAME_LAYOUT", "LINEAR_LAYOUT", "TEXT_VIEW", "IMAGE_VIEW"
};

typedef struct view view;

typedef struct viewlist {
	view* elem;
	struct viewlist* next;
} viewlist;

typedef union {
	// View Groups
	struct {	viewlist* children; 	} frame_layout;
	struct {	viewlist* children;
				enum { ORIENTATION_VERTICAL,
					ORIENTATION_HORIZONTAL } orientation;
				} linear_layout;

	// Singular Views
	struct {	
		int size;
		enum {
			STYLE_NONE,
			STYLE_BOLD,
			STYLE_ITALIC,
			STYLE_BOLDITALIC
		} style;
		HPDF_Font font;
		char* text;
		char color[6];
	} text_view;

	struct {	
		enum {
			SCALE_CENTER,
			SCALE_CENTER_CROP,
			SCALE_CENTER_INSIDE,
			SCALE_FIT_CENTER
		} scale_type;
		char* src;
	} image_view;
} view_properties;

static const char* orientation_string[] = {
	"ORIENTATION_VERTICAL", "ORIENTATION_HORIZONTAL"
};

static const char* style_string[] = {
	"STYLE_NONE", "STYLE_BOLD", "STYLE_ITALIC", "STYLE_BOLDITALIC"
};

static const char* scale_type_string[] = {
	"SCALE_CENTER", "SCALE_CENTER_CROP", "SCALE_CENTER_INSIDE", "SCALE_FIT_CENTER"
};

typedef struct layout_params {
	
	// Defined Types
    size_type width_type;
    size_type height_type;
    double width;
	double height;
    int margin_left;
    int margin_right;
    int margin_top;
    int margin_bottom;
    int padding_left;
    int padding_right;
    int padding_top;
	int padding_bottom;
	
	// Width and Height as well as position will be used in layout and draw
	double x;
	double y;
} layout_params;

struct view {
	view_type type;
	view_properties properties;
	layout_params layout;
};

// build_view(rootnode) builds a ViewTree from the page node representing
//   the page.
view* build_page(xmlNode* pagenode);

// print_view(tree) prints out the tree
void print_view(view* tree);

// free_view(tree) frees the allocated memory
void free_view(view* tree);

#endif