#ifndef __MARKER_PREFS_H__
#define __MARKER_PREFS_H__

typedef struct {
  // Editor //
  char syntax_theme[256];
  gboolean show_line_numbers;
  gboolean highlight_current_line;
  gboolean wrap_text;
  gboolean show_right_margin;
  guint right_margin_position;
  
  // Preview //
  char css_theme[256];
  
  // Window //
  gboolean single_view_mode;
  gboolean client_side_decorations;
  gboolean gnome_appmenu;
} MarkerPrefs;

void
marker_prefs_load();

void
marker_prefs_save();

void
marker_prefs_show_window();

GList*
marker_prefs_get_available_stylesheets();

GList*
marker_prefs_get_available_syntax_themes();

#endif

