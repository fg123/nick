#include "error.h"

// Implementation for Error Module

void error(int line, char* message) {
    printf("Error encountered on line %d: %s\n", line, message);
    exit(1);
}