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
#include <adwaita.h>
#include <gtksourceview/gtksource.h>
#include <libspelling.h>
#include <stdlib.h>

#include <locale.h>
#include <glib/gi18n.h>

#include "marker-prefs.h"
#include "marker-window.h"
#include "marker-exporter.h"

#include "marker.h"

static GtkApplication *app = NULL;
static gboolean initialized = FALSE;

GtkApplication*
marker_get_app (void)
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

static const GActionEntry APP_ACTION_ENTRIES[] =
{
  { "new", new_cb, NULL, NULL, NULL },
  { "prefs", marker_prefs_cb, NULL, NULL, NULL },
  { "shortcuts", marker_shortcuts_cb, NULL, NULL, NULL },
  { "help", marker_help_cb, NULL, NULL, NULL },
  { "about", marker_about_cb, NULL, NULL, NULL },
  { "quit", marker_quit_cb, NULL, NULL, NULL }
};

static void
marker_init (GtkApplication *application)
{
  if (initialized)
    return;
  initialized = TRUE;

  marker_prefs_load ();

  g_set_application_name ("Marker");
  gtk_window_set_default_icon_name (APP_ID);

  g_action_map_add_action_entries (G_ACTION_MAP (application),
                                   APP_ACTION_ENTRIES,
                                   G_N_ELEMENTS (APP_ACTION_ENTRIES),
                                   application);

  const gchar *quit_accels[] = { "<Ctrl>q", NULL };
  gtk_application_set_accels_for_action (application, "app.quit", quit_accels);

  marker_prefs_apply_color_scheme ();
}

static void
activate (GtkApplication *application)
{
  marker_init (application);
  marker_create_new_window ();
}

static void
marker_open (GtkApplication *application,
             GFile         **files,
             gint            num_files,
             const gchar    *hint)
{
  g_application_hold (G_APPLICATION (application));
  marker_init (application);

  if (outfile_arg != NULL) {
    g_autoptr (GFile) outfile = g_file_new_for_commandline_arg (outfile_arg);
    g_autofree gchar *outfile_path = g_file_get_path (outfile);
    g_autofree gchar *infile_path = g_file_get_path (files[0]);
    marker_exporter_export (infile_path, outfile_path);
    exit (0);
  }

  for (int i = 0; i < num_files; ++i)
  {
    marker_open_file (files[i]);
  }
  g_application_release (G_APPLICATION (application));
}

void
new_cb (GSimpleAction *action,
        GVariant      *parameter,
        gpointer       user_data)
{
  marker_create_new_window ();
}

void
marker_prefs_cb (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  marker_prefs_show_window ();
}

void
marker_help_cb (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  GtkWindow *window = gtk_application_get_active_window (app);
  g_autoptr (GtkUriLauncher) launcher = gtk_uri_launcher_new ("help:Marker");
  gtk_uri_launcher_launch (launcher, window, NULL, NULL, NULL);
}

void
marker_about_cb (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  const gchar *developers[] = {
    "Fabio Colacio",
    "Martino Ferrari",
    NULL
  };

  const gchar *designers[] = {
    "Fabio Colacio",
    NULL
  };

  AdwDialog *dialog = adw_about_dialog_new ();
  AdwAboutDialog *about = ADW_ABOUT_DIALOG (dialog);

  adw_about_dialog_set_application_name (about, "Marker");
  adw_about_dialog_set_application_icon (about, APP_ID);
  adw_about_dialog_set_version (about, MARKER_VERSION);
  adw_about_dialog_set_comments (about, _("A markdown editor for GNOME"));
  adw_about_dialog_set_website (about, "https://github.com/fabiocolacio/Marker");
  adw_about_dialog_set_issue_url (about, "https://github.com/fabiocolacio/Marker/issues");
  adw_about_dialog_set_copyright (about, "Copyright 2017-2020 Fabio Colacio");
  adw_about_dialog_set_license_type (about, GTK_LICENSE_GPL_3_0);
  adw_about_dialog_set_developers (about, developers);
  adw_about_dialog_set_designers (about, designers);
  adw_about_dialog_set_translator_credits (about, _("translator-credits"));

  GtkWindow *window = gtk_application_get_active_window (app);
  adw_dialog_present (dialog, window ? GTK_WIDGET (window) : NULL);
}

void
marker_quit_cb (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  marker_quit ();
}

void
marker_shortcuts_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/github/fabiocolacio/marker/ui/marker-shortcuts-dialog.ui");

  AdwDialog *dialog = ADW_DIALOG (gtk_builder_get_object (builder, "shortcuts"));
  GtkWindow *parent = gtk_application_get_active_window (app);

  adw_dialog_present (dialog, parent ? GTK_WIDGET (parent) : NULL);

  g_object_unref (builder);
}

void
marker_create_new_window (void)
{
  MarkerWindow *window = marker_window_new (app);
  gtk_window_present (GTK_WINDOW (window));
}

void
marker_create_new_window_from_file (GFile *file)
{
  MarkerWindow *window = marker_window_new_from_file (app, file);
  gtk_window_present (GTK_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
    return;

  if (preview_mode_arg)
    marker_editor_set_view_mode (editor, PREVIEW_ONLY_MODE);
  else if (editor_mode_arg)
    marker_editor_set_view_mode (editor, EDITOR_ONLY_MODE);
  else if (dual_pane_mode_arg)
    marker_editor_set_view_mode (editor, DUAL_PANE_MODE);
  else if (dual_window_mode_arg)
    marker_editor_set_view_mode (editor, DUAL_WINDOW_MODE);
}

void
marker_open_file (GFile *file)
{
  GList *windows = gtk_application_get_windows (app);
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW (item->data))
    {
      marker_window_new_editor_from_file (MARKER_WINDOW (item->data), file);
      return;
    }
  }

  marker_create_new_window_from_file (file);
}

void
marker_quit (void)
{
  GList *windows = g_list_copy (gtk_application_get_windows (app));
  for (GList *item = windows; item != NULL; item = item->next)
  {
    if (MARKER_IS_WINDOW (item->data))
    {
      marker_window_try_close (MARKER_WINDOW (item->data));
    }
  }
  g_list_free (windows);
}

int
main (int    argc,
      char **argv)
{
  /* Initialize gettext support */
  bindtextdomain ("marker", LOCALE_DIR);
  bind_textdomain_codeset ("marker", "UTF-8");
  textdomain ("marker");

  g_set_prgname (APP_ID);

  gtk_source_init ();
  spelling_init ();

  app = GTK_APPLICATION (adw_application_new (APP_ID, G_APPLICATION_HANDLES_OPEN));
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_signal_connect (app, "open", G_CALLBACK (marker_open), NULL);

  g_application_add_main_option_entries (G_APPLICATION (app), CLI_OPTIONS);

  int status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  gtk_source_finalize ();

  return status;
}
