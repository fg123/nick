#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
#include "print.h"
#include "view.h"
#include "variables.h"

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

typedef struct xml_doc_node {
	xmlDoc* elem;
	struct xml_doc_node* next;
} xml_doc_node;

xml_doc_node* xml_docs = NULL;

void usage() {
	printf("Usage: nick [file1 file2 ...] [flags]\n");
	printf("	file: a valid Nick XML Layout File\n");
	printf("	-b, --show-bounding-box: showing layout bounding box\n");
	printf("	-s: silent mode\n");
	printf("	-o file: optional output file\n");
	printf("Nick can also take file contents through stdin.\n");

}

void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
	fprintf(stderr, "Internal HPDF Error: %04X, Detail Error: %u\n", (HPDF_UINT) error_no,
		(HPDF_UINT) detail_no);
	exit(1);
}

bool file_exist (const char* name) {
	return access(name, R_OK) != -1;
}

void process_doc(xmlDoc* doc);

void process_stdin() {
	print_status("Reading from stdin...");
	xmlDoc* doc = NULL;
	doc = xmlReadFd(STDIN_FILENO, "", "UTF-8", 0);
	if (doc == NULL) {
		fprintf(stderr, "Could not read from stdin!\n");
		exit(1);
	}
	process_doc(doc);
}

void process_file(char* filename) {
	print_status("Processing file: %s...\n", filename);
	xmlDoc* doc = NULL;
	doc = xmlReadFile(filename, "UTF-8", 0);
	if (doc == NULL) {
		fprintf(stderr, "Could not parse file!\n");
		exit(1);
	}
	process_doc(doc);
}

void process_doc(xmlDoc* doc) {
	xmlNode* root_element = NULL;

	// Add to LL
	xml_doc_node* doc_node = malloc(sizeof(xml_doc_node));
	doc_node->elem = doc;
	doc_node->next = xml_docs;
	xml_docs = doc_node;

	/* Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	if (xmlStrcmp(root_element->name, (const xmlChar *) "Document")) {
		fprintf(stderr, "Document of the wrong type, your root node must be a 'Document' object!\n");
		exit(1);
	}

	// Only margin is used here
	layout_params document_params = get_layout_params(root_element);

	if (!pdf) {
		fprintf(stderr, "Cannot create pdf object.\n");
		exit(1);
	}

	for (xmlNode* child = root_element->children; child; child = child->next) {
		if (xmlIsBlankNode(child)) continue;
		if (child->type == XML_ELEMENT_NODE) {
			if (xmlStrEqual(child->name, XML_FONT)) {
				xmlChar* name = xmlGetProp(child, XML_NAME);
				if (!name) {
					error(child->line, FONT_REQUIRES_NAME);
				}
				print_status("Loading Font: %s...\n", name);
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
			else if (is_viewgroup_xml(child)) {
				view* root = build_view(child, NULL);
				// TODO(felixguo): Arbitrary Page Height right now
				int pagec = measure_and_layout(root,
					(72.0 * 8.5) - document_params.margin_left - document_params.margin_right,
					(72 * 11) - document_params.margin_top - document_params.margin_bottom,
					document_params.margin_left,
					document_params.margin_top);

				HPDF_Page* pages = malloc(pagec * sizeof(*pages));

				for (int i = 0; i < pagec; i++) {
					pages[i] = HPDF_AddPage(pdf);
					HPDF_Page_SetSize(pages[i], HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
				}
				page_height_no_margin = HPDF_Page_GetHeight(pages[0]) -
					document_params.margin_top - document_params.margin_bottom;

				draw(root, pages, pagec);
				free(pages);
			}
			else if (xmlStrEqual(child->name, XML_PAGE)) {
				print_status("Building Page...\n");
				HPDF_Page page = HPDF_AddPage(pdf);
				HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
				// root_element is DOCUMENT, first child is a page, loop through here
				view* root = build_page(child);
				measure_and_layout(root, HPDF_Page_GetWidth(page), HPDF_Page_GetHeight(page), 0, 0);
				// print_view(root);

				draw(root, &page, 1);
				free_view(root);
			}
			else if (xmlStrEqual(child->name, XML_TEMPLATE)) {
				add_template(child, child->line);
			}
		}
	}
}

bool is_variable_set(const char* arg) {
	while (arg[0]) {
		if (arg[0] == '=') return true;
		arg++;
	}
	return false;
}

int main(int argc, char **argv) {
	LIBXML_TEST_VERSION
	pdf = HPDF_New(error_handler, NULL);
	HPDF_UseUTFEncodings(pdf);
	HPDF_SetCurrentEncoder(pdf, "UTF-8");
	char* write_to_name = NULL;
	variables_init();
	fonts_init();
	bool processed_file = false;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp("-b", argv[i]) == 0 ||
				strcmp("--show-bounding-box", argv[i]) == 0) {
				set_settings_flag(SETTINGS_SHOW_BOUNDING_BOX);
			}
			else if (strcmp("-o", argv[i]) == 0 && i < argc - 1) {
				write_to_name = argv[i + 1];
				i++;
			}
			else if (strcmp("-s", argv[i]) == 0 ||
				strcmp("--silent", argv[i]) == 0) {
				set_settings_flag(SETTINGS_SILENT);
			}
			else {
				usage();
				exit(1);
			}
		}
		else if (file_exist(argv[i])) {
			processed_file = true;
			process_file(argv[i]);
		}
		else if (is_variable_set(argv[i])) {
			char* arg = argv[i];
			size_t j = 0;
			while (arg[j] != '=') {
				j++;
			}
			arg[j] = 0;
			// Replace the equal sign so we can strdup
			add_variable(argv[i], &arg[j + 1]);
			arg[j] = '=';
		}
    }

	if (!processed_file) {
		process_stdin();
	}

	free_templates_ll();

	while (xml_docs) {
		xmlFreeDoc(xml_docs->elem);
		xml_doc_node* next = xml_docs->next;
		free(xml_docs);
		xml_docs = next;
	}

	xmlCleanupParser();
	if (!write_to_name) {
		write_to_name = "out.pdf";
	}
	print_status("Writing to file: %s...\n", write_to_name);
	HPDF_SaveToFile(pdf, write_to_name);
	HPDF_Free(pdf);
	free_fonts_ll();
	variables_destroy();
	print_status("Done!\n");
	return 0;
}
#else
int main(void) {
	fprintf(stderr, "Tree support not compiled in\n");
	exit(1);
}
#endif
