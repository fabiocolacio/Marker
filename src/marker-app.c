
#include "marker-app.h"
#include "marker-editor-window.h"

struct _MarkerApp
{
    GtkApplication parent;
};

G_DEFINE_TYPE(MarkerApp, marker_app, GTK_TYPE_APPLICATION);

static void
marker_app_activate(GApplication* app)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-app-menu.ui");
    GMenuModel* app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));
    gtk_application_set_app_menu(GTK_APPLICATION(app), app_menu);
    g_object_unref(builder);

    MarkerEditorWindow* window = marker_editor_window_new();
    gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));
    gtk_widget_show_all(GTK_WIDGET(window));
}

static void
marker_app_open(GApplication* app,
                GFile**       files,
                gint          num_files,
                const gchar*  hint)
{
    printf("num files %d\n", num_files);
    for (int i = 0; i < num_files; ++i)
    {
        GFile* file = files[i];
        GtkWindow* win = GTK_WINDOW(marker_editor_window_new_from_file(file));
        gtk_application_add_window(GTK_APPLICATION(app), win);
        gtk_widget_show_all(GTK_WIDGET(win));
    }
}

static void
marker_app_init(MarkerApp* self)
{
    
}

static void
marker_app_class_init(MarkerAppClass* class)
{
    G_APPLICATION_CLASS(class)->activate = marker_app_activate;
    G_APPLICATION_CLASS(class)->open = marker_app_open;
}

MarkerApp*
marker_app_new(void)
{
    return g_object_new(MARKER_APP_TYPE,
                        "application-id", "com.github.fabiocolacio.marker",
                        "flags", G_APPLICATION_HANDLES_OPEN,
                        NULL);
}

