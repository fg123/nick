#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#define SETTINGS_COUNT 1

char* format_special_characters(char* src);
void insert_newline(char** str, int position);
char* strdup(char* src);
int min(int a, int b);
int max(int a, int b);
char* trim(char* str);

typedef enum {
	SETTINGS_SHOW_BOUNDING_BOX = 0
} settings_flags; 

void set_settings_flag(settings_flags flag);
bool get_settings_flag(settings_flags flag);

// Colors
#ifdef _WIN32

#define RED   ""
#define GRN   ""
#define YEL   ""
#define BLU   ""
#define MAG   ""
#define CYN   ""
#define WHT   ""
#define RESET ""

#else

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#endif
#endif