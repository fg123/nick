#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static bool settings_data[SETTINGS_COUNT] = { false };

void set_settings_flag(settings_flags flag) {
    settings_data[flag] = true;
}

bool get_settings_flag(settings_flags flag) {
    return settings_data[flag];
}

char* strdup(char* src) {
	char* str;
	char* p;
	size_t len = strlen(src);
	str = malloc(len + 1);
	if (str) {
	  memcpy(str, src, len + 1);
	}
	return str;
}

char* format_special_characters(char* src) {
	int len = strlen(src);
	char* str = malloc(len + 1);
	size_t s = 0;
	if (str) {
		for (int i = 0; i < len - 1; i++) {
			if (src[i] == '\\') {
				switch(src[i + 1]) {
					case 'n':
						str[s++] = '\n';
						i++;
						break;
					case 't':
						str[s++] = '\t';
						i++;
						break;
					default:
						str[s++] = '\\';
				}
			}
			else {
				str[s++] = src[i];
			}
		}
		str[s++] = src[len - 1];
		str[s] = 0;
	}
	return str;
}

int min(int a, int b) {
	return a < b ? a : b;
}

int max(int a, int b) {
	return a > b ? a : b;
}

void insert_newline(char** str, int position) {
	int length = strlen(*str);
	char* new = malloc(length + 2);
	int s = 0;
	if (new) {
		for (int i = 0; i < length; i++) {
			if (i == position) {
				new[s++] = '\n';
			}
			new[s++] = (*str)[i];
		}
		new[s] = 0;
		free(*str);
		*str = new;
	}
}

char* trim(char* str) {
    size_t len = 0;
    char* frontp = str;
    char* endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }


    return str;
}