#include "draw.h"
#include "view.h"
#include "util.h"
#include "color.h"

static void set_page_color(HPDF_Page page, rgb c) {
	HPDF_Page_SetRGBFill(page, c.red, c.green, c.blue);
}

static void draw_rectangle(HPDF_Page page, rgb c, 
		double x, double y, double width, double height) {
	HPDF_Page_SetRGBFill(page, c.red, 
		c.green, c.blue);
	HPDF_Page_Rectangle(page, x, y, width, height);
	HPDF_Page_Fill(page);
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
	// Draw Borders if requested
	// Y travels in the negative direction
	float inner_tx = v->layout.x;
	float inner_ty = pageHeight - v->layout.y;
	float inner_bx = inner_tx + v->layout.width;
	float inner_by = inner_ty - v->layout.height;

	float outer_tx = inner_tx - v->layout.border_left;
	float outer_ty = inner_ty + v->layout.border_top;
	float outer_bx = inner_bx + v->layout.border_right;
	float outer_by = inner_by - v->layout.border_bottom;
	
	if (v->layout.border_left) {
		draw_rectangle(page, v->layout.border_color, 
			outer_tx,
			outer_by, 
			v->layout.border_left,
			outer_ty - outer_by);
	}
	if (v->layout.border_right) {
		draw_rectangle(page, v->layout.border_color, 
			inner_bx,
			outer_by, 
			v->layout.border_right,
			outer_ty - outer_by);
	}
	if (v->layout.border_top) {
		draw_rectangle(page, v->layout.border_color, 
			outer_tx,
			outer_ty, 
			outer_bx - outer_tx,
			-v->layout.border_top);
	}
	if (v->layout.border_bottom) {
		draw_rectangle(page, v->layout.border_color, 
			outer_tx,
			outer_by, 
			outer_bx - outer_tx,
			v->layout.border_bottom);
	}

	if (v->type == TYPE_TEXT_VIEW) {
		HPDF_Rect bbox = HPDF_Font_GetBBox(v->properties.text_view.font);
		set_page_color(page, v->properties.text_view.color);
		HPDF_Page_BeginText(page);
		HPDF_Page_SetFontAndSize(page, v->properties.text_view.font, 
			v->properties.text_view.size);
		HPDF_Page_SetTextLeading(page, v->properties.text_view.size * 1.15);
		HPDF_TextAlignment alignment = HPDF_TALIGN_LEFT;
		switch (v->properties.text_view.align) {
			case ALIGN_RIGHT: alignment = HPDF_TALIGN_RIGHT; break;
			case ALIGN_CENTER: alignment = HPDF_TALIGN_CENTER; break;
			case ALIGN_JUSTIFY: alignment = HPDF_TALIGN_JUSTIFY; break;
		}
		HPDF_Page_TextRect(page, 
			v->layout.x + v->layout.padding_left, 
			pageHeight - (v->layout.y + v->layout.padding_top),
			v->layout.x + v->layout.width - v->layout.padding_right, 
			pageHeight - (v->layout.y + v->layout.height - v->layout.padding_bottom),
			v->properties.text_view.text, 
			alignment, NULL);
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
