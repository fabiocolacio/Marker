#ifndef __MARKER_APP_H__
#define __MARKER_APP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MARKER_APP_TYPE (marker_app_get_type ())

G_DECLARE_FINAL_TYPE(MarkerApp,
                     marker_app,
                     MARKER,
                     APP,
                     GtkApplication)

MarkerApp*
marker_app_new(void);

G_END_DECLS

#endif

