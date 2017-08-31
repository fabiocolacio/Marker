#ifndef __MARKER_PREFS__
#define __MARKER_PREFS__

typedef struct {
  char syntax_theme[50];
  char css_theme[50];
  gboolean show_line_numbers;
  gboolean highlight_current_line;
  gboolean wrap_text;
  gboolean show_right_margin;
} MarkerPrefs;

char*
marker_prefs_get_syntax_theme();

char*
marker_prefs_get_css_theme();

gboolean
marker_prefs_get_show_line_numbers();

gboolean
marker_prefs_get_highlight_current_line();

gboolean
marker_prefs_get_wrap_text();

gboolean
marker_prefs_get_show_right_margin();

void
marker_prefs_load(GtkApplication* app);

void
marker_prefs_save();

void
marker_prefs_show_window(GtkApplication* app);

#endif

