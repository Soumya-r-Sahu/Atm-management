#include "utils.h"
#include <string.h>

int is_valid_string(const char *str) {
    return str != NULL && strlen(str) > 0;
}