#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>

static void
activate(GtkApplication* app,
         gpointer        user_data)
{
    GtkWidget* window;
    GtkWidget* paned;
    GtkWidget* source_view;
    GtkWidget* web_view;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    source_view = gtk_source_view_new();
    web_view = webkit_web_view_new();
    gtk_window_set_title(GTK_WINDOW(window), "Editing: Untitled");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_paned_add1(GTK_PANED(paned), source_view);
    gtk_paned_add2(GTK_PANED(paned), web_view);
    gtk_container_add(GTK_CONTAINER(window), paned);
    gtk_application_add_window(app, GTK_WINDOW(window));
    gtk_widget_show_all(window);
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

