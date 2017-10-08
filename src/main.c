#include <stdio.h>
#include <string.h>
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "view.h"
#include "libpdf/hpdf.h"
#include "layout.h"
#include "pdf.h"
#include "draw.h"
#include "util.h"
#include "fonts.h"
#include "error.h"
#include "templates.h"

#ifdef LIBXML_TREE_ENABLED

// XML String Constants
#define XML_FONT		 			(const xmlChar*) "Font"
#define XML_FONT_REGULAR			(const xmlChar*) "regular"
#define XML_FONT_BOLD				(const xmlChar*) "bold"
#define XML_FONT_ITALIC				(const xmlChar*) "italic"
#define XML_FONT_BOLDITALIC			(const xmlChar*) "bolditalic"
#define XML_NAME					(const xmlChar*) "name"
#define XML_PAGE		 			(const xmlChar*) "Page"
#define XML_TEMPLATE	 			(const xmlChar*) "Template"

// Defining from pdf.h
HPDF_Doc pdf;

void usage() {
	printf("Usage: nick [file]\n");
	printf("	file: a valid Nick XML Layout File\n");
}

void error_handler (HPDF_STATUS   error_no,
	HPDF_STATUS   detail_no,
	void         *user_data)
{
	printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
		(HPDF_UINT)detail_no);
	exit(1);
}

void process_options(char** options, int len) {
    for (int i = 0; i < len; i++) {
		if (strcmp("-b", options[i]) == 0 ||  
			strcmp("--show-bounding-box", options[i]) == 0) {
            set_settings_flag(SETTINGS_SHOW_BOUNDING_BOX);
		}
        else {
			usage();
			exit(1);
        }
    }
}

int main(int argc, char **argv) {
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	if (argc == 1) {
		usage();
		return(1);
	}

	LIBXML_TEST_VERSION

	doc = xmlReadFile(argv[1], NULL, 0);

	if (doc == NULL) {
		fprintf(stderr, "error: could not parse file %s\n", argv[1]);
		return 1;
	}

	process_options(&argv[2], argc - 2);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	if (xmlStrcmp(root_element->name, (const xmlChar *) "Document")) {
		fprintf(stderr, "Document of the wrong type, your root node must be a 'Document' object!\n");
		return 1;
	}
	pdf = HPDF_New(error_handler, NULL);
	fonts_init();
	if (!pdf) {
		fprintf(stderr, "Cannot create pdf object.\n");
		return 1;
	}
	for (xmlNode* child = root_element->children; child; child = child->next) {
		if (xmlIsBlankNode(child)) continue;
		if (child->type == XML_ELEMENT_NODE) {
				//error(child->line, EXPECTED_ELEMENT_IN_DOCUMENT, child->type);
			
			if (xmlStrEqual(child->name, XML_FONT)) {
				// name src bold regular bolditalic italic
				xmlChar* name = xmlGetProp(child, XML_NAME);
				if (!name) {
					error(child->line, FONT_REQUIRES_NAME);
				}
				xmlChar* regular_path = xmlGetProp(child, XML_FONT_REGULAR);
				xmlChar* bold_path = xmlGetProp(child, XML_FONT_BOLD);
				xmlChar* italic_path = xmlGetProp(child, XML_FONT_ITALIC);
				xmlChar* bolditalic_path = xmlGetProp(child, XML_FONT_BOLDITALIC);

				if (regular_path) {
					put_font(name, STYLE_NONE, regular_path, child->line);
					free(regular_path);
				}
				if (bold_path) {
					put_font(name, STYLE_BOLD, bold_path, child->line);
					free(bold_path);
				}
				if (italic_path) {
					put_font(name, STYLE_ITALIC, italic_path, child->line);
					free(italic_path);
				}
				if (bolditalic_path) {
					put_font(name, STYLE_BOLDITALIC, bolditalic_path, child->line);
					free(bolditalic_path);
				}
				free(name);
			}
			else if (xmlStrEqual(child->name, XML_PAGE)) {
				HPDF_Page page = HPDF_AddPage(pdf);
				HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
				// root_element is DOCUMENT, first child is a page, loop through here
				view* root = build_page(child);
				measure_and_layout(root);
				print_view(root);
			
				draw(root, page);
				free_view(root);
			}
			else if (xmlStrEqual(child->name, XML_TEMPLATE)) {
				add_template(child, child->line);
			}
		}
	}
	free_templates_ll();
	xmlFreeDoc(doc);
	xmlCleanupParser();

	HPDF_SaveToFile(pdf, "test.pdf");
	HPDF_Free(pdf);
	free_fonts_ll();
	return 0;
}
#else
int main(void) {
	fprintf(stderr, "Tree support not compiled in\n");
	exit(1);
}
#endif