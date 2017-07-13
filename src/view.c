#include "view.h"
#include "libxml/tree.h"
#include "error.h"

// view.c implements the building of the ViewTree from a pageNode

static void
print_element_names(xmlNode * a_node) {
	xmlNode *cur_node = NULL;
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

view *build_view(xmlNode *rootnode) {
	
}

viewgroup *build_viewgroup(xmlNode *rootnode) {

}

viewgroup *build_viewtree(xmlNode *pagenode) {
	// Node is a Page Node
	//pagenode = pagenode->children;
	
	
	xmlNode *cur_node = NULL;
	for (cur_node = pagenode; cur_node; cur_node = cur_node->next) {
		printf("Type %d\n", pagenode->type);
		print_element_names(cur_node);
		if (cur_node->type != XML_ELEMENT_NODE) {
		    error(cur_node->line, EXPECTED_ELEMENT_IN_PAGE);
		}
		if (!xmlStrcmp(cur_node->name, (const xmlChar *) "FrameLayout") 
			&& !xmlStrcmp(cur_node->name, (const xmlChar *) "LinearLayout")) {
				error(cur_node->line, PAGE_MUST_START_VIEWGROUP);
		}
	}
}


void free_viewtree(viewgroup *tree);
