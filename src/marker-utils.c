
#include "marker-utils.h"

int
marker_utils_rfind(char query, char* str)
{
    char ch = str[0];
    int pos = -1;
    int index = 0;
    while (ch)
    {
        if (ch == query)
        {
            pos = index;
        }
        ch = str[++index];
    }
    return pos;
}

