#include "view.h"
#include "libxml/tree.h"
#include "libpdf/hpdf.h"
#include "error.h"
#include "pdf.h"
#include "util.h"
#include <stdbool.h>
#include <string.h>
#include "templates.h"

// view.c implements the building of the ViewTree from a PageNode

// XML String Constants
#define XML_FRAME_LAYOUT 			(const xmlChar*) "FrameLayout"
#define XML_LINEAR_LAYOUT 			(const xmlChar*) "LinearLayout"
#define XML_TEXT_VIEW 				(const xmlChar*) "TextView"
#define XML_IMAGE_VIEW 				(const xmlChar*) "ImageView"
#define XML_CONTENT 				(const xmlChar*) "Content"

#define XML_LAYOUT_WIDTH 			(const xmlChar*) "width"
#define XML_LAYOUT_HEIGHT 			(const xmlChar*) "height"

#define XML_LAYOUT_SIZE_AUTO 		(const xmlChar*) "auto"
#define XML_LAYOUT_SIZE_FILL		(const xmlChar*) "fill"

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
#define XML_LAYOUT_BORDER 			(const xmlChar*) "border"
#define XML_LAYOUT_BORDER_TOP 		(const xmlChar*) "border-top"
#define XML_LAYOUT_BORDER_BOTTOM 	(const xmlChar*) "border-bottom"
#define XML_LAYOUT_BORDER_LEFT 		(const xmlChar*) "border-left"
#define XML_LAYOUT_BORDER_RIGHT 	(const xmlChar*) "border-right"
#define XML_LAYOUT_BORDER_COLOR		(const xmlChar*) "border-color"
#define XML_GRAVITY 				(const xmlChar*) "gravity"

#define XML_GRAVITY_LEFT 			(const xmlChar*) "left"
#define XML_GRAVITY_RIGHT 			(const xmlChar*) "right"
#define XML_GRAVITY_BOTTOM 			(const xmlChar*) "bottom"
#define XML_GRAVITY_TOP	 			(const xmlChar*) "top"
#define XML_GRAVITY_CENTER 			(const xmlChar*) "center"
#define XML_GRAVITY_CENTER_H		(const xmlChar*) "center-horizontal"
#define XML_GRAVITY_CENTER_V		(const xmlChar*) "center-vertical"

// XML Properties
#define XML_ORIENTATION				(const xmlChar*) "orientation"
#define XML_ORIENTATION_VERTICAL 	(const xmlChar*) "vertical"
#define XML_ORIENTATION_HORIZONTAL 	(const xmlChar*) "horizontal"
#define XML_TEXT					(const xmlChar*) "text"
#define XML_FONT					(const xmlChar*) "font"
#define XML_TEXT_SIZE				(const xmlChar*) "size"
#define XML_TEXT_COLOR				(const xmlChar*) "color"
#define XML_TEXT_STYLE				(const xmlChar*) "style"
#define XML_TEXT_STYLE_NONE			(const xmlChar*) "none"
#define XML_TEXT_STYLE_BOLD			(const xmlChar*) "bold"
#define XML_TEXT_STYLE_ITALIC		(const xmlChar*) "italic"
#define XML_TEXT_STYLE_BOLDITALIC	(const xmlChar*) "bolditalic"
#define XML_ALIGN 					(const xmlChar*) "align"
#define XML_ALIGN_LEFT 				(const xmlChar*) "left"
#define XML_ALIGN_RIGHT 			(const xmlChar*) "right"
#define XML_ALIGN_CENTER	 		(const xmlChar*) "center"
#define XML_ALIGN_JUSTIFY	 		(const xmlChar*) "justify"
#define XML_SRC						(const xmlChar*) "src"
#define XML_SCALE_TYPE				(const xmlChar*) "scale-type"
#define XML_SCALE_TYPE_CENTER		(const xmlChar*) "center"
#define XML_SCALE_TYPE_CROP			(const xmlChar*) "center-crop"
#define XML_SCALE_TYPE_INSIDE		(const xmlChar*) "center-inside"
#define XML_SCALE_TYPE_FIT_CENTER	(const xmlChar*) "fit-center"

