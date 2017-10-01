#include <stdio.h>
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "view.h"
#include "libpdf/hpdf.h"
#include "layout.h"
#include "pdf.h"
#include "draw.h"

#ifdef LIBXML_TREE_ENABLED

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

int main(int argc, char **argv) {
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;

	if (argc != 2) {
		usage();
		return(1);
	}

	LIBXML_TEST_VERSION

	doc = xmlReadFile(argv[1], NULL, 0);

	if (doc == NULL) {
		fprintf(stderr, "error: could not parse file %s\n", argv[1]);
		return 1;
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	if (xmlStrcmp(root_element->name, (const xmlChar *) "Document")) {
		fprintf(stderr, "Document of the wrong type, your root node must be a 'Document' object!\n");
		return 1;
	}
	pdf = HPDF_New(error_handler, NULL);
	if (!pdf) {
		fprintf(stderr, "Cannot create pdf object.\n");
		return 1;
	}
//////////////////////////////
	HPDF_Page page = HPDF_AddPage(pdf);
	HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
	// root_element is DOCUMENT, first child is a page, loop through here
	view* root = build_page(xmlFirstElementChild(root_element));
	measure_and_layout(root);
	print_view(root);

	draw(root, page);
	free_view(root);
/////////////////////////////

	xmlFreeDoc(doc);
	xmlCleanupParser();

	HPDF_SaveToFile(pdf, "test.pdf");
	HPDF_Free(pdf);
	return 0;
}
#else
int main(void) {
	fprintf(stderr, "Tree support not compiled in\n");
	exit(1);
}
#endif