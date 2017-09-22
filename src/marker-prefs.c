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

const char*
marker_prefs_get_syntax_theme()
{
  return prefs.syntax_theme;
}

gboolean
marker_prefs_get_show_line_numbers()
{
  return prefs.show_line_numbers;
}

gboolean
marker_prefs_get_highlight_current_line()
{
  return prefs.highlight_current_line;
}

gboolean
marker_prefs_get_wrap_text()
{
  return prefs.wrap_text;
}

gboolean
marker_prefs_get_show_right_margin()
{
  return prefs.show_right_margin;
}

guint
marker_prefs_get_right_margin_position()
{
  return prefs.right_margin_position;
}

const char*
marker_prefs_get_css_theme()
{
  return prefs.css_theme;
}

gboolean
marker_prefs_get_single_view_mode()
{
  return prefs.single_view_mode;
}

gboolean
marker_prefs_get_client_side_decorations()
{
  return prefs.client_side_decorations;
}

gboolean
marker_prefs_get_gnome_appmenu()
{
  return prefs.gnome_appmenu;
}

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
  prefs.show_line_numbers = state;
  
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
  
  marker_prefs_save();
}

static void
highlight_current_line_toggled(GtkToggleButton* button,
                               gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  prefs.highlight_current_line = state;
  
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
  
  marker_prefs_save();
}

static void
wrap_text_toggled(GtkToggleButton* button,
                  gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  prefs.wrap_text = state;
  
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
  
  marker_prefs_save();
}

static void
show_right_margin_toggled(GtkToggleButton* button,
                          gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  prefs.show_right_margin = state;
  
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
  
  marker_prefs_save();
}

static void
syntax_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);
  marker_string_buffer_set(choice, prefs.syntax_theme, 256);
  
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
  
  marker_prefs_save();
}

static void
css_chosen(GtkComboBox* combo_box,
           gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);
  char path[256];
  if (strcmp(choice, "none") != 0)
  {
    marker_string_prepend(choice, STYLES_DIR, path, sizeof(path));
  }
  memcpy(prefs.css_theme, path, 256);
  free(choice);
  
  GtkApplication* app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  for (GList* item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_EDITOR_WINDOW(item->data))
    {
      MarkerEditorWindow* window = item->data;
      marker_editor_window_refresh_preview(window);
    }
  }
  
  marker_prefs_save();
}

static void
use_single_view_mode_toggled(GtkToggleButton* button,
                             gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  prefs.single_view_mode = state;
  marker_prefs_save();
}

static void
use_gnome_appmenu_toggled(GtkToggleButton* button,
                          gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  prefs.gnome_appmenu = state;
  marker_prefs_save();
  prefs.gnome_appmenu = !state;
}

static void
use_client_side_decorations_toggled(GtkToggleButton* button,
                                    gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  prefs.client_side_decorations = state;
  marker_prefs_save();
  prefs.client_side_decorations = !state;
}

void
marker_prefs_show_window()
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource(
      "/com/github/fabiocolacio/marker/ui/prefs-window.ui");
 
  GList *list = NULL;
  GtkComboBox* combo_box;
  GtkToggleButton* check_button;
  
  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
  list = marker_prefs_get_available_syntax_themes();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  marker_widget_combo_box_set_active_str(combo_box,prefs.syntax_theme, g_list_length(list));
  g_list_free_full(list, free);
  list = NULL;
 
  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
  list = marker_prefs_get_available_stylesheets();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  const char* css = marker_string_rfind(prefs.css_theme, "/");
  if (css)
  {
    marker_widget_combo_box_set_active_str(combo_box, ++css, g_list_length(list));
  }
  g_list_free_full(list, free);
  list = NULL;
   
  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_line_numbers_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.show_line_numbers);
  
  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_right_margin_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.show_right_margin);
  
  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wrap_text_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.wrap_text);
  
  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "highlight_current_line_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.highlight_current_line);
  
  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "use_client_side_decorations_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.client_side_decorations);

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "use_appmenu_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.gnome_appmenu);
  
  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "use_single_view_mode_check_button"));
  gtk_toggle_button_set_active(check_button, prefs.single_view_mode);
  
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
  gtk_builder_add_callback_symbol(builder,
                                  "use_gnome_appmenu_toggled",
                                  G_CALLBACK(use_gnome_appmenu_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "use_client_side_decorations_toggled",
                                  G_CALLBACK(use_client_side_decorations_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "use_single_view_mode_toggled",
                                  G_CALLBACK(use_single_view_mode_toggled));
  gtk_builder_connect_signals(builder, NULL);
  
  g_object_unref(builder);
}

void
marker_prefs_load()
{
  char key[256];
  char val[256];
  memset(key, 0, sizeof(key));
  memset(val, 0, sizeof(val));

  FILE* fp = NULL;
  fp = fopen(CONF_FILE, "r");
  if (fp)
  {
    while(fscanf(fp, "%s %s", key, val) == 2)
    {
      if (strcmp(key, "syntax_theme") == 0)
      {
        memcpy(prefs.syntax_theme, val, 256);
      }
      else
      if (strcmp(key, "css_theme") == 0)
      {
        memcpy(prefs.css_theme, val, 256);
      }
      else
      if (strcmp(key, "show_line_numbers") == 0)
      {
        prefs.show_line_numbers = atoi(val);
      }
      else
      if (strcmp(key, "highlight_current_line") == 0)
      {
        prefs.highlight_current_line = atoi(val);
      }
      else
      if (strcmp(key, "wrap_text") == 0)
      {
        prefs.wrap_text = atoi(val);
      }
      else
      if (strcmp(key, "show_right_margin") == 0)
      {
        prefs.show_right_margin = atoi(val);
      }
      else
      if (strcmp(key, "right_margin_position") == 0)
      {
        prefs.right_margin_position = (unsigned int) atoi(val);
      }
      if (strcmp(key, "single_view_mode") == 0)
      {
        prefs.single_view_mode = atoi(val);
      }
      if (strcmp(key, "client_side_decorations") == 0)
      {
        prefs.client_side_decorations = atoi(val);
      }
      if (strcmp(key, "gnome_appmenu") == 0)
      {
        prefs.gnome_appmenu = atoi(val);
      }
      
      memset(key, 0, sizeof(key));
      memset(val, 0, sizeof(val));
    }
    fclose(fp);
  } 
}

void
marker_prefs_save()
{
  FILE* fp = NULL;
  fp = fopen(CONF_FILE, "w");
  if(fp)
  {
    fprintf(fp, "syntax_theme %s\n", (prefs.syntax_theme) ? prefs.syntax_theme : "none");
    fprintf(fp, "show_line_numbers %d\n", prefs.show_line_numbers);
    fprintf(fp, "highlight_current_line %d\n", prefs.highlight_current_line);
    fprintf(fp, "wrap_text %d\n", prefs.wrap_text);
    fprintf(fp, "show_right_margin %d\n", prefs.show_right_margin);
    fprintf(fp, "css_theme %s\n", (prefs.css_theme) ? prefs.css_theme : "none");
    fprintf(fp, "single_view_mode %d\n", prefs.single_view_mode);
    fprintf(fp, "client_side_decorations %d\n", prefs.client_side_decorations);
    fprintf(fp, "gnome_appmenu %d\n", prefs.gnome_appmenu);
    fclose(fp);
  }
}

