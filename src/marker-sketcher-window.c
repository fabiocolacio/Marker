/*
 * marker-sketcher-window.c
 *
 * Copyright (C) 2017 - 2018 Marker Project
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Marker; see the file LICENSE.md. If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */


#include "marker-sketcher-window.h"


struct _MarkerSketcherWindow
{
  GtkApplicationWindow        parent_instance;
};

G_DEFINE_TYPE(MarkerSketcherWindow, marker_sketcher_window, GTK_TYPE_APPLICATION_WINDOW)

MarkerSketcherWindow*  
marker_sketcher_window_new (GtkApplication * app)
{
    return g_object_new(MARKER_TYPE_SKETCHER_WINDOW, "application", app, NULL);
}


static void
marker_sketcher_window_class_init(MarkerSketcherWindowClass* class)
{
  
}

static void
marker_sketcher_window_init (MarkerSketcherWindow *sketcher)
{

}


static void
init_ui ()
{
    GtkBuilder* builder =
    gtk_builder_new_from_resource(
      "/com/github/fabiocolacio/marker/ui/sketcher-window.ui");
    GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "sketcher-window"));
        gtk_widget_show_all(GTK_WIDGET(window));
    gtk_window_present(window);
    g_object_unref(builder);
}


void
marker_sketcher_window_show()
{
    init_ui();    
}