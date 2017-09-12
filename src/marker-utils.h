#ifndef __MARKER_UTILS_H__
#define __MARKER_UTILS_H__

#include <gtk/gtk.h>

void
marker_utils_surround_selection_with(GtkTextBuffer* buffer,
                                     char*          insertion);

int
marker_utils_rfind(char  query,
                   char* str);

int
marker_utils_get_current_time_seconds();

int
marker_utils_str_starts_with(char* str,
                             char* sub_str);

int
marker_utils_str_ends_with(char* str,
                           char* sub_str);

char*
marker_utils_combo_box_get_active_str(GtkComboBox* combo_box);

void
marker_utils_combo_box_set_model(GtkComboBox*  combo_box,
                                 GtkTreeModel* model);
                                 
char*
marker_utils_allocate_string(char* str);

char*
marker_utils_escape_file_path(char* filename);

#endif

