#include "util.h"
#include <string.h>
#include <stdlib.h>

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

char* formatSpecialCharacters(char* src) {
	size_t len = strlen(src);
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