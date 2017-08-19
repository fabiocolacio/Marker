#include <time.h>

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

int
marker_utils_get_current_time_seconds()
{
    time_t timer;
    struct tm y2k = {0};
    double seconds;

    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

    time(&timer);  /* get current time; same as: timer = time(NULL)  */

    seconds = difftime(timer,mktime(&y2k));
    
    return seconds;
}

