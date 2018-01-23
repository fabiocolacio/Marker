/*
 * marker-window.c
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

#include "marker.h"
#include "marker-editor.h"
#include "marker-exporter.h"
#include "marker-sketcher-window.h"

#include "marker-window.h"

struct _MarkerWindow
{
  GtkApplicationWindow  parent_instance;
  
  GtkBox               *header_box;
  GtkHeaderBar         *header_bar;
  GtkButton            *zoom_original_btn;
  
  gboolean              is_fullscreen;
  GtkButton            *unfullscreen_btn;
  
  GtkBox               *vbox;
  MarkerEditor         *editor;
};

G_DEFINE_TYPE (MarkerWindow, marker_window, GTK_TYPE_APPLICATION_WINDOW);

/**
 * show_unsaved_documents_warning:
 * @parent The parent window for this dialog to be transient for, or NULL
 *
 * Shows a dialog asking the user to proceed closing a document without saving,
 * or to cancel the close operation
 * 
 * Returns: TRUE if the user would like to proceed without saving. FALSE if the user
 * wants to cancel the operation.
 */
static gboolean
show_unsaved_documents_warning (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  GFile *file = marker_editor_get_file (editor);
  g_autofree gchar *warning_message = NULL;
  
  if (G_IS_FILE (file))
  {
    g_autofree gchar *filename = g_file_get_basename (file);
    warning_message = g_strdup_printf ("<span weight='bold' size='larger'>"
                                       "Discard changes to the document '%s'?"
                                       "</span>\n\n"
                                       "The document has unsaved changes "
                                       "that will be lost if it is closed now.", filename);
  }
  else
  {
    warning_message = g_strdup ("<span weight='bold' size='larger'>"
                                "Discard changes to the document?"
                                "</span>\n\n"
                                "The document has unsaved changes "
                                "that will be lost if it is closed now.");
  }

  GtkWidget *dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW (window),
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_QUESTION,
                                                         GTK_BUTTONS_OK_CANCEL,
                                                         warning_message);
                                           
  gint response = gtk_dialog_run(GTK_DIALOG(dialog));
  
  gtk_widget_destroy (GTK_WIDGET (dialog));
  
  if (response == GTK_RESPONSE_OK)
    return TRUE;
  
  return FALSE;
}

static void
action_zoom_out (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  MarkerPreview *preview = marker_editor_get_preview (editor);
  marker_preview_zoom_out (preview);
}

static void
action_zoom_original (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  MarkerPreview *preview = marker_editor_get_preview (editor);
  marker_preview_zoom_original (preview);
}

static void
action_zoom_in (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  MarkerPreview *preview = marker_editor_get_preview (editor);
  marker_preview_zoom_in (preview);
}

static void
action_save_as (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  MarkerWindow *window = user_data;
  marker_window_save_active_file_as (window);
}

static void
action_export (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  MarkerWindow *window = user_data;
  marker_exporter_show_export_dialog (window);
}

static void
action_print (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  MarkerPreview *preview = marker_editor_get_preview (editor);
  marker_preview_run_print_dialog (preview, GTK_WINDOW (window));
}

static void
action_editor_only_mode (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, EDITOR_ONLY_MODE);
}

static void
action_preview_only_mode (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, PREVIEW_ONLY_MODE);
}

static void
action_dual_pane_mode (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, DUAL_PANE_MODE);
}

static void
action_dual_window_mode (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, DUAL_WINDOW_MODE);
}

static const GActionEntry WINDOW_ACTIONS[] =
{
  { "zoomout", action_zoom_out, NULL, NULL, NULL },
  { "zoomoriginal", action_zoom_original, NULL, NULL, NULL },
  { "zoomin", action_zoom_in, NULL, NULL, NULL },
  { "saveas", action_save_as, NULL, NULL, NULL },
  { "export", action_export, NULL, NULL, NULL },
  { "print", action_print, NULL, NULL, NULL },
  { "editoronlymode", action_editor_only_mode, NULL, NULL, NULL },
  { "previewonlymode", action_preview_only_mode, NULL, NULL, NULL },
  { "dualpanemode", action_dual_pane_mode, NULL, NULL, NULL },
  { "dualwindowmode", action_dual_window_mode, NULL, NULL, NULL }
};

