#ifndef __MARKER_H__
#define __MARKER_H__

#include <gtk/gtk.h>

void
marker_create_new_window();

void
marker_create_new_window_from_file(GFile* file);

void
marker_quit();

#endif

