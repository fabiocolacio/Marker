#include <gtk/gtk.h>

#include "marker-editor-window.h"

#include "marker.h"

GtkApplication* app;

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
marker_quit(GtkApplication* app)
{

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

