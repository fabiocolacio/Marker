#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "marker.h"
#include "marker-editor-window.h"
#include "marker-widget.h"
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

static void
show_line_numbers_toggled(GtkToggleButton* button,
                          gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_set_show_line_numbers(window, state);
    }
  }
}

static void
highlight_current_line_toggled(GtkToggleButton* button,
                               gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_set_highlight_current_line(window, state);
    }
  }
}

static void
wrap_text_toggled(GtkToggleButton* button,
                  gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_set_wrap_text(window, state);
    }
  }
}

static void
show_right_margin_toggled(GtkToggleButton* button,
                          gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_set_show_right_margin(window, state);
    }
  }
}

static void
syntax_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_set_syntax_theme(window, choice);
    }
  }
  free(choice);
}

static void
css_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);
  char* path = NULL;
  if (strcmp(choice, "none") != 0)
  {
    path = marker_string_prepend(choice, STYLES_DIR);
  }
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_set_css_theme(window, path);
    }
  }
  free(path);
  free(choice);
}

void
marker_prefs_show_window()
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource(
      "/com/github/fabiocolacio/marker/prefs-window.ui");
 
  GList *list = NULL;
  GtkComboBox* combo_box;
  GtkCheckButton* check_button;
  
  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
  list = marker_prefs_get_available_syntax_themes();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  g_list_free_full(list, free);
  list = NULL;
 
  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
  list = marker_prefs_get_available_stylesheets();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  g_list_free_full(list, free);
  list = NULL;
  
  GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "prefs_win"));
  gtk_window_set_position(window, GTK_WIN_POS_CENTER);
  gtk_window_set_keep_above(window, TRUE);
  gtk_widget_show_all(GTK_WIDGET(window));
  
  gtk_builder_add_callback_symbol(builder,
                                  "syntax_chosen",
                                  G_CALLBACK(syntax_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "css_chosen",
                                  G_CALLBACK(css_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "show_line_numbers_toggled", 
                                  G_CALLBACK(show_line_numbers_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "highlight_current_line_toggled", 
                                  G_CALLBACK(highlight_current_line_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "wrap_text_toggled", 
                                  G_CALLBACK(wrap_text_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "show_right_margin_toggled", 
                                  G_CALLBACK(show_right_margin_toggled));
  gtk_builder_connect_signals(builder, NULL);
  
  g_object_unref(builder);
}

