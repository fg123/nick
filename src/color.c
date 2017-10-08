#include <stdio.h>

#include "color.h"
#include "error.h"

static int hcton(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	else {
		printf("Invalid color code occured: %c.\n", c);
		return 0;
	}
}

rgb stoc(char* str) {
	float red = hcton(str[0]) * 16 + hcton(str[1]);
	float green = hcton(str[2]) * 16 + hcton(str[3]);
	float blue = hcton(str[4]) * 16 + hcton(str[5]);
	rgb c = { red / 255, green / 255, blue / 255};
	return c;
}