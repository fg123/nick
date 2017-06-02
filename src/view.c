#include "view.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "common.h"
#include "error.h"

// view.c implements the building of the ViewTree from a pageNode

view *build_view(xmlNode *rootnode) {
	
}

viewgroup *build_viewgroup(xmlNode *rootnode) {

}


viewgroup *build_viewtree(xmlNode *pagenode) {
	// Node is a Page Node
	xmlNode *cur_node = NULL;
	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type != XML_ELEMENT_NODE) {
		    error(cur_node->line, EXPECTED_ELEMENT_IN_PAGE);
		}
		char *node_name = cur_node->name;
		if (!str_equ(node_name, "framelayout") 
			&& !str_equ(node_name, "linearlayout")) {
				error(cur_node->line, PAGE_MUST_START_VIEWGROUP);
		}

		print_element_names(cur_node->children);
	}
	if (cur_node->next) {
		error(cur_node->line, PAGE_MUST_HAVE_ONLY_ONE);
	}
}


void free_viewtree(viewgroup *tree);
