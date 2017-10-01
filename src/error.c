#define _GNU_SOURCE 
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Implementation for Error Module

void error(int line, char* message, ...) {
	va_list valist;
	va_start(valist, message);
	char *error_string;
	vasprintf(&error_string, message, valist);
	fprintf(stderr, "Error encountered on line %d: %s\n", line, error_string);
	free(error_string);
	exit(1);
}