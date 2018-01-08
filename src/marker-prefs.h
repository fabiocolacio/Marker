#ifndef __MARKER_PREFS_H__
#define __MARKER_PREFS_H__

#include "marker-editor-window.h"

typedef struct {
  GSettings* editor_settings;
  GSettings* preview_settings;
  GSettings* window_settings;
} MarkerPrefs;

char*
marker_prefs_get_syntax_theme();

void
marker_prefs_set_syntax_theme(const char* theme);

gboolean
marker_prefs_get_replace_tabs();

void
marker_prefs_set_replace_tabs(gboolean state);

guint
marker_prefs_get_tab_width();

void
marker_prefs_set_tab_width(guint width);

gboolean
marker_prefs_get_auto_indent();

void
marker_prefs_set_auto_indent(gboolean state);

gboolean
marker_prefs_get_spell_check();

void
marker_prefs_set_spell_check(gboolean state);

gchar*
marker_prefs_get_spell_check_langauge();

void
marker_prefs_set_spell_check_language(const gchar* lang);

gboolean
marker_prefs_get_show_line_numbers();

void
marker_prefs_set_show_line_numbers(gboolean state);

gboolean
marker_prefs_get_highlight_current_line();

void
marker_prefs_set_highlight_current_line(gboolean state);

gboolean
marker_prefs_get_wrap_text();

void
marker_prefs_set_wrap_text(gboolean state);

gboolean
marker_prefs_get_show_right_margin();

void
marker_prefs_set_show_right_margin(gboolean state);

guint
marker_prefs_get_right_margin_position();

void
marker_prefs_set_right_margin_position(guint position);

char*
marker_prefs_get_css_theme();

void
marker_prefs_set_css_theme(const char* theme);

gboolean
marker_prefs_get_use_css_theme();

void
marker_prefs_set_use_css_theme(gboolean state);

char*
marker_prefs_get_highlight_theme();

void
marker_prefs_set_highlight_theme(const char* theme);

gboolean
marker_prefs_get_use_katex();

void
marker_prefs_set_use_katex(gboolean state);

gboolean
marker_prefs_get_use_highlight();

void
marker_prefs_set_use_highlight(gboolean state);

gboolean
marker_prefs_get_gnome_appmenu();

void
marker_prefs_set_gnome_appmenu(gboolean state);

MarkerEditorWindowViewMode
marker_prefs_get_default_view_mode();

void
marker_prefs_set_default_view_mode(MarkerEditorWindowViewMode view_mode);

void
marker_prefs_load();

void
marker_prefs_show_window();

GList*
marker_prefs_get_available_stylesheets();

GList*
marker_prefs_get_available_syntax_themes();

#endif

