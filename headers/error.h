#ifndef ERROR_H
#define ERROR_H

#define EXPECTED_ELEMENT_IN_PAGE "Expected ELEMENT_NODE in Page but got %d!"
#define PAGE_MUST_START_VIEWGROUP "Page must begin with ViewGroup element."
#define EXPECTED_ELEMENT_IN_VIEWGROUP "Expected ELEMENT_NODE in ViewGroup but got %d!"
#define PAGE_MUST_HAVE_ONLY_ONE "Page must have only one child. Found %d."
#define UNRECOGNIZED_ELEMENT "Encountered unrecognized element '%s'!"

void error(int line, char *message, ...);

#endif