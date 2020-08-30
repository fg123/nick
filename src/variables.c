#include "variables.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct variable* head = NULL;

void variables_init()
{

}

void add_variable(const char* name, const char* value)
{
	printf("%s = %s\n", name, value);
	struct variable* var = malloc(sizeof(struct variable));
	var->name = strdup(name);
	var->value = strdup(value);
	var->next = head;
	head = var;
}

const char* get_variable(const char* name)
{
	struct variable* curr = head;
	while (curr)
	{
		if (strcmp(curr->name, name) == 0)
		{
			return curr->value;
		}
		curr = curr->next;
	}
	return NULL;
}

void variables_destroy()
{
	struct variable* curr = head;
	while (curr)
	{
		free(curr->name);
		free(curr->value);
		struct variable* next = curr->next;
		free(curr);
		curr = next;
	}
	head = NULL;
}
