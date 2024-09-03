/*
 * marker-prefs.c
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Marker; see the file LICENSE.md. If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <gtkspell/gtkspell.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "marker.h"
#include "marker-widget.h"
#include "marker-string.h"
#include "marker-window.h"

#include "marker-prefs.h"

MarkerPrefs prefs;

gboolean
marker_prefs_get_use_dark_theme()
{
  return g_settings_get_boolean(prefs.window_settings, "enable-dark-mode");
}

void
marker_prefs_set_use_dark_theme(gboolean state)
{
  g_settings_set_boolean(prefs.window_settings, "enable-dark-mode", state);
}

guint
marker_prefs_get_window_width()
{
  return g_settings_get_uint(prefs.window_settings, "window-width");
}

void
marker_prefs_set_window_width(guint width)
{
  g_settings_set_uint(prefs.window_settings, "window-width", width);
}

guint
marker_prefs_get_window_height()
{
  return g_settings_get_uint(prefs.window_settings, "window-height");
}

void
marker_prefs_set_window_height(guint height)
{
  g_settings_set_uint(prefs.window_settings, "window-height", height);
}

void
marker_prefs_get_window_position(gint *pos_x,
                                 gint *pos_y)
{
  g_settings_get(prefs.window_settings, "window-position", "(ii)", pos_x, pos_y);
}

void
marker_prefs_set_window_position(gint pos_x,
                                 gint pos_y)
{
  g_settings_set(prefs.window_settings, "window-position", "(ii)", pos_x, pos_y);
}

guint
marker_prefs_get_editor_pane_width()
{
  return g_settings_get_uint(prefs.window_settings, "editor-pane-width");
}

void
marker_prefs_set_editor_pane_width(guint width)
{
  g_settings_set_uint(prefs.window_settings, "editor-pane-width", width);
}

gboolean
marker_prefs_get_show_sidebar()
{
  return g_settings_get_boolean(prefs.window_settings, "show-sidebar");
}

void
marker_prefs_set_show_sidebar(gboolean state)
{
  g_settings_set_boolean(prefs.window_settings, "show-sidebar", state);
}

gboolean
marker_prefs_get_use_syntax_theme()
{
  return g_settings_get_boolean(prefs.editor_settings, "enable-syntax-theme");
}

void
marker_prefs_set_use_syntax_theme(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "enable-syntax-theme", state);
}

gboolean
marker_prefs_get_auto_save() 
{
  return g_settings_get_boolean(prefs.preview_settings, "autosave-check-button" );
}

void
marker_prefs_set_auto_save(gboolean state)
{
  g_settings_set_boolean(prefs.preview_settings, "autosave-check-button" , state); 
}

char*
marker_prefs_get_css_theme()
{
  return g_settings_get_string(prefs.preview_settings, "css-theme");
}

void
marker_prefs_set_css_theme(const char* theme)
{
  g_settings_set_string(prefs.preview_settings, "css-theme", theme);
}

gboolean
marker_prefs_get_use_css_theme()
{
  return g_settings_get_boolean(prefs.preview_settings, "css-toggle");
}

void
marker_prefs_set_use_css_theme(gboolean state)
{
  g_settings_set_boolean(prefs.preview_settings, "css-toggle", state);
}

char*
marker_prefs_get_highlight_theme()
{
  return g_settings_get_string(prefs.preview_settings, "highlight-theme");
}


void
marker_prefs_set_highlight_theme(const char* theme)
{
  g_settings_set_string(prefs.preview_settings, "highlight-theme", theme);
}

gboolean
marker_prefs_get_use_mathjs()
{
  return g_settings_get_boolean(prefs.preview_settings, "mathjs-toggle");
}

void
marker_prefs_set_use_mathjs(gboolean state)
{
  g_settings_set_boolean(prefs.preview_settings, "mathjs-toggle", state);
}

gdouble
makrer_prefs_get_zoom_level()
{
  return  g_settings_get_double(prefs.preview_settings, "preview-zoom-level");
}

void
marker_prefs_set_zoom_level(gdouble val)
{
  g_settings_set_double(prefs.preview_settings, "preview-zoom-level", val);
}

gboolean
marker_prefs_get_use_mermaid()
{
  return g_settings_get_boolean(prefs.preview_settings, "mermaid-toggle");
}

void
marker_prefs_set_use_mermaid(gboolean state)
{
  g_settings_set_boolean(prefs.preview_settings, "mermaid-toggle", state);
}

gboolean
marker_prefs_get_use_charter()
{
  return g_settings_get_boolean(prefs.preview_settings, "charter-toggle");
}

void
marker_prefs_set_use_charter(gboolean state)
{
  g_settings_set_boolean(prefs.preview_settings, "charter-toggle", state);
}

gboolean
marker_prefs_get_use_highlight()
{
  return g_settings_get_boolean(prefs.preview_settings, "highlight-toggle");
}

void
marker_prefs_set_use_highlight(gboolean state)
{
  g_settings_set_boolean(prefs.preview_settings, "highlight-toggle", state);
}

char*
marker_prefs_get_syntax_theme()
{
  return g_settings_get_string(prefs.editor_settings, "syntax-theme");
}

void
marker_prefs_set_syntax_theme(const char* theme)
{
  g_settings_set_string(prefs.editor_settings, "syntax-theme", theme);
}

guint
marker_prefs_get_right_margin_position()
{
  return g_settings_get_uint(prefs.editor_settings, "show-right-margin-position");
}

void
marker_prefs_set_right_margin_position(guint position)
{
  g_settings_set_uint(prefs.editor_settings, "show-right-margin-position", position);
}

gboolean
marker_prefs_get_replace_tabs()
{
  return  g_settings_get_boolean(prefs.editor_settings, "replace-tabs");
}

void
marker_prefs_set_replace_tabs(gboolean state)
{
   g_settings_set_boolean(prefs.editor_settings, "replace-tabs", state);
}

guint
marker_prefs_get_tab_width()
{
  return g_settings_get_uint(prefs.editor_settings, "tab-width");
}

void
marker_prefs_set_tab_width(guint width)
{
  g_settings_set_uint(prefs.editor_settings, "tab-width", width);
}

gboolean
marker_prefs_get_auto_indent()
{
  return g_settings_get_boolean(prefs.editor_settings, "auto-indent");
}

void
marker_prefs_set_auto_indent(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "auto-indent", state);
}

gboolean
marker_prefs_get_show_spaces (void)
{
  return g_settings_get_boolean(prefs.editor_settings, "show-spaces");
}

void
marker_prefs_set_show_spaces (gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "show-spaces", state);
}

gboolean
marker_prefs_get_spell_check()
{
  return g_settings_get_boolean(prefs.editor_settings, "spell-check");
}

void
marker_prefs_set_spell_check(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "spell-check", state);
}

gchar*
marker_prefs_get_spell_check_language()
{
  return g_settings_get_string(prefs.editor_settings, "spell-check-lang");
}

void
marker_prefs_set_spell_check_language(const char* lang)
{
  g_settings_set_string(prefs.editor_settings, "spell-check-lang", lang);
}

gboolean
marker_prefs_get_show_line_numbers()
{
  return g_settings_get_boolean(prefs.editor_settings, "show-line-numbers");
}

void
marker_prefs_set_show_line_numbers(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "show-line-numbers", state);
}

gboolean
marker_prefs_get_highlight_current_line()
{
  return g_settings_get_boolean(prefs.editor_settings, "highlight-current-line");
}

void
marker_prefs_set_highlight_current_line(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "highlight-current-line", state);
}

gboolean
marker_prefs_get_wrap_text()
{
  return g_settings_get_boolean(prefs.editor_settings, "wrap-text");
}

void
marker_prefs_set_wrap_text(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "wrap-text", state);
}

gboolean
marker_prefs_get_show_right_margin()
{
  return g_settings_get_boolean(prefs.editor_settings, "show-right-margin");
}

void
marker_prefs_set_show_right_margin(gboolean state)
{
  g_settings_set_boolean(prefs.editor_settings, "show-right-margin", state);
}

MarkerViewMode
marker_prefs_get_default_view_mode()
{
  return g_settings_get_enum(prefs.window_settings, "view-mode");
}

void
marker_prefs_set_default_view_mode(MarkerViewMode view_mode)
{
  g_settings_set_enum(prefs.window_settings, "view-mode", view_mode);
}


MarkerMathBackEnd
marker_prefs_get_math_backend (void)
{
  return g_settings_get_enum(prefs.preview_settings, "math-backend");
}

void
marker_prefs_set_math_backend (MarkerMathBackEnd   backend)
{
  g_settings_set_enum(prefs.preview_settings, "math-backend", backend);
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

  return list;
}

GList*
marker_prefs_get_available_highlight_themes()
{
  GList* list = NULL;
  char* list_item;

  DIR* dir;
  struct dirent* ent;
  char* filename;

  if ((dir = opendir(HIGHLIGHT_STYLES_DIR)) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      filename = ent->d_name;

      if (marker_string_ends_with(filename, ".css"))
      {
        list_item = marker_string_filename_get_name_noext(filename);
        list = g_list_prepend(list, list_item);
      }
    }
  }
  closedir(dir);

  return list;
}

GList*
marker_prefs_get_available_languages()
{
  GList* list = gtk_spell_checker_get_language_list ();
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
update_editors ()
{
  GtkApplication *app = marker_get_app();
  GList *windows = gtk_application_get_windows(app);
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW(item->data))
    {
      MarkerWindow *window = item->data;
      MarkerEditor *editor = marker_window_get_active_editor (window);
      marker_editor_apply_prefs (editor);
    }
  }
}

static void
refresh_preview ()
{
  GtkApplication *app = marker_get_app();
  GList *windows = gtk_application_get_windows(app);
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW(item->data))
    {
      MarkerWindow *window = item->data;
      marker_window_refresh_all_preview(window);
    }
  }
}

static void
show_line_numbers_toggled(GtkToggleButton* button,
                          gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_show_line_numbers(state);
  update_editors ();
}

static void
editor_syntax_toggled(GtkToggleButton* button,
                      gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_syntax_theme(state);

  marker_prefs_set_use_highlight(state);
  if (user_data)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), state);
  }

  update_editors ();
}

static void
css_toggled(GtkToggleButton* button,
            gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_css_theme(state);

  if (user_data)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), state);
  }

  refresh_preview();
}

static void
highlight_current_line_toggled(GtkToggleButton* button,
                               gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_highlight_current_line(state);
  update_editors ();
}

static void
enable_mathjs_toggled(GtkToggleButton* button,
                       gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_mathjs(state);
  if (user_data)
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), state);
  refresh_preview();
}

static void
enable_mermaid_toggled(GtkToggleButton* button,
                       gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_mermaid(state);
  refresh_preview();
}

static void
enable_charter_toggled(GtkToggleButton* button,
                       gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_charter(state);
  refresh_preview();
}

static void
wrap_text_toggled(GtkToggleButton* button,
                  gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_wrap_text(state);
  update_editors ();
}

static void
show_spaces_toggled(GtkToggleButton* button,
                    gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_show_spaces(state);
  update_editors ();
}

static void
enable_dark_mode_toggled(GtkToggleButton* button,
                         gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_dark_theme(state);
  g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", state, NULL);
}

static void
auto_indent_toggled(GtkToggleButton* button,
                    gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_auto_indent(state);
  update_editors ();
}

static void
spell_lang_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);
  marker_prefs_set_spell_check_language(choice);
  update_editors ();
}

static void
spell_check_toggled(GtkToggleButton* button,
                     gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_spell_check(state);

  if (user_data)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), state);
  }

  update_editors ();
}

static void
replace_tabs_toggled(GtkToggleButton* button,
                     gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_replace_tabs(state);
  update_editors ();
}

static void
tab_width_value_changed(GtkSpinButton *spin_button,
                        gpointer       user_data)
{
  guint value = gtk_spin_button_get_value_as_int (spin_button);
  marker_prefs_set_tab_width(value);
  update_editors ();
}

static void
right_margin_position_value_changed(GtkSpinButton* spin_button,
                                    gpointer       user_data)
{
  guint value = gtk_spin_button_get_value_as_int(spin_button);
  marker_prefs_set_right_margin_position(value);
  update_editors ();
}

static void
show_right_margin_toggled(GtkToggleButton* button,
                          gpointer         user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_show_right_margin(state);

  if (user_data)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), state);
  }

  update_editors ();
}

static void
syntax_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
  g_autofree gchar* choice = marker_widget_combo_box_get_active_str(combo_box);
  marker_prefs_set_syntax_theme (choice);
  update_editors ();
}

static void
css_chosen(GtkComboBox* combo_box,
           gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);

  marker_prefs_set_css_theme(choice);

  free(choice);
  refresh_preview();
}

static void
highlight_css_chosen(GtkComboBox* combo_box,
                     gpointer     user_data)
{
  char* choice = marker_widget_combo_box_get_active_str(combo_box);
  marker_prefs_set_highlight_theme(choice);

  free(choice);

  refresh_preview();
}

static void
code_highlight_toggled(GtkToggleButton* button,
                       gpointer user_data)
{
  gboolean state = gtk_toggle_button_get_active(button);
  marker_prefs_set_use_highlight(state);

  if (user_data)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), state);
  }

  refresh_preview();
}

static void
default_view_mode_chosen(GtkComboBox* combo_box,
                         gpointer     user_data)
{
  MarkerViewMode mode = gtk_combo_box_get_active(combo_box);
  marker_prefs_set_default_view_mode(mode);
}

static void
math_backend_changed (GtkComboBox* combo_box,
                         gpointer     user_data)
{
  MarkerMathBackEnd backend = gtk_combo_box_get_active(combo_box);
  marker_prefs_set_math_backend(backend);
  refresh_preview();
}

void
marker_prefs_show_window()
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource(
      "/com/github/fabiocolacio/marker/ui/marker-prefs-window.ui");

  GList *list = NULL;
  GtkComboBox* combo_box;
  GtkToggleButton* check_button;
  GtkSpinButton* spin_button;

  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
  list = marker_prefs_get_available_syntax_themes();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  char* syntax = marker_prefs_get_syntax_theme();
  marker_widget_combo_box_set_active_str(combo_box,syntax, g_list_length(list));
  gtk_widget_set_sensitive(GTK_WIDGET(combo_box), marker_prefs_get_use_highlight());
  g_free(syntax);
  g_list_free_full(list, free);
  list = NULL;

  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
  list = marker_prefs_get_available_stylesheets();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  char* css = marker_prefs_get_css_theme();
  char* css_filename = marker_string_filename_get_name(css);
  marker_widget_combo_box_set_active_str(combo_box, css_filename, g_list_length(list));
  gtk_widget_set_sensitive(GTK_WIDGET(combo_box), marker_prefs_get_use_css_theme());
  free(css_filename);
  g_free(css);
  g_list_free_full(list, free);
  list = NULL;

  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "highlight_css_chooser"));
  list = marker_prefs_get_available_highlight_themes();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  char* theme = marker_prefs_get_highlight_theme();
  marker_widget_combo_box_set_active_str(combo_box, theme, g_list_length(list));
  gtk_widget_set_sensitive(GTK_WIDGET(combo_box), marker_prefs_get_use_highlight());
  g_free(theme);
  g_list_free_full(list, free);
  list = NULL;

  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "view_mode_chooser"));
  GtkCellRenderer* cell_renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box), cell_renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box),
                                 cell_renderer,
                                 "text", 0,
                                 NULL);
  gtk_combo_box_set_active(combo_box, marker_prefs_get_default_view_mode());

  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "math_backends_combo"));
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box), cell_renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box),
                                 cell_renderer,
                                 "text", 0,
                                 NULL);
  gtk_combo_box_set_active(combo_box, marker_prefs_get_math_backend());
  gtk_widget_set_sensitive(GTK_WIDGET(combo_box), marker_prefs_get_use_mathjs());

  combo_box = GTK_COMBO_BOX(gtk_builder_get_object(builder, "spell_lang_chooser"));
  list = marker_prefs_get_available_languages();
  marker_widget_populate_combo_box_with_strings(combo_box, list);
  char* lang = marker_prefs_get_spell_check_language();
  marker_widget_combo_box_set_active_str(combo_box, lang, g_list_length(list));
  gtk_widget_set_sensitive(GTK_WIDGET(combo_box), marker_prefs_get_spell_check());
  g_free(lang);
  g_list_free_full(list, free);
  list = NULL;

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "editor_syntax_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_highlight());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "css_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_css_theme());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "editor_syntax_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_syntax_theme());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "mathjs_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_mathjs());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "mermaid_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_mermaid());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "charter_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_charter());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "code_highlight_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_highlight());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_line_numbers_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_show_line_numbers());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_right_margin_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_show_right_margin());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wrap_text_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_wrap_text());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_spaces_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_show_spaces());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "highlight_current_line_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_highlight_current_line());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "auto_indent_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_auto_indent());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "replace_tabs_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_replace_tabs());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "spell_check_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_spell_check());

  check_button =
    GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "enable_dark_mode_check_button"));
  gtk_toggle_button_set_active(check_button, marker_prefs_get_use_dark_theme());

  spin_button =
    GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "right_margin_position_spin_button"));
  gtk_widget_set_sensitive(GTK_WIDGET(spin_button), marker_prefs_get_show_right_margin());
  gtk_spin_button_set_range(spin_button, 1, 1000);
  gtk_spin_button_set_increments(spin_button, 1, 0);
  gtk_spin_button_set_value(spin_button, marker_prefs_get_right_margin_position());

  spin_button =
    GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "tab_width_spin_button"));
  gtk_spin_button_set_range(spin_button, 1, 12);
  gtk_spin_button_set_increments(spin_button, 1, 0);
  gtk_spin_button_set_value(spin_button, marker_prefs_get_tab_width());

  GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "prefs_win"));
	gtk_widget_show_all(GTK_WIDGET(window));
  gtk_window_present(window);


  gtk_builder_add_callback_symbol(builder,
                                  "syntax_chosen",
                                  G_CALLBACK(syntax_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "editor_syntax_toggled",
                                  G_CALLBACK(editor_syntax_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "css_chosen",
                                  G_CALLBACK(css_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "css_toggled",
                                  G_CALLBACK(css_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "highlight_css_chosen",
                                  G_CALLBACK(highlight_css_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "code_highlight_toggled",
                                  G_CALLBACK(code_highlight_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "default_view_mode_chosen",
                                  G_CALLBACK(default_view_mode_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "math_backends_combo_changed_cb",
                                  G_CALLBACK(math_backend_changed));
  gtk_builder_add_callback_symbol(builder,
                                  "show_line_numbers_toggled",
                                  G_CALLBACK(show_line_numbers_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "highlight_current_line_toggled",
                                  G_CALLBACK(highlight_current_line_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "replace_tabs_toggled",
                                  G_CALLBACK(replace_tabs_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "auto_indent_toggled",
                                  G_CALLBACK(auto_indent_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "spell_check_toggled",
                                  G_CALLBACK(spell_check_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "spell_lang_chosen",
                                  G_CALLBACK(spell_lang_chosen));
  gtk_builder_add_callback_symbol(builder,
                                  "tab_width_value_changed",
                                  G_CALLBACK(tab_width_value_changed));
  gtk_builder_add_callback_symbol(builder,
                                  "right_margin_position_value_changed",
                                  G_CALLBACK(right_margin_position_value_changed));
  gtk_builder_add_callback_symbol(builder,
                                  "enable_mathjs_toggled",
                                  G_CALLBACK(enable_mathjs_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "enable_mermaid_toggled",
                                  G_CALLBACK(enable_mermaid_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "wrap_text_toggled",
                                  G_CALLBACK(wrap_text_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "show_spaces_toggled",
                                  G_CALLBACK(show_spaces_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "show_right_margin_toggled",
                                  G_CALLBACK(show_right_margin_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "enable_dark_mode_toggled",
                                  G_CALLBACK(enable_dark_mode_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "editor_syntax_toggled",
                                  G_CALLBACK(editor_syntax_toggled));
  gtk_builder_add_callback_symbol(builder,
                                  "enable_charter_toggled",
                                  G_CALLBACK(enable_charter_toggled));
  gtk_builder_connect_signals(builder, NULL);

  g_object_unref(builder);
}

void
marker_prefs_load()
{
  prefs.editor_settings =
    g_settings_new("com.github.fabiocolacio.marker.preferences.editor");
  prefs.preview_settings =
    g_settings_new("com.github.fabiocolacio.marker.preferences.preview");
  prefs.window_settings =
    g_settings_new("com.github.fabiocolacio.marker.preferences.window");
}

