#include "str.h"

#include <string.h>

bool start_with(char *line, char *token) {
    if (strstr(line, token) == line)
        return true;
    return false;
}