static gboolean
key_pressed_cb (GtkWidget   *widget,
                GdkEventKey *event,
                gpointer     user_data)
{
  MarkerWindow *window = MARKER_WINDOW (widget);
  MarkerEditor *editor = marker_window_get_active_editor (window);
  MarkerSourceView *source_view = marker_editor_get_source_view (editor);

  gboolean ctrl_pressed = (event->state & GDK_CONTROL_MASK);
  if (ctrl_pressed)
  {
    switch (event->keyval)
    {
      case GDK_KEY_i:
        marker_source_view_surround_selection_with (source_view, "*");
        break;
        
      case GDK_KEY_m:
        marker_source_view_surround_selection_with (source_view, "``");
        break;
        
      case GDK_KEY_b:
        marker_source_view_surround_selection_with (source_view, "**");
        break;
    
      case GDK_KEY_q:
        marker_quit ();
        break;
    
      case GDK_KEY_n:
        marker_editor_new_file(editor);
        break;
      
      case GDK_KEY_N:
        marker_create_new_window ();
        break;
      
      case GDK_KEY_w:
        marker_window_close_current_document (window);
        break;
        
      case GDK_KEY_W:
        marker_window_try_close (window);
        break;
        
      case GDK_KEY_r:
        marker_editor_refresh_preview (editor);
        break;
      
      case GDK_KEY_o:
        marker_window_open_file (window);
        break;
      
      case GDK_KEY_k:
        marker_window_open_sketcher (window);
        break;
      
      case GDK_KEY_S:
        marker_window_save_active_file_as (window);
        break;
      
      case GDK_KEY_s:
        marker_window_save_active_file (window);
        break;
      
      case GDK_KEY_p:
        marker_preview_run_print_dialog (marker_editor_get_preview (editor), GTK_WINDOW (window));
        break;
        
      case GDK_KEY_1:
        marker_editor_set_view_mode (editor, EDITOR_ONLY_MODE);
        break;
      
      case GDK_KEY_2:
        marker_editor_set_view_mode (editor, PREVIEW_ONLY_MODE);
        break;
      
      case GDK_KEY_3:
        marker_editor_set_view_mode (editor, DUAL_PANE_MODE);
        break;
      
      case GDK_KEY_4:
        marker_editor_set_view_mode (editor, DUAL_WINDOW_MODE);
        break;
    }
  }
  else
  {
    switch (event->keyval)
    {
      case GDK_KEY_F11:
        marker_window_toggle_fullscreen (window);
        break;
    }
  }

  return FALSE;
}

static void
title_changed_cb (MarkerEditor *editor,
                  const gchar  *title,
                  gpointer      user_data)
{
  MarkerWindow *window = user_data;
  gtk_header_bar_set_title (window->header_bar, title);
}

static void
subtitle_changed_cb (MarkerEditor *editor,
                     const gchar  *subtitle,
                     gpointer      user_data)
{
  MarkerWindow *window = user_data;
  gtk_header_bar_set_subtitle (window->header_bar, subtitle);
}

static void
preview_zoom_changed_cb (MarkerPreview *preview,
                         gpointer       user_data)
{
  MarkerWindow *window = user_data;
  const gdouble zoom_percentage = 100 * webkit_web_view_get_zoom_level (WEBKIT_WEB_VIEW (preview));
  g_autofree gchar *zoom_level_str = g_strdup_printf ("%.0f%%", zoom_percentage);
  gtk_button_set_label (window->zoom_original_btn, zoom_level_str);
}

static gboolean
window_deleted_event_cb (GtkWidget *widget,
                         GdkEvent  *event,
                         gpointer   user_data)
{
  MarkerWindow *window = user_data;
  marker_window_try_close (window);
  return TRUE;
}

void
marker_window_fullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));  
  g_return_if_fail (!marker_window_is_fullscreen (window));
  
  window->is_fullscreen = TRUE;
  gtk_window_fullscreen (GTK_WINDOW (window));
  
  GtkBox * const header_box = window->header_box;
  GtkBox * const vbox = window->vbox;
  GtkWidget * const header_bar = GTK_WIDGET (window->header_bar);
  GtkWidget * const editor = GTK_WIDGET (window->editor);
  
  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (header_box), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), FALSE);
  gtk_widget_show (GTK_WIDGET (window->unfullscreen_btn));
  
  g_object_ref (editor);
  gtk_container_remove (GTK_CONTAINER (vbox), editor);
  
  gtk_box_pack_start (vbox, header_bar, FALSE, TRUE, 0);
  gtk_box_pack_start (vbox, editor, TRUE, TRUE, 0);
}

