
#include "marker-editor-window.h"

static void
activate(GtkApplication* app,
         gpointer        user_data)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-app-menu.ui");
    GMenuModel* app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));
    gtk_application_set_app_menu(GTK_APPLICATION(app), app_menu);

    MarkerEditorWindow* window = marker_editor_window_new();
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

