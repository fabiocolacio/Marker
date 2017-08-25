#ifndef __MARKER_H__
#define __MARKER_H__

#include <gtk/gtk.h>

void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint);

#endif

