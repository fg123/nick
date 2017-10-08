#ifndef TEMPLATES_H
#define TEMPLATES_H

#include "libxml/tree.h"
#include <stdbool.h>

typedef struct template_node {
	char* name;
	xmlNode* node;
	struct template_node* next;
} template_node;

bool template_exist(const char* name);

void add_template(xmlNode* node, int line);

xmlNode* get_template(const xmlChar* name, int line);

void free_templates_ll();

#endif