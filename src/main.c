#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>

#include "marker-editor-window.h"

static void
activate(GtkApplication* app,
         gpointer        user_data)
{
    MarkerEditorWindow* window;
    window = marker_editor_window_new();
    gtk_application_add_window(app, GTK_WINDOW(window));
    gtk_widget_show_all(GTK_WIDGET(window));
}

int
main(int   argc,
     char* argv[])
{
    GtkApplication* app;
    int status;
    app = gtk_application_new("com.github.fabiocolacio.marker",
                              G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

