#include <stdio.h>
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "view.h"
#include <hpdf.h>

#ifdef LIBXML_TREE_ENABLED

void usage() {
	printf("Usage: nick [file]\n");
	printf("	file: a valid Nick XML Layout File\n");
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
		printf("error: could not parse file %s\n", argv[1]);
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	if (xmlStrcmp(root_element->name, (const xmlChar *) "Document")) {
		fprintf(stderr,"Document of the wrong type, your root node must be a 'Document' object!\n");
	}
	else {
		// root_element is DOCUMENT, first child is a page, loop through here
		build_viewtree(root_element->children);
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return 0;
}
#else
int main(void) {
	fprintf(stderr, "Tree support not compiled in\n");
	exit(1);
}
#endif