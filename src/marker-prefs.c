#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "marker-widget-utils.h"
#include "marker-string.h"

#include "marker-prefs.h"

MarkerPrefs prefs;

GList*
marker_prefs_get_available_stylesheets()
{
  GList* list = NULL;
  char* list_item;
  
  DIR* dir;
  struct dirent* ent;
  char* filename;
  if ((dir = opendir(STYLES_DIR)) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      filename = ent->d_name;
      
      if (marker_string_ends_with(filename, ".css"))
      {
        list_item = marker_string_alloc(filename);
        list = g_list_prepend(list, list_item);
      }
    }
  }
  closedir(dir);
  
  list_item = marker_string_alloc("none");
  list = g_list_prepend(list, list_item);
  
  return list;
}

GList*
marker_prefs_get_available_syntax_themes()
{
  GList* list = NULL;
  
  GtkSourceStyleSchemeManager* style_manager =
    gtk_source_style_scheme_manager_get_default();
  const gchar * const * ids =
    gtk_source_style_scheme_manager_get_scheme_ids(style_manager);
    
  for (int i = 0; ids[i] != NULL; ++i)
  {
    const gchar* id = ids[i];
    char* item = marker_string_alloc(id);
    list = g_list_prepend(list, item);
  }
  
  return list;
}

void
marker_prefs_show_window()
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource(
      "/com/github/fabiocolacio/marker/prefs-window.ui");
 
  GList *list = NULL;
  
  GtkComboBox* syntax_chooser =
    GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
  list = marker_prefs_get_available_syntax_themes();
  marker_widget_utils_populate_combo_box_with_strings(syntax_chooser, list);
  g_list_free_full(list, free);
  list = NULL;
 
  GtkComboBox* css_chooser = 
    GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
  list = marker_prefs_get_available_stylesheets();
  marker_widget_utils_populate_combo_box_with_strings(css_chooser, list);
  g_list_free_full(list, free);
  list = NULL;
  
  GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "prefs_win"));
  gtk_window_set_position(window, GTK_WIN_POS_CENTER);
  gtk_window_set_keep_above(window, TRUE);
  gtk_widget_show_all(GTK_WIDGET(window));
  
  g_object_unref(builder);
}

