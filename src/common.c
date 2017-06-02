#include "common.h"
#include <string.h>

bool str_equ(char *a, char *b) {
    for (int i = 0; a[i]; i++) {
		if (!b[i])
		    return false;
		char ac = a[i] < 'a' ? a[i] - 'A' + 'a' : a[i];
		char bc = b[i] < 'a' ? b[i] - 'A' + 'a' : b[i];
		if (ac != bc)
		    return false;
	}
	return true;
}