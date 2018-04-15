#include "templates.h"
#include "libxml/tree.h"
#include "error.h"
#include "print.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define XML_NAME 					(const xmlChar*) "name"

template_node* templates = NULL;

bool template_exist(const char* name) {
	template_node* curr = templates;
	while (curr) {
		if (strcmp(curr->name, name) == 0) {
			return true;
		}
		curr = curr->next;
	}
	return false;
}

void add_template(xmlNode* node, int line) {
	long element_count = xmlChildElementCount(node);	
	if (element_count != 1) {
		error(line, TEMPLATE_MUST_HAVE_ONLY_ONE, element_count);
	}
	xmlChar* name = xmlGetProp(node, XML_NAME);
	if (!name) {
		error(line, TEMPLATE_NO_NAME_ATTR);
	}
	if (template_exist(name)) {
		error(line, TEMPLATE_ALREADY_EXISTS, name);
	}
	print_status("Adding Template: %s...\n", name);
	template_node* new_node = malloc(sizeof(template_node));
	new_node->name = name;
	new_node->node = xmlFirstElementChild(node);
	new_node->next = templates;
	templates = new_node;
}


xmlNode* get_template(const xmlChar* name, int line) {
	template_node* curr = templates;
	while (curr) {
		if (strcmp(curr->name, name) == 0) {
			return curr->node;
		}
		curr = curr->next;
	}
	error(line, TEMPLATE_NOT_FOUND, name);
	return NULL;
}

void free_templates_ll() {
	template_node* curr = templates;
	while (curr) {
		template_node* next = curr->next;
		free(curr->name);
		free(curr);
		curr = next;
	}
}
