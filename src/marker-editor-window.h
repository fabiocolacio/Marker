#ifndef __MARKER_EDITOR_WINDOW_H__
#define __MARKER_EDITOR_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MARKER_TYPE_EDITOR_WINDOW (marker_editor_window_get_type ())

G_DECLARE_FINAL_TYPE(MarkerEditorWindow,
                     marker_editor_window,
                     MARKER,
                     EDITOR_WINDOW,
                     GtkApplicationWindow)

MarkerEditorWindow*
marker_editor_window_new(GtkApplication* app);

MarkerEditorWindow*
marker_editor_window_new_from_file(GtkApplication* app,
                                   GFile*          file);

void
marker_editor_window_open_file(MarkerEditorWindow* window,
                               GFile*              file);
                               
void
marker_editor_window_save_file(MarkerEditorWindow* window,
                               GFile*              file);

gchar*
marker_editor_window_get_markdown(MarkerEditorWindow* window);

void
marker_editor_window_refresh_preview(MarkerEditorWindow* window);

void
marker_editor_window_try_close(MarkerEditorWindow* window);

void
marker_editor_window_set_syntax_theme(MarkerEditorWindow* window,
                                      const char*         theme);

void
marker_editor_window_set_show_line_numbers(MarkerEditorWindow* window,
                                           gboolean            state);
                                           
void
marker_editor_window_set_highlight_current_line(MarkerEditorWindow* window,
                                                gboolean            state);

void
marker_editor_window_set_wrap_text(MarkerEditorWindow* window,
                                   gboolean            state);

void
marker_editor_window_set_show_right_margin(MarkerEditorWindow* window,
                                           gboolean            state);

G_END_DECLS

#endif

