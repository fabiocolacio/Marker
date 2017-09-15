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

G_END_DECLS

#endif

