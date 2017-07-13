#ifndef VERIFY_H
#define VERIFY_H

#include <libxml/tree.h>
// view.h: interface for a View object as well as a ViewGroup object
//   Provides options to build a ViewTree from a XML document, as well as
//   freeing the ViewTree
 
typedef enum size_type {
	SIZE_MATCHPARENT,
	SIZE_WRAPCONTENT,
	SIZE_EXACT
} size_type;

typedef struct layoutparams {
    size_type width_type;
    size_type height_type;
    int width;
    int height;
    int margin_left;
    int margin_right;
    int margin_top;
    int margin_bottom;
    int padding_left;
    int padding_right;
    int padding_top;
    int padding_bottom;
} layoutparams;

typedef struct view {
	enum {
		TEXTVIEW,
		IMAGEVIEW
	} type;
	int size;
	enum {
		STYLE_NONE,
		STYLE_BOLD,
		STYLE_ITALICS,
		STYLE_BOLDITALICS
	} style;
	char *text;
	layoutparams layout;
} view;

typedef struct viewlist {
	view *elem;
	struct viewlist *next;
} viewlist;

typedef struct viewgroup {
	enum {
		FRAMELAYOUT,
		LINEARLAYOUT
	} type;
	enum {
	    ORIENTATION_VERTICAL,
		ORIENTATION_HORIZONTAL
	} orientation;
	layoutparams layout;
	viewlist *children;
} viewgroup;

// build_viewtree(rootnode) builds a ViewTree from the page node representing
//   the page.
viewgroup *build_viewtree(xmlNode *pagenode);

// free_viewtree(viewgroup) frees the allocated memory
void free_viewtree(viewgroup *tree);

#endif