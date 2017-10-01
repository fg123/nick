#include "layout.h"
#include "util.h"
#include "libpdf/hpdf.h"
#include <stdbool.h>
#include <string.h>

static bool is_viewgroup(view* v) {
	return v->type == TYPE_LINEAR_LAYOUT || v->type == TYPE_FRAME_LAYOUT;
}

static bool is_view(view* v) {
	return !is_viewgroup(v);
}

void measure(view* v, int max_width, int max_height, 
		measure_spec ms_width, measure_spec ms_height) {
	// IGNORING SPEC FOR NOW, just use as much as needed
	double desired_height = 0;
	double desired_width = 0;
	double width_margin = v->layout.margin_left + v->layout.margin_right;
	double height_margin = v->layout.margin_top + v->layout.margin_bottom;
	double width_padding = v->layout.padding_left + v->layout.padding_right;
	double height_padding = v->layout.padding_top + v->layout.padding_bottom;
	max_width -= width_margin;
	max_height -= height_margin;
	if (v->type == TYPE_TEXT_VIEW) {
		int lines = 1;
		HPDF_BYTE* text = v->properties.text_view.text;
		// Count Newlines
		char* t = text;
		while (*t) {
			if (t[0] == '\n') lines++;
			t++;
		}
		desired_width = 
			HPDF_Font_TextWidth(v->properties.text_view.font, text, 
				strlen(text)).width;
		HPDF_Rect bound_box = HPDF_Font_GetBBox(v->properties.text_view.font);
		desired_height = (bound_box.top - bound_box.bottom) * lines;
		desired_width = (desired_width * v->properties.text_view.size) / 1000.0;
		desired_height = (desired_height * v->properties.text_view.size) / 1000.0;
	}
	else if (v->type == TYPE_IMAGE_VIEW) {

	} 
	else if (v->type == TYPE_LINEAR_LAYOUT) {
		viewlist* child = v->properties.linear_layout.children;
		while (child) {
			measure(child->elem, max_width, 
				max_height, ms_width, ms_height);
			if (v->properties.linear_layout.orientation == 
				ORIENTATION_VERTICAL) {	
				desired_height += child->elem->layout.height;
				desired_width = max(desired_width, child->elem->layout.width);
			}
			else {
				desired_width += child->elem->layout.width;
				desired_height = max(desired_height, child->elem->layout.height);
			}
			child = child->next;
		}
		
	}
	else if (v->type == TYPE_FRAME_LAYOUT) {

	}

	if (v->layout.width_type == SIZE_WRAP_CONTENT) {
		v->layout.width = desired_width + width_padding;
	}
	else if (v->layout.width_type == SIZE_MATCH_PARENT) {
		v->layout.width = max_width;
	}

	if (v->layout.height_type == SIZE_WRAP_CONTENT) {
		v->layout.height = desired_height + height_padding;
	}
	else if (v->layout.height == SIZE_MATCH_PARENT) {
		v->layout.height = max_height;
	}
	
	v->layout.width_type = SIZE_EXACT;
	v->layout.height_type = SIZE_EXACT;
}

void position(view* v, int x, int y) {
	x += v->layout.margin_left;
	y += v->layout.margin_top;
	v->layout.x = x;
	v->layout.y = y;
	if (v->type == TYPE_LINEAR_LAYOUT) {
		viewlist* child = v->properties.linear_layout.children;
		while (child) {
			position(child->elem, x, y);
			if (v->properties.linear_layout.orientation == 
				ORIENTATION_VERTICAL) {	
				y += child->elem->layout.height;
			}
			else {
				x += child->elem->layout.width;
			}
			child = child->next;
		}
	}
	else if (v->type == TYPE_FRAME_LAYOUT) {

	}
}
void layout(view* v, int max_width, int max_height, 
		measure_spec ms_width, measure_spec ms_height) {
	// Measure Children and Position Them
	measure(v, max_width, max_height, ms_width, ms_height);
	position(v, 0, 0);
}

// takes a view tree and generates width, height, x and y values
void measure_and_layout(view* tree) {
	return layout(tree, (int)(72.0 * 8.5), 72 * 11, MS_AT_MOST, MS_AT_MOST);
}