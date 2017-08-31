#include <gtk/gtk.h>

#include "marker-prefs.h"
#include "marker-editor-window.h"
#include "marker-utils.h"
#include "marker.h"

static void
prefs_activated(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       data)
{
  GtkApplication* app = data;
  marker_prefs_show_window(app);
}

static void
about_activated(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       data)
{

}

static void
quit_activated(GSimpleAction* action,
               GVariant*      parameter,
               gpointer       data)
{
  GtkApplication* app = data;
  marker_quit(app);
}

static GActionEntry app_entries[] =
{
  { "prefs", prefs_activated, NULL, NULL, NULL },
  { "about", about_activated, NULL, NULL, NULL },
  { "quit", quit_activated, NULL, NULL, NULL }
};

static void
init_app_menu(GtkApplication* app)
{
  GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-app-menu.ui");
  GMenuModel* app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));
  gtk_application_set_app_menu(app, app_menu);
  g_object_unref(builder);
    
  g_action_map_add_action_entries(G_ACTION_MAP(app),
                                  app_entries,
                                  G_N_ELEMENTS(app_entries),
                                  app);
}

static void
activate(GtkApplication* app)
{
  init_app_menu(app);

  MarkerEditorWindow* window = marker_editor_window_new(app);
  gtk_widget_show_all(GTK_WIDGET(window));
}

void
marker_quit(GtkApplication* app)
{
  GList* windows = gtk_application_get_windows(app);
  if (windows)
  {
    for (; windows != NULL; windows = windows->next)
    {
      MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
      marker_editor_window_try_close(window);
    }
  }
}

void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint)
{
  init_app_menu(app);

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

