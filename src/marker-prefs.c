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
#include <libspelling.h>
#include <adwaita.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "marker.h"
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
marker_prefs_apply_color_scheme (void)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();
  adw_style_manager_set_color_scheme (manager,
                                      marker_prefs_get_use_dark_theme ()
                                        ? ADW_COLOR_SCHEME_FORCE_DARK
                                        : ADW_COLOR_SCHEME_DEFAULT);
}

static void
update_editors (void)
{
  GtkApplication *app = marker_get_app ();
  GList *windows = gtk_application_get_windows (app);
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW (item->data))
    {
      MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (item->data));
      if (editor)
        marker_editor_apply_prefs (editor);
    }
  }
}

static void
refresh_preview (void)
{
  GtkApplication *app = marker_get_app ();
  GList *windows = gtk_application_get_windows (app);
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW (item->data))
      marker_window_refresh_all_preview (MARKER_WINDOW (item->data));
  }
}

/* ------------------------------------------------------------------------- */
/* Preferences dialog                                                         */
/* ------------------------------------------------------------------------- */

typedef enum
{
  AFTER_NOTHING,
  AFTER_UPDATE_EDITORS,
  AFTER_REFRESH_PREVIEW
} AfterChange;

static void
run_after (AfterChange after)
{
  switch (after)
  {
    case AFTER_UPDATE_EDITORS:  update_editors ();  break;
    case AFTER_REFRESH_PREVIEW: refresh_preview (); break;
    default: break;
  }
}

static void
switch_changed_cb (GObject    *row,
                   GParamSpec *pspec,
                   gpointer    user_data)
{
  run_after (GPOINTER_TO_INT (user_data));
}

static void
bind_switch (GtkBuilder  *builder,
             const gchar *id,
             GSettings   *settings,
             const gchar *key,
             AfterChange  after)
{
  GObject *row = gtk_builder_get_object (builder, id);
  g_return_if_fail (row != NULL);
  g_settings_bind (settings, key, row, "active", G_SETTINGS_BIND_DEFAULT);
  g_signal_connect (row, "notify::active", G_CALLBACK (switch_changed_cb), GINT_TO_POINTER (after));
}

static void
bind_spin (GtkBuilder  *builder,
           const gchar *id,
           GSettings   *settings,
           const gchar *key,
           AfterChange  after)
{
  GObject *row = gtk_builder_get_object (builder, id);
  g_return_if_fail (row != NULL);
  g_settings_bind (settings, key, row, "value", G_SETTINGS_BIND_DEFAULT);
  g_signal_connect (row, "notify::value", G_CALLBACK (switch_changed_cb), GINT_TO_POINTER (after));
}

static void
bind_sensitivity (GtkBuilder  *builder,
                  const gchar *switch_id,
                  const gchar *target_id)
{
  GObject *sw = gtk_builder_get_object (builder, switch_id);
  GObject *target = gtk_builder_get_object (builder, target_id);
  g_return_if_fail (sw != NULL && target != NULL);
  g_object_bind_property (sw, "active", target, "sensitive", G_BINDING_SYNC_CREATE);
}

/* String combo rows: the displayed model is a GtkStringList; an optional
 * "values" GtkStringList (same length) holds the value written to GSettings. */

typedef struct
{
  GSettings   *settings;
  const gchar *key;
  AfterChange  after;
} StringComboBinding;

static void
string_combo_changed_cb (AdwComboRow *row,
                         GParamSpec  *pspec,
                         gpointer     user_data)
{
  StringComboBinding *binding = user_data;
  guint selected = adw_combo_row_get_selected (row);
  if (selected == GTK_INVALID_LIST_POSITION)
    return;

  GtkStringList *values = g_object_get_data (G_OBJECT (row), "values");
  if (!values)
    values = GTK_STRING_LIST (adw_combo_row_get_model (row));

  const gchar *value = gtk_string_list_get_string (values, selected);
  if (!value)
    return;

  g_settings_set_string (binding->settings, binding->key, value);
  run_after (binding->after);
}

