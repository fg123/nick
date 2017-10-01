#include "view.h"
#include "libxml/tree.h"
#include "libpdf/hpdf.h"
#include "error.h"
#include "pdf.h"
#include "util.h"
#include <stdbool.h>
#include <string.h>

// view.c implements the building of the ViewTree from a PageNode

// XML String Constants
#define XML_FRAME_LAYOUT 			(const xmlChar*) "FrameLayout"
#define XML_LINEAR_LAYOUT 			(const xmlChar*) "LinearLayout"
#define XML_TEXT_VIEW 				(const xmlChar*) "TextView"
#define XML_IMAGE_VIEW 				(const xmlChar*) "ImageView"

#define XML_LAYOUT_WIDTH 			(const xmlChar*) "width"
#define XML_LAYOUT_HEIGHT 			(const xmlChar*) "height"

#define XML_LAYOUT_SIZE_MATCH_PARENT (const xmlChar*)"match_parent"
#define XML_LAYOUT_SIZE_WRAP_CONTENT (const xmlChar*)"wrap_content"

#define XML_LAYOUT_MARGIN 			(const xmlChar*) "margin"
#define XML_LAYOUT_MARGIN_TOP 		(const xmlChar*) "margin-top"
#define XML_LAYOUT_MARGIN_BOTTOM 	(const xmlChar*) "margin-bottom"
#define XML_LAYOUT_MARGIN_LEFT	 	(const xmlChar*) "margin-left"
#define XML_LAYOUT_MARGIN_RIGHT 	(const xmlChar*) "margin-right"
#define XML_LAYOUT_PADDING 			(const xmlChar*) "padding"
#define XML_LAYOUT_PADDING_TOP 		(const xmlChar*) "padding-top"
#define XML_LAYOUT_PADDING_BOTTOM 	(const xmlChar*) "padding-bottom"
#define XML_LAYOUT_PADDING_LEFT 	(const xmlChar*) "padding-left"
#define XML_LAYOUT_PADDING_RIGHT 	(const xmlChar*) "padding-right"

// XML Properties
#define XML_ORIENTATION				(const xmlChar*) "orientation"
#define XML_ORIENTATION_VERTICAL 	(const xmlChar*) "vertical"
#define XML_ORIENTATION_HORIZONTAL 	(const xmlChar*) "horizontal"
#define XML_TEXT					(const xmlChar*) "text"
#define XML_FONT					(const xmlChar*) "font"
#define XML_TEXT_SIZE				(const xmlChar*) "size"
#define XML_TEXT_STYLE				(const xmlChar*) "style"
#define XML_TEXT_STYLE_NONE			(const xmlChar*) "none"
#define XML_TEXT_STYLE_BOLD			(const xmlChar*) "bold"
#define XML_TEXT_STYLE_ITALIC		(const xmlChar*) "italic"
#define XML_TEXT_STYLE_BOLDITALIC	(const xmlChar*) "bolditalic"
#define XML_SRC						(const xmlChar*) "src"
#define XML_SCALE_TYPE				(const xmlChar*) "scale-type"
#define XML_SCALE_TYPE_CENTER		(const xmlChar*) "center"
#define XML_SCALE_TYPE_CROP			(const xmlChar*) "center-crop"
#define XML_SCALE_TYPE_INSIDE		(const xmlChar*) "center-inside"
#define XML_SCALE_TYPE_FIT_CENTER	(const xmlChar*) "fit-center"

static int indentation = 0;

// Forward Declarations
view *build_view(xmlNode*);
view *build_viewgroup(xmlNode*);

static bool is_viewgroup(const xmlNode* node) {
	return xmlStrEqual(node->name, XML_FRAME_LAYOUT)
	|| xmlStrEqual(node->name, XML_LINEAR_LAYOUT);
}

static bool is_view(const xmlNode* node) {
	return xmlStrEqual(node->name, XML_TEXT_VIEW)
	|| xmlStrEqual(node->name, XML_IMAGE_VIEW);
}

view_properties get_frame_layout_properties(const xmlNode* node, 
		viewlist* child_list) {
	view_properties properties;
	// Setup Default Values
	properties.frame_layout.children = child_list;
	return properties;
}

