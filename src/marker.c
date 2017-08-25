
#include "marker-editor-window.h"

static void
activate(GtkApplication* app)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-app-menu.ui");
    GMenuModel* app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));
    gtk_application_set_app_menu(app, app_menu);
    g_object_unref(builder);

    MarkerEditorWindow* window = marker_editor_window_new(app);
    gtk_widget_show_all(GTK_WIDGET(window));
}

void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint)
{
    for (int i = 0; i < num_files; ++i)
    {
        GFile* file = files[i];
        g_object_ref(file);
        MarkerEditorWindow* win = marker_editor_window_new_from_file(app, file);
        gtk_widget_show_all(GTK_WIDGET(win));
    }
}

int
main(int    argc,
     char** argv)
{
    GtkApplication* app = gtk_application_new("com.github.fabiocolacio.marker",
                                              G_APPLICATION_HANDLES_OPEN);
    
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    g_signal_connect(app, "open", G_CALLBACK(marker_open), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}