static int indentation = 0;

// Forward Declarations
static view *build_view(xmlNode*, xmlNode*);
static view *build_viewgroup(xmlNode*, xmlNode*);

static bool is_viewgroup(const xmlNode* node) {
	return xmlStrEqual(node->name, XML_FRAME_LAYOUT)
	|| xmlStrEqual(node->name, XML_LINEAR_LAYOUT);
}

static bool is_view(const xmlNode* node) {
	return xmlStrEqual(node->name, XML_TEXT_VIEW)
	|| xmlStrEqual(node->name, XML_IMAGE_VIEW);
}

static void fill_in_substitution(char** str_p, int start, int end, 
	char* replace_with) {
	// Example: "Hello $testing whee" replace "Felix"
	//           start-^       ^-end
	// o_l = 19
	// n_l = 5
	// start = 6
	// end = 14
	int sub_length = end - start;
	int o_length = strlen(*str_p);
	int n_length = strlen(replace_with);
	(*str_p)[start] = 0;
	int final_length = o_length - sub_length + n_length;
	char* old = *str_p;
	*str_p = malloc((final_length + 1) * sizeof(char));
	(*str_p)[0] = 0;
	strcat(*str_p, old);
	strcat(*str_p, replace_with);
	strcat(*str_p, &old[end]);
	free(old);
}

static void fill_template_substitutions(xmlNode* node, xmlNode* template_base) {
	if (!node) return;
	for (xmlAttr* attr = node->properties; attr; attr = attr->next) {
		char* value = xmlGetProp(node, attr->name);
		if (!value) {
			fprintf(stderr, "Internal Error: %s attr not found...\n", 
				attr->name);
			exit(1);
		}
		for (int i = 0; value[i]; i++) {
			if (value[i] == '$' && (i == 0 || value[i - 1] != '\\')) {
				// Perform Substitution
				int start = i;
				while (value[i] != ' ' && value[i] != 0) {
					i++;
				}
				int end = i;
				char old = value[end];
				// Make a fake string pointer ending at the name of the var
				value[end] = 0;
				char* replace_with = xmlGetProp(template_base, &value[start + 1]);
				value[end] = old;
				if (replace_with) {
					i = start + strlen(replace_with) - 1;
					fill_in_substitution(&value, start, end, replace_with);
					free(replace_with);
				}
				else {
					i = start - 1;
					fill_in_substitution(&value, start, end, "");
				}
			}
		}
		xmlSetProp(node, attr->name, value);
		free(value);
	}
	// Shouldn't this be refactored to only check for itself? Idk why it broke
	//  but OK ...
	for (xmlNode* curr = node->children; curr; curr = curr->next) {
		if (curr->type == XML_ELEMENT_NODE && 
				xmlStrEqual(curr->name, XML_CONTENT)) {
			xmlNode* content_node_p = curr;
			// Link in each content from template_base
			xmlNode* child = template_base->children;
			while (child) {
				if (child->type == XML_ELEMENT_NODE) {
					xmlNode* child_dup = xmlCopyNode(child, 1);
					curr = xmlAddNextSibling(curr, child_dup);
					//xmlFreeNode(child_dup);
				}
				child = child->next;
			}
			xmlUnlinkNode(content_node_p);
			xmlFreeNode(content_node_p);
		}
	}
}