view_properties get_text_view_properties(const xmlNode* node) {
	view_properties properties;
	// Setup Default Values
	properties.text_view.size = 14;
	properties.text_view.style = STYLE_NONE;
	properties.text_view.text = strdup("");
	// Grab Properties
	xmlChar* text_size = xmlGetProp(node, XML_TEXT_SIZE);
	xmlChar* font = xmlGetProp(node, XML_FONT);
	xmlChar* text_style = xmlGetProp(node, XML_TEXT_STYLE);
	xmlChar* text = xmlGetProp(node, XML_TEXT);
	if (text_size) {
		properties.text_view.size = atoi(text_size);
		free(text_size);
	}
	if (text) {
		free(properties.text_view.text);
		properties.text_view.text = formatSpecialCharacters(text);
		free(text);
	}
	if (text_style) {
		if (xmlStrEqual(text_style, XML_TEXT_STYLE_NONE)) {
			properties.text_view.style = STYLE_NONE;
		}
		else if (xmlStrEqual(text_style, XML_TEXT_STYLE_BOLD)) {
			properties.text_view.style = STYLE_BOLD;
		}
		else if (xmlStrEqual(text_style, XML_TEXT_STYLE_ITALIC)) {
			properties.text_view.style = STYLE_ITALIC;
		}
		else if (xmlStrEqual(text_style, XML_TEXT_STYLE_BOLDITALIC)) {
			properties.text_view.style = STYLE_BOLDITALIC;
		}
		else {
			// Maybe Error Handle, but we can just ignore invalid values	
		}
		free(text_style);
	}
	if (font) {
		properties.text_view.font = HPDF_GetFont(pdf, font, NULL);
		free(font);
	}
	else {
		if (properties.text_view.style == STYLE_NONE) {
			properties.text_view.font = HPDF_GetFont(pdf, "Helvetica", NULL);
		}
		else if (properties.text_view.style == STYLE_BOLD) {
			properties.text_view.font = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);
		}
		else if (properties.text_view.style == STYLE_ITALIC) {
			properties.text_view.font = HPDF_GetFont(pdf, "Helvetica-Oblique", NULL);
		}
		else {
			properties.text_view.font = HPDF_GetFont(pdf, "Helvetica-BoldOblique", NULL);
		}
	}
	return properties;
}

view_properties get_image_view_properties(const xmlNode* node) {
	view_properties properties;
	// Setup Default Values
	properties.image_view.scale_type = SCALE_CENTER;
	properties.image_view.src = strdup("");
	// Grab Properties
	xmlChar* scale_type = xmlGetProp(node, XML_SCALE_TYPE);
	xmlChar* src = xmlGetProp(node, XML_SRC);
	if (src) {
		free(properties.image_view.src);
		properties.image_view.src = strdup(src);
		free(src);
	}
	if (scale_type) {
		if (xmlStrEqual(scale_type, XML_SCALE_TYPE_CENTER)) {
			properties.image_view.scale_type = SCALE_CENTER;
		}
		else if (xmlStrEqual(scale_type, XML_SCALE_TYPE_CROP)) {
			properties.image_view.scale_type = SCALE_CENTER_CROP;
		}
		else if (xmlStrEqual(scale_type, XML_SCALE_TYPE_INSIDE)) {
			properties.image_view.scale_type = SCALE_CENTER_INSIDE;
		}
		else if (xmlStrEqual(scale_type, XML_SCALE_TYPE_FIT_CENTER)) {
			properties.image_view.scale_type = SCALE_FIT_CENTER;
		}
		else {
			// Maybe Error Handle, but we can just ignore invalid values	
		}
		free(scale_type);
	}
	return properties;
}

