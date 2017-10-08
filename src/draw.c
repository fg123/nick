#include "draw.h"
#include "view.h"
#include "util.h"
#include "color.h"

void set_page_color(HPDF_Page page, char* color) {
	rgb c = stoc(color);
	HPDF_Page_SetRGBFill(page, c.red, c.green, c.blue);
}

void draw_rectangle(HPDF_Page page, rgb c, 
		double x, double y, double width, double height) {
	HPDF_Page_SetRGBFill(page, c.red, 
		c.green, c.blue);
	HPDF_Page_Rectangle(page, x, y, width, height);
	HPDF_Page_FillStroke(page);
}

void draw(view* v, HPDF_Page page) {
	int pageHeight = HPDF_Page_GetHeight(page);
	if (get_settings_flag(SETTINGS_SHOW_BOUNDING_BOX)) {
		draw_rectangle(page, COLOR_MARGIN, 
			v->layout.x - v->layout.margin_left,
			pageHeight - v->layout.y - (v->layout.height + v->layout.margin_bottom), 
			v->layout.width + v->layout.margin_right + v->layout.margin_left,
			v->layout.height + v->layout.margin_bottom + v->layout.margin_top);
		draw_rectangle(page, COLOR_PADDING, 
			v->layout.x,
			pageHeight - v->layout.y - v->layout.height, 
			v->layout.width,
			v->layout.height);
		draw_rectangle(page, COLOR_CONTENT, 
			v->layout.x + v->layout.padding_left, 
			pageHeight - v->layout.y - (v->layout.height - v->layout.padding_bottom), 
			v->layout.width - v->layout.padding_right - v->layout.padding_left, 
			v->layout.height - v->layout.padding_bottom - v->layout.padding_top);
	}

	if (v->type == TYPE_TEXT_VIEW) {
		HPDF_Rect bbox = HPDF_Font_GetBBox(v->properties.text_view.font);
		set_page_color(page, "000000");
		HPDF_Page_BeginText(page);
		HPDF_Page_SetFontAndSize(page, v->properties.text_view.font, 
			v->properties.text_view.size);
		HPDF_Page_SetTextLeading(page, v->properties.text_view.size * 1.15);
		
		HPDF_Page_TextRect(page, 
			v->layout.x + v->layout.padding_left, 
			pageHeight - (v->layout.y + v->layout.padding_top),
			v->layout.x + v->layout.width - v->layout.padding_right, 
			pageHeight - (v->layout.y + v->layout.height - v->layout.padding_bottom),
			v->properties.text_view.text, 
			HPDF_TALIGN_LEFT, NULL);
		HPDF_Page_EndText(page);
	}
	else if (v->type == TYPE_IMAGE_VIEW) {

	}
	else if (v->type == TYPE_LINEAR_LAYOUT) {
		viewlist* child = v->properties.linear_layout.children;
		while (child) {
			draw(child->elem, page);
			child = child->next;
		}
	}
	else if (v->type == TYPE_FRAME_LAYOUT) {

	}
}
