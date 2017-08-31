#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "marker-prefs.h"
#include "marker-editor-window.h"
#include "marker-utils.h"

MarkerPrefs prefs;

static void
css_chosen(GtkComboBox* combo_box,
           gpointer     user_data)
{
  GtkApplication* app = user_data;
  char* css_theme = marker_utils_combo_box_get_active_str(combo_box);
    
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      marker_editor_window_set_css_theme(window, css_theme);
    }
  }
  
  free(css_theme);
}

static void
syntax_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
  GtkApplication* app = user_data;
  char* syntax_theme = marker_utils_combo_box_get_active_str(combo_box);
    
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      marker_editor_window_set_syntax_theme(window, syntax_theme);
    }
  }
    
  free(syntax_theme);
}

static void
show_line_numbers_toggled(GtkToggleButton* toggler,
                          gpointer         user_data)
{
  GtkApplication* app = user_data;
  
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      gboolean toggled = gtk_toggle_button_get_active(toggler);
      marker_editor_window_set_show_line_numbers(window, toggled);
    }
  }
}

static void
highlight_current_line_toggled(GtkToggleButton* toggler,
                               gpointer         user_data)
{
  GtkApplication* app = user_data;
  
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      gboolean toggled = gtk_toggle_button_get_active(toggler);
      marker_editor_window_set_highlight_current_line(window, toggled);
    }
  }
}

static void
show_right_margin_toggled(GtkToggleButton* toggler,
                          gpointer         user_data)
{
  GtkApplication* app = user_data;
  
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      gboolean toggled = gtk_toggle_button_get_active(toggler);
      marker_editor_window_set_show_right_margin(window, toggled);
    }
  }
}

static void
wrap_text_toggled(GtkToggleButton* toggler,
                  gpointer         user_data)
{
  GtkApplication* app = user_data;
  
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      gboolean toggled = gtk_toggle_button_get_active(toggler);
      marker_editor_window_set_wrap_text(window, toggled);
    }
  }
}

void
marker_prefs_load(GtkApplication* app)
{
  char key[50];
  char val[50];
  memset(key, 0, sizeof(key));
  memset(val, 0, sizeof(val));
  
  FILE* fp = fopen(CONF_FILE, "r");
  while (fscanf(fp, "%s %s", key, val) == 2)
  {
    if (marker_utils_str_starts_with(key, "syntax_theme"))
    {
      memcpy(prefs.syntax_theme, val, 50);
    }
    else
    if (marker_utils_str_starts_with(key, "css_theme"))
    {
      memcpy(prefs.css_theme, val, 50);
    }
    else
    if (marker_utils_str_starts_with(key, "show_line_numbers"))
    {
      if (strcmp(val, "TRUE") == 0)
      {
        prefs.show_line_numbers = TRUE;
      }
      else
      {
        prefs.show_line_numbers = FALSE;
      }
    }
    else
    if (marker_utils_str_starts_with(key, "highlight_current_line"))
    {
      if (strcmp(val, "TRUE") == 0)
      {
        prefs.highlight_current_line = TRUE;
      }
      else
      {
        prefs.highlight_current_line = FALSE;
      }
    }
    else
    if (marker_utils_str_starts_with(key, "wrap_text"))
    {
      if (strcmp(val, "TRUE") == 0)
      {
        prefs.wrap_text = TRUE;
      }
      else
      {
        prefs.wrap_text = FALSE;
      }
    }
    else
    if (marker_utils_str_starts_with(key, "show_right_margin"))
    {
      if (strcmp(val, "TRUE") == 0)
      {
        prefs.show_right_margin = TRUE;
      }
      else
      {
        prefs.show_right_margin = FALSE;
      }
    }
    else
    {
      printf("'%s' is not a valid key.\n", key);
    }
    
    memset(key, 0, sizeof(key));
    memset(val, 0, sizeof(val));
  }
  fclose(fp);
}

void
marker_prefs_save()
{
  
}