view_properties get_linear_layout_properties(const xmlNode* node, 
		viewlist* child_list) {
	view_properties properties;
	// Setup Default Values
	properties.linear_layout.children = child_list;
	properties.linear_layout.orientation = ORIENTATION_VERTICAL;
	// Grab Properties
	xmlChar* orientation = xmlGetProp(node, XML_ORIENTATION);
	if (orientation) {
		if (xmlStrEqual(orientation, XML_ORIENTATION_HORIZONTAL)) {
			properties.linear_layout.orientation = ORIENTATION_HORIZONTAL;
		}
		else if (xmlStrEqual(orientation, XML_ORIENTATION_VERTICAL)) {
			properties.linear_layout.orientation = ORIENTATION_VERTICAL;
		}
		else {
			// Maybe Error Handle, but we can just ignore invalid values	
		}
		free(orientation);
	}
	return properties;
}

layout_params get_layout_params(const xmlNode* node) {
	layout_params layout;
	// Setup Default Values
	layout.margin_left = 0;
	layout.margin_right = 0;
	layout.margin_top = 0;
	layout.margin_bottom = 0;
	layout.padding_left = 0;
	layout.padding_right = 0;
	layout.padding_top = 0;
	layout.padding_bottom = 0;
	layout.width_type = SIZE_WRAP_CONTENT;
	layout.height_type = SIZE_WRAP_CONTENT;
	layout.x = 0;
	layout.y = 0;
	// Width and Height are Required
	xmlChar* width = xmlGetProp(node, XML_LAYOUT_WIDTH);
	xmlChar* height = xmlGetProp(node, XML_LAYOUT_HEIGHT);
	xmlChar* margin = xmlGetProp(node, XML_LAYOUT_MARGIN);
	xmlChar* margin_top = xmlGetProp(node, XML_LAYOUT_MARGIN_TOP);
	xmlChar* margin_bottom = xmlGetProp(node, XML_LAYOUT_MARGIN_BOTTOM);
	xmlChar* margin_left = xmlGetProp(node, XML_LAYOUT_MARGIN_LEFT);
	xmlChar* margin_right = xmlGetProp(node, XML_LAYOUT_MARGIN_RIGHT);
	xmlChar* padding = xmlGetProp(node, XML_LAYOUT_PADDING);
	xmlChar* padding_top = xmlGetProp(node, XML_LAYOUT_PADDING_TOP);
	xmlChar* padding_bottom = xmlGetProp(node, XML_LAYOUT_PADDING_BOTTOM);
	xmlChar* padding_left = xmlGetProp(node, XML_LAYOUT_PADDING_LEFT);
	xmlChar* padding_right = xmlGetProp(node, XML_LAYOUT_PADDING_RIGHT);
	
	if (width) {
		if (xmlStrEqual(width, XML_LAYOUT_SIZE_MATCH_PARENT)) {
			layout.width_type = SIZE_MATCH_PARENT;
		}
		else if (xmlStrEqual(width, XML_LAYOUT_SIZE_WRAP_CONTENT)) {
			layout.width_type = SIZE_WRAP_CONTENT;
		}
		else {
			layout.width = atoi(width);
			layout.width_type = SIZE_EXACT;
		}
		free(width);
	}

	if (height) {
		if (xmlStrEqual(height, XML_LAYOUT_SIZE_MATCH_PARENT)) {
			layout.height_type = SIZE_MATCH_PARENT;
		}
		else if (xmlStrEqual(height, XML_LAYOUT_SIZE_WRAP_CONTENT)) {
			layout.height_type = SIZE_WRAP_CONTENT;
		}
		else {
			layout.height = atoi(height);
			layout.height_type = SIZE_EXACT;
		}
		free(height);
	}

	// Specific Margin Definitions are prioritized
	if (margin) {
		layout.margin_left = layout.margin_right =
		layout.margin_bottom = layout.margin_top = atoi(margin);
		free(margin);
	}
	if (padding) {
		layout.padding_left = layout.padding_right =
		layout.padding_bottom = layout.padding_top = atoi(padding);
		free(padding);
	}
	if (margin_left) {
		layout.margin_left = atoi(margin_left);
		free(margin_left);
	}
	if (margin_right) {
		layout.margin_right = atoi(margin_right);
		free(margin_right);
	}
	if (margin_top) {
		layout.margin_top = atoi(margin_top);
		free(margin_top);
	}
	if (margin_bottom) {
		layout.margin_bottom = atoi(margin_bottom);
		free(margin_bottom);
	}
	if (padding_left) {
		layout.padding_left = atoi(padding_left);
		free(padding_left);
	}
	if (padding_right) {
		layout.padding_right = atoi(padding_right);
		free(padding_right);
	}
	if (padding_top) {
		layout.padding_top = atoi(padding_top);
		free(padding_top);
	}
	if (padding_bottom) {
		layout.padding_bottom = atoi(padding_bottom);
		free(padding_bottom);
	}
	return layout;
}