static void
bind_string_combo (GtkBuilder  *builder,
                   const gchar *id,
                   GList       *display_items,
                   GList       *value_items,   /* may be NULL: same as display */
                   GSettings   *settings,
                   const gchar *key,
                   const gchar *current,
                   AfterChange  after)
{
  AdwComboRow *row = ADW_COMBO_ROW (gtk_builder_get_object (builder, id));
  g_return_if_fail (row != NULL);

  GtkStringList *display = gtk_string_list_new (NULL);
  GtkStringList *values = value_items ? gtk_string_list_new (NULL) : NULL;

  guint selected = 0, i = 0;
  GList *d = display_items, *v = value_items;
  for (; d != NULL; d = d->next, v = v ? v->next : NULL, ++i)
  {
    const gchar *value = v ? v->data : d->data;
    gtk_string_list_append (display, d->data);
    if (values)
      gtk_string_list_append (values, value);
    if (current && g_strcmp0 (value, current) == 0)
      selected = i;
  }

  adw_combo_row_set_model (row, G_LIST_MODEL (display));
  g_object_unref (display);
  if (values)
    g_object_set_data_full (G_OBJECT (row), "values", values, g_object_unref);

  adw_combo_row_set_selected (row, selected);

  StringComboBinding *binding = g_new0 (StringComboBinding, 1);
  binding->settings = settings;
  binding->key = key;
  binding->after = after;
  g_object_set_data_full (G_OBJECT (row), "binding", binding, g_free);
  g_signal_connect (row, "notify::selected", G_CALLBACK (string_combo_changed_cb), binding);
}

/* Enum combo rows: selected index == enum value */

static void
enum_combo_changed_cb (AdwComboRow *row,
                       GParamSpec  *pspec,
                       gpointer     user_data)
{
  StringComboBinding *binding = user_data;
  guint selected = adw_combo_row_get_selected (row);
  if (selected == GTK_INVALID_LIST_POSITION)
    return;
  g_settings_set_enum (binding->settings, binding->key, selected);
  run_after (binding->after);
}

static void
bind_enum_combo (GtkBuilder  *builder,
                 const gchar *id,
                 GSettings   *settings,
                 const gchar *key,
                 AfterChange  after)
{
  AdwComboRow *row = ADW_COMBO_ROW (gtk_builder_get_object (builder, id));
  g_return_if_fail (row != NULL);

  adw_combo_row_set_selected (row, g_settings_get_enum (settings, key));

  StringComboBinding *binding = g_new0 (StringComboBinding, 1);
  binding->settings = settings;
  binding->key = key;
  binding->after = after;
  g_object_set_data_full (G_OBJECT (row), "binding", binding, g_free);
  g_signal_connect (row, "notify::selected", G_CALLBACK (enum_combo_changed_cb), binding);
}

static void
dark_mode_changed_cb (GObject    *row,
                      GParamSpec *pspec,
                      gpointer    user_data)
{
  marker_prefs_apply_color_scheme ();
}

static gint
compare_strings (gconstpointer a, gconstpointer b)
{
  return g_ascii_strcasecmp (a, b);
}

static void
list_spelling_languages (GList **names,
                         GList **codes)
{
  *names = NULL;
  *codes = NULL;

  SpellingProvider *provider = spelling_provider_get_default ();
  GListModel *languages = spelling_provider_list_languages (provider);
  if (!languages)
    return;

  guint n = g_list_model_get_n_items (languages);
  for (guint i = 0; i < n; ++i)
  {
    g_autoptr (SpellingLanguage) info = g_list_model_get_item (languages, i);
    const gchar *code = spelling_language_get_code (info);
    const gchar *name = spelling_language_get_name (info);
    if (!code)
      continue;
    *codes = g_list_append (*codes, g_strdup (code));
    *names = g_list_append (*names, g_strdup_printf ("%s (%s)", name ? name : code, code));
  }
  g_object_unref (languages);
}

