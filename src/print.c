#include "print.h"
#include "util.h"
#include <stdarg.h>
#include <stdio.h>

void print_status(char* fmt, ...) {
    va_list valist;
	va_start(valist, fmt);
    if (!get_settings_flag(SETTINGS_SILENT)) {
	    vprintf(fmt, valist);
    }
}