view_type get_view_type(const xmlNode* node) {
	if (xmlStrEqual(node->name, XML_FRAME_LAYOUT)) {
		return TYPE_FRAME_LAYOUT;
	}
	else if (xmlStrEqual(node->name, XML_LINEAR_LAYOUT)) {
		return TYPE_LINEAR_LAYOUT;
	}
	else if (xmlStrEqual(node->name, XML_TEXT_VIEW)) {
		return TYPE_TEXT_VIEW;
	}
	else if (xmlStrEqual(node->name, XML_IMAGE_VIEW)) {
		return TYPE_IMAGE_VIEW;
	}
	else {
		error(node->line, UNRECOGNIZED_ELEMENT, node->name);
		return -1;
	}
}

static void print_element_names(xmlNode* a_node) {
	xmlNode* cur_node = NULL;
	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("node type: Element, name: %s\n", cur_node->name);
		}
		if (cur_node->children) {
			printf("Children!\n");
			print_element_names(cur_node->children);
		}
	}
}

view* build_view(xmlNode* node) {
	if (is_viewgroup(node)) {
		return build_viewgroup(node);
	}
	if (!is_view(node)) {
		error(node->line, UNRECOGNIZED_ELEMENT, node->name);
	}
	view* new_view = malloc(sizeof(view));
	new_view->type = get_view_type(node);
	new_view->layout = get_layout_params(node);
	// Grab attributes based on type
	switch (new_view->type) {
		case TYPE_IMAGE_VIEW:
			new_view->properties = get_image_view_properties(node);
			break;
		case TYPE_TEXT_VIEW:
			new_view->properties = get_text_view_properties(node);
			break;
	}
	return new_view;
}

// Given an XML ViewGroup Node
view* build_viewgroup(xmlNode* node) {
	viewlist* child_list = NULL;
	viewlist** head_ptr = &child_list;
	for (xmlNode* curr_child = node->children; 
		 curr_child; 
		 curr_child = curr_child->next) {
		if (xmlIsBlankNode(curr_child)) continue;
		if (node->type != XML_ELEMENT_NODE) {
		    error(node->line, EXPECTED_ELEMENT_IN_VIEWGROUP, node->type);
		}
		viewlist* new_child = malloc(sizeof(viewlist));
		new_child->elem = build_view(curr_child);
		new_child->next = NULL;
		*head_ptr = new_child;
		head_ptr = &new_child->next;
	}
	view* new_view = malloc(sizeof(view));
	new_view->type = get_view_type(node);
	new_view->layout = get_layout_params(node);
	// Grab attributes based on type
	switch (new_view->type) {
		case TYPE_FRAME_LAYOUT:
			new_view->properties = 
				get_frame_layout_properties(node, child_list);
			break;
		case TYPE_LINEAR_LAYOUT:
			new_view->properties = 
				get_linear_layout_properties(node, child_list);
			break;
	}
	return new_view;
}

// Given a page node, verify it contains only one element, that is, a ViewGroup,
//   then pass that into build_viewgroup.
view* build_page(xmlNode* pagenode) {
	// Node is a Page Node
	long element_count = xmlChildElementCount(pagenode);	
	if (element_count != 1) {
		error(pagenode->line, PAGE_MUST_HAVE_ONLY_ONE, element_count);
	}
	xmlNode* root_child = xmlFirstElementChild(pagenode);
	if (!is_viewgroup(root_child)) {
		error(pagenode->line, PAGE_MUST_START_VIEWGROUP);
	}
	return build_view(root_child);
}

void print_indent() {
    char a[50] = {0};
    for (int i = 0; i < indentation; i++) {
        strcat(a, "| ");
    }
    strcat(a, "`-");
    printf("%s", a);
}

