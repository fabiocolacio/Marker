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
marker_editor_window_refresh_web_view(MarkerEditorWindow* window);

void
marker_editor_window_open_file(MarkerEditorWindow* window,
                               GFile*              file);

void
marker_editor_window_save_file_as(MarkerEditorWindow* window,
                                  GFile*              file);
G_END_DECLS

#endif