void
marker_window_unfullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  g_return_if_fail (marker_window_is_fullscreen (window));
  
  window->is_fullscreen = FALSE;
  gtk_window_unfullscreen (GTK_WINDOW (window));
  
  GtkBox * const vbox = window->vbox;
  GtkBox * const header_box = window->header_box;
  GtkWidget * const header_bar = GTK_WIDGET (window->header_bar);
  
  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (vbox), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), TRUE);
  gtk_widget_hide (GTK_WIDGET (window->unfullscreen_btn));
  
  gtk_box_pack_start (header_box, header_bar, FALSE, TRUE, 0);
}

static void
marker_window_init (MarkerWindow *window)
{
  window->is_fullscreen = FALSE;
  
  GtkBuilder *builder = gtk_builder_new ();

  /** VBox **/
  GtkBox *vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->vbox = vbox;
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));
  gtk_widget_show (GTK_WIDGET (vbox));
  
  /** Editor **/
  MarkerEditor *editor = marker_editor_new ();
  window->editor = editor;
  gtk_box_pack_start (vbox, GTK_WIDGET (editor), TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (editor));
  g_signal_connect (editor, "title-changed", G_CALLBACK (title_changed_cb), window);
  g_signal_connect (editor, "subtitle-changed", G_CALLBACK (subtitle_changed_cb), window);
  MarkerPreview *preview = marker_editor_get_preview (editor);
  g_signal_connect (preview, "zoom-changed", G_CALLBACK (preview_zoom_changed_cb), window);
  
  /** HeaderBar **/
  GtkBox *header_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->header_box = header_box;
  gtk_window_set_titlebar (GTK_WINDOW (window), GTK_WIDGET (header_box));
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-headerbar.ui", NULL);
  GtkHeaderBar *header_bar = GTK_HEADER_BAR (gtk_builder_get_object (builder, "header_bar"));
  window->header_bar = header_bar;
  g_autofree gchar *title = marker_editor_get_title (marker_window_get_active_editor (window));
  g_autofree gchar *subtitle = marker_editor_get_subtitle (marker_window_get_active_editor (window));
  gtk_header_bar_set_title (header_bar, title);
  gtk_header_bar_set_subtitle (header_bar, subtitle);
  GtkButton *unfullscreen_btn = GTK_BUTTON (gtk_builder_get_object (builder, "unfullscreen_btn"));
  window->unfullscreen_btn = unfullscreen_btn;
  g_signal_connect_swapped (unfullscreen_btn, "clicked", G_CALLBACK (marker_window_unfullscreen), window);
  gtk_header_bar_set_show_close_button (header_bar, TRUE);
  gtk_box_pack_start (header_box, GTK_WIDGET (header_bar), FALSE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (header_box));
  
  /** Popover **/
  GtkMenuButton *menu_btn = GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn")); 
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-gear-popover.ui", NULL);
  GtkWidget *popover = GTK_WIDGET (gtk_builder_get_object (builder, "gear_menu_popover"));
  window->zoom_original_btn = GTK_BUTTON (gtk_builder_get_object (builder, "zoom_original_btn"));
  gtk_menu_button_set_use_popover (menu_btn, TRUE);
  gtk_menu_button_set_popover (menu_btn, popover);
  gtk_menu_button_set_direction (menu_btn, GTK_ARROW_DOWN);
  preview_zoom_changed_cb (preview, window);
  
  g_action_map_add_action_entries(G_ACTION_MAP(window), WINDOW_ACTIONS, G_N_ELEMENTS(WINDOW_ACTIONS), window);
  
  if (!marker_has_app_menu ())
  {
    GtkWidget *extra_items = GTK_WIDGET (gtk_builder_get_object (builder, "appmenu_popover_items"));
    GtkBox *popover_vbox = GTK_BOX (gtk_builder_get_object (builder, "gear_menu_popover_vbox"));
    gtk_box_pack_end (popover_vbox, extra_items, FALSE, FALSE, 0);
    GtkApplication* app = marker_get_app ();
    g_action_map_add_action_entries (G_ACTION_MAP(app),
                                     APP_MENU_ACTION_ENTRIES,
                                     APP_MENU_ACTION_ENTRIES_LEN,
                                     window);
  }
  
  /** Window **/
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_signal_connect (window, "key-press-event", G_CALLBACK (key_pressed_cb), window);
  g_signal_connect(window, "delete-event", G_CALLBACK(window_deleted_event_cb), window);
  
  gtk_builder_add_callback_symbol (builder, "save_button_clicked_cb", G_CALLBACK (marker_window_save_active_file));
  gtk_builder_add_callback_symbol (builder, "open_button_clicked_cb", G_CALLBACK (marker_window_open_file));
  gtk_builder_connect_signals (builder, window);
  
  g_object_unref (builder);
}

