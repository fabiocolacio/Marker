#include <gtk/gtk.h>

#include "marker-editor-window.h"

#include "marker.h"

GtkApplication* app;

GtkApplication*
marker_get_app()
{
  return app;
}

static void
activate(GtkApplication* app)
{
  marker_create_new_window();
}

static void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint)
{
  for (int i = 0; i < num_files; ++i)
  {
    GFile* file = files[i];
    g_object_ref(file);
    marker_create_new_window_from_file(file);
  }
}

void
prefs_cb(GSimpleAction* action,
         GVariant*      parameter,
         gpointer       user_data)
{

}

void
about_cb(GSimpleAction* action,
         GVariant*      parameter,
         gpointer       user_data)
{

}

void
quit_cb(GSimpleAction*  action,
        GVariant*      parameter,
        gpointer       user_data)
{
  marker_quit();
}

GActionEntry APP_MENU_ACTION_ENTRIES[] =
{
  { "quit", quit_cb, NULL, NULL, NULL },
  { "about", about_cb, NULL, NULL, NULL },
  { "prefs", prefs_cb, NULL, NULL, NULL }
};

void
marker_create_new_window()
{
  MarkerEditorWindow* window = marker_editor_window_new(app);
  gtk_widget_show_all(GTK_WIDGET(window));
}

void
marker_create_new_window_from_file(GFile* file)
{
  MarkerEditorWindow* window = marker_editor_window_new_from_file(app, file);
  gtk_widget_show_all(GTK_WIDGET(window));
}

void
marker_quit()
{
  GList *windows = gtk_application_get_windows(app), *window = NULL;
  if (windows)
  {
    for (window = windows; windows != NULL; windows = windows->next)
    {
      if (MARKER_IS_EDITOR_WINDOW(window->data))
      {
        marker_editor_window_try_close(MARKER_EDITOR_WINDOW(window->data));
      }
      else
      {
        gtk_widget_destroy(GTK_WIDGET(window->data));
      }
    }
  }
}

int
main(int    argc,
     char** argv)
{
  app = gtk_application_new("com.github.fabiocolacio.marker",
                            G_APPLICATION_HANDLES_OPEN);
  
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(app, "open", G_CALLBACK(marker_open), NULL);
    
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  
  g_object_unref(app);
    
  return status;
}

