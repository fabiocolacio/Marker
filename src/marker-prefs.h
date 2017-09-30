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
marker_prefs_get_client_side_decorations();

void
marker_prefs_set_client_side_decorations(gboolean state);

gboolean
marker_prefs_get_gnome_appmenu();

void
marker_prefs_set_gnome_appmenu(gboolean state);

void
marker_prefs_load();

void
marker_prefs_show_window();

GList*
marker_prefs_get_available_stylesheets();

GList*
marker_prefs_get_available_syntax_themes();

#endif

