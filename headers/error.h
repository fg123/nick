#ifndef ERROR_H
#define ERROR_H

#define PAGE_MUST_START_VIEWGROUP "Page must begin with ViewGroup element."
#define EXPECTED_ELEMENT_IN_PAGE "Expected ELEMENT_NODE in Page but got %d!"
#define EXPECTED_ELEMENT_IN_VIEWGROUP "Expected ELEMENT_NODE in ViewGroup but got %d!"
#define EXPECTED_ELEMENT_IN_DOCUMENT "Expected ELEMENT_NODE in Document but got %d!"
#define PAGE_MUST_HAVE_ONLY_ONE "Page must have only one child. Found %d."
#define TEMPLATE_MUST_HAVE_ONLY_ONE "Template must have only one child. Found %d."
#define UNRECOGNIZED_ELEMENT "Encountered unrecognized element '%s'!"
#define FONT_NOT_FOUND "Font %s is not found!"
#define FONT_STYLE_NOT_FOUND "Font %s found but did not find %s style."
#define FONT_STYLE_ALREADY_LOADED "Font %s with style %s already loaded!"
#define FONT_REQUIRES_NAME "Font tag requires name attribute!"
#define INVALID_GRAVITY "Invalid gravity value: %s"
#define TEMPLATE_NO_NAME_ATTR "Templates must have a name attribute!"
#define TEMPLATE_ALREADY_EXISTS "Template %s already exists!"
#define TEMPLATE_NOT_FOUND "Template %s was not found!"
#define INVALID_IMAGE_EXTENSION "Image %s must be JPEG or PNG"

void error(int line, char *message, ...);

#endif
