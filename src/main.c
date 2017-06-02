#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <hpdf.h>

#ifdef LIBXML_TREE_ENABLED

static void
print_element_names(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("node type: Element, name: %s\n", cur_node->name);
		}

		print_element_names(cur_node->children);
	}
}

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
	if (xmlStrcmp(root_element->name, (const xmlChar *) "document")) {
		fprintf(stderr,"Document of the wrong type, your root node must be a 'document' object!");
	}
	else {
	    verify_document(root_element);
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