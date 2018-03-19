#ifndef VERIFY_H
#define VERIFY_H

#include "libpdf/hpdf.h"
#include "libxml/tree.h"
#include "fonts.h"
#include "color.h"

// view.h: interface for a View object as well as a ViewGroup object
//   Provides options to build a ViewTree from a XML document, as well as
//   freeing the ViewTree
 
typedef enum size_type {
	SIZE_FILL,
	SIZE_AUTO,
	SIZE_EXACT
} size_type;

static const char* size_type_string[] = {
	"FILL", "AUTO", "EXACT"
};

typedef enum align_direction {
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER,
	ALIGN_JUSTIFY
} align_direction;

// gravity is an int, with bit fields set
typedef enum gravity_type {
	//GRAVITY_TOP 				= 0x00000001,
	GRAVITY_BOTTOM 				= 1 << 0,
	
	//GRAVITY_LEFT 				= 0x00000010,
	GRAVITY_RIGHT				= 1 << 1,

	GRAVITY_CENTER_HORIZONTAL	= 1 << 3,
	GRAVITY_CENTER_VERTICAL		= 1 << 4,
	GRAVITY_CENTER				= GRAVITY_CENTER_HORIZONTAL | GRAVITY_CENTER_VERTICAL
} gravity_type;

static const int default_gravity = 0;

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
		font_style style;
		align_direction align;
		HPDF_Font font;
		char* text;
		rgb color;
		char* link;
	} text_view;

	struct {	
		enum {
			SCALE_CENTER,
			SCALE_CENTER_CROP,
			SCALE_CENTER_INSIDE,
			SCALE_FIT_CENTER
		} scale_type;
		char* src;
		HPDF_Image image;
		double dpi;
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
    float margin_left;
    float margin_right;
    float margin_top;
    float margin_bottom;
    float padding_left;
    float padding_right;
    float padding_top;
	float padding_bottom;
	float border_left;
	float border_right;
	float border_top;
	float border_bottom;
	rgb border_color;
	int gravity;

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
