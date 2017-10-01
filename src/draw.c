#include "draw.h"
#include "view.h"

void draw(view* v, HPDF_Page page) {
	int pageHeight = HPDF_Page_GetHeight(page);
	
	if (v->type == TYPE_TEXT_VIEW) {
		HPDF_Page_BeginText(page);
		HPDF_Page_SetFontAndSize(page, v->properties.text_view.font, 
			v->properties.text_view.size);
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
