#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <stdlib.h>

#include "marker-prefs.h"
#include "marker-editor-window.h"
#include "marker-utils.h"

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
marker_prefs_show_window(GtkApplication* app)
{
  GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-prefs-window.ui");
  GtkWindow* win = GTK_WINDOW(gtk_builder_get_object(builder, "prefs_win"));
    
  GtkTreeIter iter;
    
  GtkListStore* syntax_list = gtk_list_store_new(1, G_TYPE_STRING);
  GtkSourceStyleSchemeManager* style_manager = gtk_source_style_scheme_manager_get_default();
  const gchar * const * ids = gtk_source_style_scheme_manager_get_scheme_ids(style_manager);
  for (int i = 0; ids[i] != NULL; ++i)
  {
    gtk_list_store_append(syntax_list, &iter);
    gtk_list_store_set(syntax_list, &iter, 0, ids[i], -1);
  }
  GtkComboBox* syntax_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
  marker_utils_combo_box_set_model(syntax_chooser, GTK_TREE_MODEL(syntax_list));
    
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
      }
    }   
  }
  GtkComboBox* css_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
  marker_utils_combo_box_set_model(css_chooser, GTK_TREE_MODEL(css_list));
    
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

