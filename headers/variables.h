#ifndef VARIABLES_H
#define VARIABLES_H

struct variable {
	char* name;
	char* value;
	struct variable* next;
};

void variables_init();

void add_variable(const char* name, const char* value);

const char* get_variable(const char* name);

void variables_destroy();

#endif
