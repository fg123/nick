#ifndef ERROR_H
#define ERROR_H

#define EXPECTED_ELEMENT_IN_PAGE "Expected ELEMENT_NODE in Page!"
#define PAGE_MUST_START_VIEWGROUP "Page must begin with ViewGroup element."
#define PAGE_MUST_HAVE_ONLY_ONE "Page must have only one child."

void error(int line, char *message);

#endif