void
marker_prefs_show_window (void)
{
  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/github/fabiocolacio/marker/ui/marker-prefs-dialog.ui");

  GSettings *editor = prefs.editor_settings;
  GSettings *preview = prefs.preview_settings;
  GSettings *window = prefs.window_settings;

  /* ---- Editor ---- */
  bind_switch (builder, "show_line_numbers_row",      editor, "show-line-numbers",      AFTER_UPDATE_EDITORS);
  bind_switch (builder, "wrap_text_row",              editor, "wrap-text",              AFTER_UPDATE_EDITORS);
  bind_switch (builder, "show_spaces_row",            editor, "show-spaces",            AFTER_UPDATE_EDITORS);
  bind_switch (builder, "highlight_current_line_row", editor, "highlight-current-line", AFTER_UPDATE_EDITORS);
  bind_switch (builder, "show_right_margin_row",      editor, "show-right-margin",      AFTER_UPDATE_EDITORS);
  bind_spin   (builder, "right_margin_position_row",  editor, "show-right-margin-position", AFTER_UPDATE_EDITORS);
  bind_sensitivity (builder, "show_right_margin_row", "right_margin_position_row");

  bind_switch (builder, "editor_syntax_row",          editor, "enable-syntax-theme",    AFTER_UPDATE_EDITORS);
  {
    GList *themes = g_list_sort (marker_prefs_get_available_syntax_themes (), compare_strings);
    g_autofree gchar *current = marker_prefs_get_syntax_theme ();
    bind_string_combo (builder, "syntax_chooser_row", themes, NULL, editor, "syntax-theme", current, AFTER_UPDATE_EDITORS);
    g_list_free_full (themes, free);
  }
  bind_sensitivity (builder, "editor_syntax_row", "syntax_chooser_row");

  bind_switch (builder, "spell_check_row",            editor, "spell-check",            AFTER_UPDATE_EDITORS);
  {
    GList *names = NULL, *codes = NULL;
    list_spelling_languages (&names, &codes);
    g_autofree gchar *current = marker_prefs_get_spell_check_language ();
    bind_string_combo (builder, "spell_lang_row", names, codes, editor, "spell-check-lang", current, AFTER_UPDATE_EDITORS);
    g_list_free_full (names, g_free);
    g_list_free_full (codes, g_free);
  }
  bind_sensitivity (builder, "spell_check_row", "spell_lang_row");

  bind_switch (builder, "auto_indent_row",            editor, "auto-indent",            AFTER_UPDATE_EDITORS);
  bind_switch (builder, "replace_tabs_row",           editor, "replace-tabs",           AFTER_UPDATE_EDITORS);
  bind_spin   (builder, "tab_width_row",              editor, "tab-width",              AFTER_UPDATE_EDITORS);

  /* ---- Preview ---- */
  bind_switch (builder, "css_row",                    preview, "css-toggle",            AFTER_REFRESH_PREVIEW);
  {
    GList *sheets = g_list_sort (marker_prefs_get_available_stylesheets (), compare_strings);
    g_autofree gchar *css = marker_prefs_get_css_theme ();
    g_autofree gchar *current = marker_string_filename_get_name (css);
    bind_string_combo (builder, "css_chooser_row", sheets, NULL, preview, "css-theme", current, AFTER_REFRESH_PREVIEW);
    g_list_free_full (sheets, free);
  }
  bind_sensitivity (builder, "css_row", "css_chooser_row");

  bind_switch (builder, "code_highlight_row",         preview, "highlight-toggle",      AFTER_REFRESH_PREVIEW);
  {
    GList *themes = g_list_sort (marker_prefs_get_available_highlight_themes (), compare_strings);
    g_autofree gchar *current = marker_prefs_get_highlight_theme ();
    bind_string_combo (builder, "highlight_css_chooser_row", themes, NULL, preview, "highlight-theme", current, AFTER_REFRESH_PREVIEW);
    g_list_free_full (themes, free);
  }
  bind_sensitivity (builder, "code_highlight_row", "highlight_css_chooser_row");

  bind_switch (builder, "mermaid_row",                preview, "mermaid-toggle",        AFTER_REFRESH_PREVIEW);
  bind_switch (builder, "charter_row",                preview, "charter-toggle",        AFTER_REFRESH_PREVIEW);
  bind_switch (builder, "mathjs_row",                 preview, "mathjs-toggle",         AFTER_REFRESH_PREVIEW);
  bind_enum_combo (builder, "math_backend_row",       preview, "math-backend",          AFTER_REFRESH_PREVIEW);
  bind_sensitivity (builder, "mathjs_row", "math_backend_row");

  /* ---- Window ---- */
  bind_enum_combo (builder, "view_mode_row",          window, "view-mode",              AFTER_NOTHING);
  bind_switch (builder, "dark_mode_row",              window, "enable-dark-mode",       AFTER_NOTHING);
  g_signal_connect (gtk_builder_get_object (builder, "dark_mode_row"), "notify::active",
                    G_CALLBACK (dark_mode_changed_cb), NULL);

  AdwDialog *dialog = ADW_DIALOG (gtk_builder_get_object (builder, "prefs_dialog"));
  GtkWindow *parent = gtk_application_get_active_window (marker_get_app ());
  adw_dialog_present (dialog, parent ? GTK_WIDGET (parent) : NULL);

  g_object_unref (builder);
}

void
marker_prefs_load (void)
{
  prefs.editor_settings =
    g_settings_new ("com.github.fabiocolacio.marker.preferences.editor");
  prefs.preview_settings =
    g_settings_new ("com.github.fabiocolacio.marker.preferences.preview");
  prefs.window_settings =
    g_settings_new ("com.github.fabiocolacio.marker.preferences.window");
}