void print_viewlist(viewlist* list) {
	if (!list) return;
	print_indent();
	printf(CYN "<ViewList Item>\n" RESET);
	indentation++;
	print_view(list->elem);
	indentation--;
	print_viewlist(list->next);
}

void print_layout_params(layout_params layout) {
	print_indent();
	printf(BLU "Layout Params:\n" RESET);
	indentation++;
	print_indent();
	if (layout.width_type == SIZE_EXACT) {
		printf(GRN "Width: " RESET "%f\n", layout.width);
	}
	else {
		printf(GRN "Width: " RESET "%s\n", 
			size_type_string[layout.width_type]);
	}
	print_indent();
	if (layout.height_type == SIZE_EXACT) {
		printf(GRN "Height: " RESET "%f\n", layout.height);
	}
	else {
		printf(GRN "Height: " RESET "%s\n", 
			size_type_string[layout.height_type]);
	}
	print_indent();
	printf(GRN "Margin: " RESET "%d, %d, %d, %d\n", layout.margin_left, 
		layout.margin_right, layout.margin_top, layout.margin_bottom);
	print_indent();
	printf(GRN "Padding: " RESET "%d, %d, %d, %d\n", layout.padding_left, 
		layout.padding_right, layout.padding_top, layout.padding_bottom);
	print_indent();
	printf(GRN "Position: " RESET "%f, %f\n", layout.x, layout.y);

	indentation--;
}

void print_view(view* tree) {
	if (!tree) return;
	print_indent();
	printf(YEL);
    if (tree->type == TYPE_LINEAR_LAYOUT) {
		printf("LinearLayout\n" RESET);
		indentation++;
		print_layout_params(tree->layout);
		print_indent();
		printf(GRN "Orientation: " RESET "%s\n", 
			orientation_string[tree->properties.linear_layout.orientation]);
		print_viewlist(tree->properties.linear_layout.children);
		indentation--;
    }
    else if (tree->type == TYPE_FRAME_LAYOUT) {
		printf("FrameLayout\n" RESET);
		indentation++;
		print_layout_params(tree->layout);
		print_viewlist(tree->properties.frame_layout.children);
		indentation--;
    }
    else if (tree->type == TYPE_TEXT_VIEW) {
		printf("TextView\n" RESET);
		indentation++;
		print_layout_params(tree->layout);
		print_indent();
		printf(GRN "Size: " RESET "%d\n", tree->properties.text_view.size);
		print_indent();
		printf(GRN "Style: " RESET "%s\n", 
			style_string[tree->properties.text_view.style]);
		print_indent();
		printf(GRN "Font: " RESET "%s\n", 
			HPDF_Font_GetFontName(tree->properties.text_view.font));
		print_indent();
		printf(GRN "Text: " RESET "%s\n", tree->properties.text_view.text);
		print_indent();
		printf(GRN "Color: " RESET "TBD\n");
		indentation--;
    }
    else if (tree->type == TYPE_IMAGE_VIEW) {
		printf("ImageView\n" RESET);
		indentation++;
		print_layout_params(tree->layout);
		print_indent();
		printf(GRN "Scale-Type: " RESET "%s\n", 
			scale_type_string[tree->properties.image_view.scale_type]);
		print_indent();
		printf(GRN "Src: " RESET "%s\n", tree->properties.image_view.src);		
		indentation--;
	}
	printf(RESET);
}

void free_viewlist(viewlist* list) {
	if (!list) return;
	free_view(list->elem);
	free_viewlist(list->next);
	free(list);
}
	
void free_view(view* tree) {
	if (!tree) return;
    if (tree->type == TYPE_LINEAR_LAYOUT) {
		free_viewlist(tree->properties.linear_layout.children);
    }
    else if (tree->type == TYPE_FRAME_LAYOUT) {
		free_viewlist(tree->properties.frame_layout.children);
	}
	else if (tree->type == TYPE_TEXT_VIEW) {
		free(tree->properties.text_view.text);
	}
	else if (tree->type == TYPE_IMAGE_VIEW) {
		free(tree->properties.image_view.src);
	}
	free(tree);
}
