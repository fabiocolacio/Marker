/*
 * marker.c
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
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

#include <gtk/gtk.h>
#include <stdlib.h>

#include <locale.h>
#include <glib/gi18n.h>

#include "marker-prefs.h"
#include "marker-window.h"
#include "marker-exporter.h"

#include "marker.h"

GtkApplication* app;

GtkApplication*
marker_get_app()
{
  return app;
}

static gboolean editor_mode_arg = FALSE;
static gboolean preview_mode_arg = FALSE;
static gboolean dual_pane_mode_arg = FALSE;
static gboolean dual_window_mode_arg = FALSE;
static gchar *outfile_arg = NULL;

static const GOptionEntry CLI_OPTIONS[] =
{
  { "editor", 'e', 0, G_OPTION_ARG_NONE, &editor_mode_arg, "Open in editor-only mode", NULL },
  { "preview", 'p', 0, G_OPTION_ARG_NONE, &preview_mode_arg, "Open in preview-only mode", NULL },
  { "dual-pane", 'd', 0, G_OPTION_ARG_NONE, &dual_pane_mode_arg, "Open in dual-pane mode", NULL },
  { "dual-window", 'w', 0, G_OPTION_ARG_NONE, &dual_window_mode_arg, "Open in dual-window mode", NULL },
  { "output", 'o', 0, G_OPTION_ARG_STRING, &outfile_arg, "Export the given markdown document as the given output file", NULL },
  { NULL }
};

const int APP_MENU_ACTION_ENTRIES_LEN = 6;

const GActionEntry APP_MENU_ACTION_ENTRIES[] =
{
  { "new", new_cb, NULL, NULL, NULL },
  { "prefs", marker_prefs_cb, NULL, NULL, NULL },
  { "shortcuts", marker_shortcuts_cb, NULL, NULL, NULL },
  { "help", marker_help_cb, NULL, NULL, NULL },
  { "about", marker_about_cb, NULL, NULL, NULL },
  { "quit", marker_quit_cb, NULL, NULL, NULL }
};

static void
marker_init(GtkApplication* app)
{
  marker_prefs_load();

  const gchar *quit_accels[] = { "<Ctrl>q", NULL };
  gtk_application_set_accels_for_action (app, "app.quit", quit_accels);

  if (gtk_application_prefers_app_menu(app))
  {
    GtkBuilder* builder =
      gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/ui/marker-appmenu.ui");

    GMenuModel* app_menu =
      G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));
    gtk_application_set_app_menu(app, app_menu);
    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                    APP_MENU_ACTION_ENTRIES,
                                    APP_MENU_ACTION_ENTRIES_LEN,
                                    app);

    g_object_unref(builder);
  }

  g_object_set(gtk_settings_get_default(),
               "gtk-application-prefer-dark-theme",
               marker_prefs_get_use_dark_theme(),
               NULL);
}

static void
activate(GtkApplication* app)
{
  marker_init (app);
  marker_create_new_window();
}

static gint
compare_files_by_name(gconstpointer a, gconstpointer b)
{
  GFile *file_a = G_FILE(a);
  GFile *file_b = G_FILE(b);
  
  gchar *name_a = g_file_get_basename(file_a);
  gchar *name_b = g_file_get_basename(file_b);
  
  gint result = g_strcmp0(name_a, name_b);
  
  g_free(name_a);
  g_free(name_b);
  
  return result;
}

static void
marker_open_directory(GtkApplication* app, GFile* directory)
{
  GError *error = NULL;
  GFileEnumerator *enumerator = g_file_enumerate_children(directory,
                                                          G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                          G_FILE_QUERY_INFO_NONE,
                                                          NULL,
                                                          &error);
  
  if (error || !enumerator) {
    if (error) {
      g_warning("Failed to enumerate directory: %s", error->message);
      g_error_free(error);
    }
    return;
  }
  
  GFileInfo *info;
  GList *markdown_files = NULL;
  
  while ((info = g_file_enumerator_next_file(enumerator, NULL, &error)) != NULL) {
    const gchar *name = g_file_info_get_name(info);
    
    // Check if file has markdown extension
    if (g_str_has_suffix(name, ".md") || 
        g_str_has_suffix(name, ".markdown") ||
        g_str_has_suffix(name, ".mdown") ||
        g_str_has_suffix(name, ".mkd") ||
        g_str_has_suffix(name, ".mkdn")) {
      GFile *child = g_file_get_child(directory, name);
      markdown_files = g_list_append(markdown_files, child);
    }
    
    g_object_unref(info);
  }
  
  if (error) {
    g_warning("Error during directory enumeration: %s", error->message);
    g_error_free(error);
  }
  
  g_object_unref(enumerator);
  
  // Sort files alphabetically
  markdown_files = g_list_sort(markdown_files, compare_files_by_name);
  
  // Open all markdown files using the same approach as glob
  if (markdown_files) {
    GList *l;
    gboolean first = TRUE;
    
    for (l = markdown_files; l != NULL; l = l->next) {
      GFile *file = G_FILE(l->data);
      
      if (first) {
        marker_create_new_window_from_file(file);
        first = FALSE;
      } else {
        g_object_ref(file);
        marker_open_file(file);
      }
      
      g_object_unref(file);
    }
    
    g_list_free(markdown_files);
  } else {
    // If no markdown files found, create empty window
    marker_create_new_window();
  }
}

static void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint)
{
  g_application_hold (G_APPLICATION (app));
  marker_init(app);

  if (outfile_arg != NULL) {
    g_autoptr (GFile) outfile = g_file_new_for_commandline_arg (outfile_arg);
    g_autofree gchar *outfile_path = g_file_get_path (outfile);
    g_autofree gchar *infile_path = g_file_get_path (files[0]);
    marker_exporter_export (infile_path, outfile_path);
    exit (0);
  }

  if (!files || num_files <= 0) {
    g_application_release (G_APPLICATION (app));
    return;
  }
 
  for (int i = 0; i < num_files; ++i)
  {
    GFile* file = files[i];
    if (!file) {
      continue;
    }
    
    // Check if this is a directory
    GFileType file_type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);
    
    if (file_type == G_FILE_TYPE_DIRECTORY) {
      marker_open_directory(app, file);
    } else {
      g_object_ref(file);
      marker_open_file(file);
    }
  }
  g_application_release (G_APPLICATION (app));
}

void
new_cb(GSimpleAction* action,
       GVariant*      parameter,
       gpointer       user_data)
{
  marker_create_new_window();
}

void
marker_prefs_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data)
{
  marker_prefs_show_window();
}

void
marker_help_cb(GSimpleAction* action,
               GVariant*      parameter,
               gpointer       user_data)
{
  GtkWindow *window;
  GtkApplication *application = user_data;
  GError *error = NULL;

  window = gtk_application_get_active_window (application);
  gtk_show_uri_on_window (window, "help:Marker",
                          gtk_get_current_event_time (), &error);
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
  gtk_about_dialog_set_comments(dialog, _("A markdown editor for GNOME"));
  gtk_about_dialog_set_website(dialog, "https://github.com/fabiocolacio/Marker");
  gtk_about_dialog_set_website_label(dialog, _("Report bugs and ideas on github"));
  gtk_about_dialog_set_copyright(dialog, "Copyright 2017-2018 Fabio Colacio");
  gtk_about_dialog_set_license_type(dialog, GTK_LICENSE_GPL_3_0);
  gtk_about_dialog_set_authors(dialog, authors);
  gtk_about_dialog_set_artists(dialog, artists);
  gtk_about_dialog_set_translator_credits(dialog, _("translator-credits"));

  GtkWindow* window = gtk_application_get_active_window(app);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
marker_quit_cb(GSimpleAction*  action,
               GVariant*       parameter,
               gpointer        user_data)
{
  marker_quit();
}

void
marker_shortcuts_cb(GSimpleAction* action,
                    GVariant*      parameter,
                    gpointer       user_data)
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/ui/marker-shortcuts-window.ui");

  GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "shortcuts"));

  GtkWindow* parent = gtk_application_get_active_window(app);

	if (GTK_WINDOW(parent) != gtk_window_get_transient_for(GTK_WINDOW(dialog)))
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	}

	gtk_widget_show_all(dialog);
  gtk_window_present(GTK_WINDOW(dialog));

  g_object_unref(builder);
}

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
  MarkerWindow *window = marker_window_new (app);
  gtk_widget_show (GTK_WIDGET (window));
}

void
marker_create_new_window_from_file (GFile *file)
{
  MarkerWindow *window = marker_window_new_from_file (app, file);
  gtk_widget_show (GTK_WIDGET (window));

  if (preview_mode_arg)
  {
    MarkerEditor *editor = marker_window_get_active_editor (window);
    marker_editor_set_view_mode (editor, PREVIEW_ONLY_MODE);
  }
  else if (editor_mode_arg)
  {
    MarkerEditor *editor = marker_window_get_active_editor (window);
    marker_editor_set_view_mode (editor, EDITOR_ONLY_MODE);
  }
  else if (dual_pane_mode_arg)
  {
    MarkerEditor *editor = marker_window_get_active_editor (window);
    marker_editor_set_view_mode (editor, DUAL_PANE_MODE);
  }
  else if (dual_window_mode_arg)
  {
    MarkerEditor *editor = marker_window_get_active_editor (window);
    marker_editor_set_view_mode (editor, DUAL_WINDOW_MODE);
  }
}


void
marker_open_file (GFile *file)
{
  GList *windows = gtk_application_get_windows(app);
  if (g_list_last(windows))
  {
    if (MARKER_IS_WINDOW(windows->data))
    {
      MarkerWindow *window = MARKER_WINDOW(windows->data);
      marker_window_new_editor_from_file(window, file);
      return;
    }
  }

  marker_create_new_window_from_file(file);

}

void
marker_quit()
{
  GtkApplication *app = marker_get_app();
  GList *windows = gtk_application_get_windows(app);
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW(item->data))
    {
      MarkerWindow *window = item->data;
      marker_window_try_close (window);
    }
  }
}

int
main(int    argc,
     char** argv)
{

  /* Initialize gettext support */
  bindtextdomain ("marker", LOCALE_DIR);
  bind_textdomain_codeset ("marker", "UTF-8");
  textdomain ("marker");

  app = gtk_application_new("com.github.fabiocolacio.marker",
                            G_APPLICATION_HANDLES_OPEN);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(app, "open", G_CALLBACK(marker_open), NULL);

  g_application_add_main_option_entries (G_APPLICATION(app), CLI_OPTIONS);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
