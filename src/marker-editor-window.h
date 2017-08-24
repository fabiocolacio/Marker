#ifndef __MARKER_EDITOR_WINDOW_H__
#define __MARKER_EDITOR_WINDOW_H__

#include <gtk/gtk.h>

#define MARKER_TYPE_EDITOR_WINDOW (marker_editor_window_get_type ())

G_DECLARE_FINAL_TYPE(MarkerEditorWindow,
                     marker_editor_window,
                     MARKER,
                     EDITOR_WINDOW,
                     GtkWindow)

MarkerEditorWindow*
marker_editor_window_new(void);

MarkerEditorWindow*
marker_editor_window_new_from_file(GFile*);

void
marker_editor_window_refresh_web_view(MarkerEditorWindow*);

void
marker_editor_window_open_file(MarkerEditorWindow*,
                               GFile*);

void
marker_editor_window_save_file_as(MarkerEditorWindow*,
                                  GFile*);

#endif