static gravity_type get_gravity_type(char* string, int line) {
	string = trim(string);
	if (strcmp(string, XML_GRAVITY_LEFT) == 0) {
		return 0;
	}
	else if (strcmp(string, XML_GRAVITY_RIGHT) == 0) {
		return GRAVITY_RIGHT;
	}
	else if (strcmp(string, XML_GRAVITY_TOP) == 0) {
		return 0;
	}
	else if (strcmp(string, XML_GRAVITY_BOTTOM) == 0) {
		return GRAVITY_BOTTOM;
	}
	else if (strcmp(string, XML_GRAVITY_CENTER) == 0) {
		return GRAVITY_CENTER;
	}
	else if (strcmp(string, XML_GRAVITY_CENTER_H) == 0) {
		return GRAVITY_CENTER_HORIZONTAL;
	}
	else if (strcmp(string, XML_GRAVITY_CENTER_V) == 0) {
		return GRAVITY_CENTER_VERTICAL;
	}
	else {
		error(line, INVALID_GRAVITY, string);
	}
}

static view_properties get_frame_layout_properties(const xmlNode* node, 
		viewlist* child_list) {
	view_properties properties;
	// Setup Default Values
	properties.frame_layout.children = child_list;
	return properties;
}

static view_properties get_text_view_properties(const xmlNode* node) {
	view_properties properties;
	// Setup Default Values
	properties.text_view.size = 14;
	properties.text_view.style = STYLE_NONE;
	properties.text_view.text = NULL;
	properties.text_view.align = ALIGN_LEFT;
	properties.text_view.color = stoc("#000000");

	// Grab Properties
	xmlChar* text_size = xmlGetProp(node, XML_TEXT_SIZE);
	xmlChar* font = xmlGetProp(node, XML_FONT);
	xmlChar* text_style = xmlGetProp(node, XML_TEXT_STYLE);
	xmlChar* text = xmlGetProp(node, XML_TEXT);
	xmlChar* align = xmlGetProp(node, XML_ALIGN);
	xmlChar* color = xmlGetProp(node, XML_TEXT_COLOR);

	if (text_size) {
		properties.text_view.size = atof(text_size);
		free(text_size);
	}
	if (color) {
		properties.text_view.color = stoc(color);
		free(color);
	}
	if (text) {
		properties.text_view.text = format_special_characters(text);
		free(text);
	}
	else {
		properties.text_view.text = strdup("");
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
	if (align) {
		if (xmlStrEqual(align, XML_ALIGN_LEFT)) {
			properties.text_view.align = ALIGN_LEFT;
		}
		else if (xmlStrEqual(align, XML_ALIGN_RIGHT)) {
			properties.text_view.align = ALIGN_RIGHT;
		}
		else if (xmlStrEqual(align, XML_ALIGN_CENTER)) {
			properties.text_view.align = ALIGN_CENTER;
		}
		else if (xmlStrEqual(align, XML_ALIGN_JUSTIFY)) {
			properties.text_view.align = ALIGN_JUSTIFY;
		}
		else {
			// Maybe Error Handle, but we can just ignore invalid values	
		}
		free(align);
	}
	if (font) {
		properties.text_view.font = 
			get_font(font, properties.text_view.style, node->line);
		free(font);
	}
	else {
		properties.text_view.font = 
			get_font("Helvetica", properties.text_view.style, node->line);
	}
	return properties;
}

static view_properties get_image_view_properties(const xmlNode* node) {
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

static view_properties get_linear_layout_properties(const xmlNode* node, 
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

static layout_params get_layout_params(const xmlNode* node) {
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
	layout.border_left = 0;
	layout.border_right = 0;
	layout.border_top = 0;
	layout.border_bottom = 0;
	layout.border_color = stoc("#000000");
	layout.width_type = SIZE_AUTO;
	layout.height_type = SIZE_AUTO;
	layout.x = 0;
	layout.y = 0;
	layout.gravity = 0;

	// Width and Height are Required
	xmlChar* width = xmlGetProp(node, XML_LAYOUT_WIDTH);
	xmlChar* height = xmlGetProp(node, XML_LAYOUT_HEIGHT);
	xmlChar* margin = xmlGetProp(node, XML_LAYOUT_MARGIN);
	xmlChar* margin_top = xmlGetProp(node, XML_LAYOUT_MARGIN_TOP);
	xmlChar* margin_bottom = xmlGetProp(node, XML_LAYOUT_MARGIN_BOTTOM);
	xmlChar* margin_left = xmlGetProp(node, XML_LAYOUT_MARGIN_LEFT);
	xmlChar* margin_right = xmlGetProp(node, XML_LAYOUT_MARGIN_RIGHT);
	xmlChar* border = xmlGetProp(node, XML_LAYOUT_BORDER);
	xmlChar* border_top = xmlGetProp(node, XML_LAYOUT_BORDER_TOP);
	xmlChar* border_bottom = xmlGetProp(node, XML_LAYOUT_BORDER_BOTTOM);
	xmlChar* border_left = xmlGetProp(node, XML_LAYOUT_BORDER_LEFT);
	xmlChar* border_right = xmlGetProp(node, XML_LAYOUT_BORDER_RIGHT);
	xmlChar* border_color = xmlGetProp(node, XML_LAYOUT_BORDER_COLOR);
	xmlChar* padding = xmlGetProp(node, XML_LAYOUT_PADDING);
	xmlChar* padding_top = xmlGetProp(node, XML_LAYOUT_PADDING_TOP);
	xmlChar* padding_bottom = xmlGetProp(node, XML_LAYOUT_PADDING_BOTTOM);
	xmlChar* padding_left = xmlGetProp(node, XML_LAYOUT_PADDING_LEFT);
	xmlChar* padding_right = xmlGetProp(node, XML_LAYOUT_PADDING_RIGHT);
	xmlChar* gravity = xmlGetProp(node, XML_GRAVITY);

	if (width) {
		if (xmlStrEqual(width, XML_LAYOUT_SIZE_FILL)) {
			layout.width_type = SIZE_FILL;
		}
		else if (xmlStrEqual(width, XML_LAYOUT_SIZE_AUTO)) {
			layout.width_type = SIZE_AUTO;
		}
		else {
			layout.width = atof(width);
			layout.width_type = SIZE_EXACT;
		}
		free(width);
	}

	if (height) {
		if (xmlStrEqual(height, XML_LAYOUT_SIZE_FILL)) {
			layout.height_type = SIZE_FILL;
		}
		else if (xmlStrEqual(height, XML_LAYOUT_SIZE_AUTO)) {
			layout.height_type = SIZE_AUTO;
		}
		else {
			layout.height = atof(height);
			layout.height_type = SIZE_EXACT;
		}
		free(height);
	}

	// Specific Margin Definitions are prioritized
	if (margin) {
		layout.margin_left = layout.margin_right =
		layout.margin_bottom = layout.margin_top = atof(margin);
		free(margin);
	}
	if (padding) {
		layout.padding_left = layout.padding_right =
		layout.padding_bottom = layout.padding_top = atof(padding);
		free(padding);
	}
	if (border_color) {
		layout.border_color = stoc(border_color);
		free(border_color);
	}
	if (border) {
		layout.border_left = layout.border_right =
		layout.border_bottom = layout.border_top = atof(border);
		free(border);
	}
	if (border_left) {
		layout.border_left = atof(border_left);
		free(border_left);
	}
	if (border_right) {
		layout.border_right = atof(border_right);
		free(border_right);
	}
	if (border_top) {
		layout.border_top = atof(border_top);
		free(border_top);
	}
	if (border_bottom) {
		layout.border_bottom = atof(border_bottom);
		free(border_bottom);
	}
	if (margin_left) {
		layout.margin_left = atof(margin_left);
		free(margin_left);
	}
	if (margin_right) {
		layout.margin_right = atof(margin_right);
		free(margin_right);
	}
	if (margin_top) {
		layout.margin_top = atof(margin_top);
		free(margin_top);
	}
	if (margin_bottom) {
		layout.margin_bottom = atof(margin_bottom);
		free(margin_bottom);
	}
	if (padding_left) {
		layout.padding_left = atof(padding_left);
		free(padding_left);
	}
	if (padding_right) {
		layout.padding_right = atof(padding_right);
		free(padding_right);
	}
	if (padding_top) {
		layout.padding_top = atof(padding_top);
		free(padding_top);
	}
	if (padding_bottom) {
		layout.padding_bottom = atof(padding_bottom);
		free(padding_bottom);
	}
	if (gravity) {
		char* token;
		// Separate by Pipe, will destroy the string
		token = strtok(gravity, "|");
		while (token) {			
		   layout.gravity |= get_gravity_type(token, node->line);
		   token = strtok(NULL, "|");
		}
		free(gravity);
	}
	if (!layout.gravity) {
		layout.gravity = default_gravity;
	}
	return layout;
}

static view_type get_view_type(const xmlNode* node) {
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

static view* build_view(xmlNode* node, xmlNode* template_base) {
	if (template_base) {
		fill_template_substitutions(node, template_base);
	}
	if (is_viewgroup(node)) {
		return build_viewgroup(node, template_base);
	}
	if (is_view(node)) {
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
	if (template_exist(node->name)) {
		// xmlCopyNode so modifications are built properly
		xmlNode* new_copy = xmlCopyNode(get_template(node->name, node->line), 1);
		view* v = build_view(new_copy, node);
		xmlFreeNode(new_copy);
		return v;
	}
	error(node->line, UNRECOGNIZED_ELEMENT, node->name);
	return NULL;
}

// Given an XML ViewGroup Node
static view* build_viewgroup(xmlNode* node, xmlNode* template_base) {
	viewlist* child_list = NULL;
	viewlist** head_ptr = &child_list;
	for (xmlNode* curr_child = node->children; 
		 curr_child; 
		 curr_child = curr_child->next) {
		if (xmlIsBlankNode(curr_child)) continue;
		if (curr_child->type == XML_ELEMENT_NODE) {
			/*error(curr_child->line, EXPECTED_ELEMENT_IN_VIEWGROUP, 
				curr_child->type);*/
		
			viewlist* new_child = malloc(sizeof(viewlist));
			new_child->elem = build_view(curr_child, template_base);
			new_child->next = NULL;
			*head_ptr = new_child;
			head_ptr = &new_child->next;
		}
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
	return build_view(root_child, NULL);
}

static void print_indent() {
    char a[50] = {0};
    for (int i = 0; i < indentation; i++) {
        strcat(a, "| ");
    }
    strcat(a, "`-");
    printf("%s", a);
}

static void print_viewlist(viewlist* list) {
	if (!list) return;
	print_indent();
	printf(CYN "<ViewList Item>\n" RESET);
	indentation++;
	print_view(list->elem);
	indentation--;
	print_viewlist(list->next);
}

static void print_layout_params(layout_params layout) {
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
	printf(GRN "Margin: " RESET "%f, %f, %f, %f\n", layout.margin_left, 
		layout.margin_right, layout.margin_top, layout.margin_bottom);
	print_indent();
	printf(GRN "Border: " RESET "%f, %f, %f, %f\n", layout.border_left, 
		layout.border_right, layout.border_top, layout.border_bottom);
	print_indent();
	printf(GRN "Padding: " RESET "%f, %f, %f, %f\n", layout.padding_left, 
		layout.padding_right, layout.padding_top, layout.padding_bottom);
	print_indent();
	printf(GRN "Position: " RESET "%f, %f\n", layout.x, layout.y);

	print_indent();
	printf(GRN "Gravity: " RESET "0x%08X\n", layout.gravity);

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
		/*print_indent();
		printf(GRN "Code: " RESET);
		char* s = tree->properties.text_view.text;
		while(*s) {
			printf("%02x ", (unsigned char) *s++);
		}
		printf("\n");*/
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

static void free_viewlist(viewlist* list) {
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