static void
marker_window_class_init (MarkerWindowClass *class)
{

}

MarkerWindow *
marker_window_new (GtkApplication *app)
{
  MarkerWindow *window = g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
  marker_editor_new_file(marker_window_get_active_editor (window));
  return window;
}

MarkerWindow *
marker_window_new_from_file (GtkApplication *app,
                             GFile          *file)
{
  MarkerWindow *window = g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
  marker_editor_open_file (marker_window_get_active_editor (window), file);
  return window;
}

void
marker_window_open_file (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open",
                                                   GTK_WINDOW (window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gint response = gtk_dialog_run (GTK_DIALOG (dialog));
  
  if (response == GTK_RESPONSE_ACCEPT)
  {
    GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
    // marker_create_new_window_from_file (file);
    marker_editor_open_file(marker_window_get_active_editor(window), file);
  }
  
  gtk_widget_destroy (dialog);
}

void
marker_window_save_active_file_as (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save As",
                                                   GTK_WINDOW (window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

  gint response = gtk_dialog_run (GTK_DIALOG (dialog));
  
  if (response == GTK_RESPONSE_ACCEPT)
  {
    GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog)); 
    marker_editor_save_file_as (marker_window_get_active_editor (window), file);
  }
  
  gtk_widget_destroy (dialog);
}

void
marker_window_save_active_file (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (marker_editor_get_file (editor))
  {
    marker_editor_save_file (editor);
  }
  else
  {
    marker_window_save_active_file_as (window);
  }
}


void
marker_window_toggle_fullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  if (marker_window_is_fullscreen (window))
  {
    marker_window_unfullscreen (window);
  }
  else
  {
    marker_window_fullscreen (window);
  }
}

gboolean
marker_window_is_fullscreen (MarkerWindow *window)
{
  return window->is_fullscreen;
}

MarkerEditor *
marker_window_get_active_editor (MarkerWindow *window)
{
  g_return_val_if_fail (MARKER_IS_WINDOW (window), NULL);
  return window->editor;
}

gboolean
marker_window_is_active_editor (MarkerWindow *window,
                                MarkerEditor *editor)
{
  g_assert (MARKER_IS_WINDOW (window));
  g_assert (MARKER_IS_EDITOR (editor));
  
  return (marker_window_get_active_editor (window) == editor);
}

void
marker_window_open_sketcher (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  
  MarkerEditor *editor = marker_window_get_active_editor (window);
  GFile *file = marker_editor_get_file (editor);
  MarkerSourceView *source_view = marker_editor_get_source_view (editor);
  
  marker_sketcher_window_show (GTK_WINDOW (window), file, source_view);
}

gboolean
marker_window_try_close (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  
  MarkerEditor *editor = marker_window_get_active_editor (window);
  gboolean status = TRUE;
  
  if (marker_editor_has_unsaved_changes (editor))
    status = show_unsaved_documents_warning (window);
  
  if (status)
  {
    marker_editor_closing(editor);
    gtk_widget_destroy (GTK_WIDGET (window));
  }
  return status;
}

void
marker_window_close_current_document (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  
  MarkerEditor *editor = marker_window_get_active_editor (window);
  gboolean status = TRUE;
  
  if (marker_editor_document_has_unsaved_changes (editor))
    status = show_unsaved_documents_warning (window);
  if (status)
  {
    if (marker_editor_close_current_document(editor))
    {
      gtk_widget_destroy (GTK_WIDGET (window));
    }
  }
}
