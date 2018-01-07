#include <gtk/gtk.h>

#include "marker-prefs.h"
#include "marker-editor-window.h"

#include "marker.h"

GtkApplication* app;

GtkApplication*
marker_get_app()
{
  return app;
}

static void
marker_init(GtkApplication* app)
{
  marker_prefs_load();
  if (marker_prefs_get_gnome_appmenu())
  {
    GtkBuilder* builder =
      gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/ui/editor-window.ui");
      
    GMenuModel* app_menu =
      G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));  
    gtk_application_set_app_menu(app, app_menu);
    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                    APP_MENU_ACTION_ENTRIES,
                                    3,
                                    app);
    
    g_object_unref(builder);
  }
}

static void
activate(GtkApplication* app)
{
  marker_init(app);
  marker_create_new_window();
}

static void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint)
{
  marker_init(app);
  for (int i = 0; i < num_files; ++i)
  {
    GFile* file = files[i];
    g_object_ref(file);
    marker_create_new_window_from_file(file);
  }
}

void
marker_prefs_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data)
{
  marker_prefs_show_window();
}

void
marker_about_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data)
{
  const gchar* authors[] = {
    "Fabio Colacio",
    "Martino Ferrari",
    NULL
  };

  const gchar* artists[] = {
    "Fabio Colacio",
    NULL
  };

  GtkAboutDialog* dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
  
  gtk_about_dialog_set_logo_icon_name(dialog, "com.github.fabiocolacio.marker");
  gtk_about_dialog_set_program_name(dialog, "Marker");
  gtk_about_dialog_set_version(dialog, MARKER_VERSION);
  gtk_about_dialog_set_comments(dialog, "A markdown editor for GNOME");
  gtk_about_dialog_set_website(dialog, "https://github.com/fabiocolacio/Marker");
  gtk_about_dialog_set_website_label(dialog, "Marker on Github");
  gtk_about_dialog_set_copyright(dialog, "Copyright 2017-2018 Fabio Colacio");
  gtk_about_dialog_set_license_type(dialog, GTK_LICENSE_GPL_3_0);
  gtk_about_dialog_set_authors(dialog, authors);
  gtk_about_dialog_set_artists(dialog, artists);
  
  GtkWindow* window = gtk_application_get_active_window(app);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
marker_quit_cb(GSimpleAction*  action,
               GVariant*      parameter,
               gpointer       user_data)
{
  marker_quit();
}

GActionEntry APP_MENU_ACTION_ENTRIES[] =
{
  { "quit", marker_quit_cb, NULL, NULL, NULL },
  { "about", marker_about_cb, NULL, NULL, NULL },
  { "prefs", marker_prefs_cb, NULL, NULL, NULL }
};

gboolean
marker_has_app_menu()
{
  if (gtk_application_get_app_menu(app))
  {
    return TRUE;
  }
  return FALSE;
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

