#ifndef COLOR_H
#define COLOR_H

#define COLOR_CONTENT 	stoc("#89b4c0")
#define COLOR_PADDING 	stoc("#c1cf87")
#define COLOR_MARGIN	stoc("#f9cc9a")


typedef struct color {
	float red;
	float green;
	float blue;
} rgb;

rgb stoc(const char* str);

#endif