void
marker_prefs_show_window(GtkApplication* app)
{
  GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-prefs-window.ui");
  GtkWindow* win = GTK_WINDOW(gtk_builder_get_object(builder, "prefs_win"));
    
  GtkTreeIter iter;
  int model_len = 0;
    
  GtkListStore* syntax_list = gtk_list_store_new(1, G_TYPE_STRING);
  GtkSourceStyleSchemeManager* style_manager = gtk_source_style_scheme_manager_get_default();
  const gchar * const * ids = gtk_source_style_scheme_manager_get_scheme_ids(style_manager);
  for (int i = 0; ids[i] != NULL; ++i)
  {
    gtk_list_store_append(syntax_list, &iter);
    gtk_list_store_set(syntax_list, &iter, 0, ids[i], -1);
    ++model_len;
  }
  GtkComboBox* syntax_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
  marker_utils_combo_box_set_model(syntax_chooser, GTK_TREE_MODEL(syntax_list));
  char* active_str = NULL;
  for (int i = 0; i < model_len; ++i)
  {
    gtk_combo_box_set_active(syntax_chooser, i);
    active_str = marker_utils_combo_box_get_active_str(syntax_chooser);
    if (strcmp(active_str, prefs.syntax_theme) == 0 || active_str == NULL)
    {
      break;
    }
  }
  model_len = 0;
  
  GtkListStore* css_list = gtk_list_store_new(1, G_TYPE_STRING);
  DIR* dir;
  struct dirent* ent;
  char* filename;
  if ((dir = opendir(STYLES_DIR)) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      filename = ent->d_name;
      if (marker_utils_str_ends_with(filename, ".css"))
      {
        gtk_list_store_append(css_list, &iter);
        gtk_list_store_set(css_list, &iter, 0, filename, -1);
        ++model_len;
      }
    }   
  }
  GtkComboBox* css_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
  marker_utils_combo_box_set_model(css_chooser, GTK_TREE_MODEL(css_list));
  active_str = NULL;
  for (int i = 0; i < model_len; ++i)
  {
    gtk_combo_box_set_active(css_chooser, i);
    active_str = marker_utils_combo_box_get_active_str(css_chooser);
    if (strcmp(active_str, prefs.css_theme) == 0 || active_str == NULL)
    {
      break;
    }
  }
  model_len = 0;
  
  
  GtkToggleButton* show_line_numbers_check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_line_numbers_check_button"));
  gtk_toggle_button_set_active(show_line_numbers_check_button,
                               prefs.show_line_numbers);
  
  GtkToggleButton* highlight_current_line_check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "highlight_current_line_check_button"));
  gtk_toggle_button_set_active(highlight_current_line_check_button,
                               prefs.highlight_current_line);
  
  GtkToggleButton* wrap_text_check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wrap_text_check_button"));
  gtk_toggle_button_set_active(wrap_text_check_button,
                               prefs.wrap_text);
  
  GtkToggleButton* show_right_margin_check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_right_margin_check_button"));
  gtk_toggle_button_set_active(show_right_margin_check_button,
                               prefs.show_right_margin);
  
  gtk_builder_add_callback_symbol(builder, "css_chosen", G_CALLBACK(css_chosen));
  gtk_builder_add_callback_symbol(builder, "syntax_chosen", G_CALLBACK(syntax_chosen));
  gtk_builder_add_callback_symbol(builder, "show_line_numbers_toggled", G_CALLBACK(show_line_numbers_toggled));
  gtk_builder_add_callback_symbol(builder, "highlight_current_line_toggled", G_CALLBACK(highlight_current_line_toggled));
  gtk_builder_add_callback_symbol(builder, "show_right_margin_toggled", G_CALLBACK(show_right_margin_toggled));
  gtk_builder_add_callback_symbol(builder, "wrap_text_toggled", G_CALLBACK(wrap_text_toggled));
  gtk_builder_connect_signals(builder, app);
  g_object_unref(builder);
  gtk_widget_show_all(GTK_WIDGET(win));
}

char*
marker_prefs_get_syntax_theme()
{
  return marker_utils_allocate_string(prefs.syntax_theme);
}

char*
marker_prefs_get_css_theme()
{
  return marker_utils_allocate_string(prefs.css_theme);
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

