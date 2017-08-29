#include <time.h>
#include <string.h>
#include <stdio.h>

#include "marker-utils.h"

char*
marker_combo_box_get_active_str(GtkComboBox* combo_box)
{
    GtkTreeIter iter;
    if (gtk_combo_box_get_active_iter(combo_box, &iter))
    {
        GtkTreeModel* model = NULL;
        model = gtk_combo_box_get_model(combo_box);
        GValue value = G_VALUE_INIT;
        if (model)
        {
            gtk_tree_model_get_value(model, &iter, 0, &value);
            return g_value_get_string(&value);
        }
    }
    return NULL;
}

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

int
marker_utils_str_ends_with(char* str,
                           char* sub_str)
{
    size_t str_len = strlen(str);
    size_t sub_len = strlen(sub_str);
    if (memcmp(&str[str_len - sub_len], sub_str, sub_len) == 0)
    {
        return 1;
    }
    return 0;
}

