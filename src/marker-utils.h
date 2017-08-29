#ifndef __MARKER_UTILS_H__
#define __MARKER_UTILS_H__

#include <gtk/gtk.h>

int
marker_utils_rfind(char, char*);

int
marker_utils_get_current_time_seconds();

int
marker_utils_str_ends_with(char* str, char* sub_str);

char*
marker_combo_box_get_active_str(GtkComboBox* combo_box);

#endif

