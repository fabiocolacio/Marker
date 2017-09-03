#include <time.h>
#include <string.h>
#include <stdio.h>

#include "marker-utils.h"

char*
marker_utils_escape_file_path(char* filename)
{
  size_t new_len = (2 * strlen(filename)) + 1;
  char* clean_str = malloc(new_len);
  memset(clean_str, 0, new_len);
  char c;
  for (int i = 0, j = 0; (c = filename[i]) != '\0'; ++i)
  {
    if (c == ' ')
    {
      clean_str[j] = '\\';
      ++j;
    }
    clean_str[j] = c;
    ++j;
  }
  return clean_str;
}

char*
marker_utils_combo_box_get_active_str(GtkComboBox* combo_box)
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
      const char* str = g_value_get_string(&value);
      size_t str_len = strlen(str) + 1;
      char* ret = malloc(str_len);
      memset(ret, 0, str_len);
      memcpy(ret, str, str_len - 1);
      return ret;
    }
  }
  return NULL;
}

void
marker_utils_combo_box_set_model(GtkComboBox*  combo_box,
                                  GtkTreeModel* model)
{
  gtk_combo_box_set_model(combo_box, model);
  GtkCellRenderer* cell_renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box), cell_renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box),
                                 cell_renderer,
                                 "text", 0,
                                 NULL);
  gtk_combo_box_set_active(combo_box, 0);
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

  time(&timer);

  seconds = difftime(timer,mktime(&y2k));
    
  return seconds;
}

int
marker_utils_str_starts_with(char* str,
                             char* sub_str)
{
  size_t sub_len = strlen(sub_str);
  if (memcmp(str, sub_str, sub_len) == 0)
  {
    return 1;
  }
  return 0;
}

char*
marker_utils_allocate_string(char* str)
{
  size_t len = strlen(str);
  char* loc = malloc(len + 1);
  memset(loc, 0, len + 1);
  memcpy(loc, str, len + 1);
  return loc;
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

