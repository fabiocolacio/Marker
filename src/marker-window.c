/*
 * marker-window.c
 *
 * Copyright (C) 2017-2020 - 2018 Fabio Colacio
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

#include <glib/gi18n.h>

#include "marker.h"
#include "marker-prefs.h"
#include "marker-editor.h"
#include "marker-exporter.h"
#include "marker-sketcher-window.h"

#include "marker-window.h"

#include <glib.h>
#include <glib/gprintf.h>

#define MIN_DELTA_T 1


enum {
  TITLE_COLUMN,
  ICON_COLUMN,
  NAME_COLUMN,
  EDITOR_COLUMN,
  N_COLUMNS
};

struct _MarkerWindow
{
  GtkApplicationWindow  parent_instance;

  GtkBox               *header_box;
  GtkHeaderBar         *header_bar;
  GtkButton            *zoom_original_btn;
  GtkButton            *view_mode_btn;
  GtkToggleButton      *spell_check_btn;
  GtkToggleButton      *line_numbers_btn;
  GtkToggleButton      *wrap_text_btn;
  GtkToggleButton      *sidebar_btn;

  gboolean              is_fullscreen;
  GtkButton            *unfullscreen_btn;

  GtkBox               *vbox;
  MarkerEditor         *active_editor;

  GtkStack             *editors_stack;
  GtkTreeView          *documents_tree_view;
  GtkTreeStore         *documents_tree_store;
  GtkPaned             *main_paned;
  GtkWidget            *paned1;
  GtkWidget            *paned2;
  guint                 editors_counter;
  guint                 untitled_files;
  gboolean              sidebar_visible;

  guint32               last_click_;
  
  /* GSettings for monitoring preference changes */
  GSettings            *editor_settings;
  GSettings            *window_settings;
};

G_DEFINE_TYPE (MarkerWindow, marker_window, GTK_TYPE_APPLICATION_WINDOW);

/* Forward declarations */
static void update_view_mode_button_icon (MarkerWindow *window, MarkerViewMode mode);
static void action_toggle_line_numbers (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void action_toggle_wrap_text (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void action_toggle_scroll_sync (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void on_preferences_changed (GSettings *settings, gchar *key, gpointer user_data);
static void action_insert_image (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void action_bullet_list (GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void action_numbered_list (GSimpleAction *action, GVariant *parameter, gpointer user_data);

gboolean
get_current_iter(MarkerWindow *window,
                 GtkTreeIter  *iter)
{
  GtkTreeModel      *model = GTK_TREE_MODEL(window->documents_tree_store);
  GtkTreeSelection  *selection = gtk_tree_view_get_selection(window->documents_tree_view);
  return gtk_tree_selection_get_selected (selection, &model, iter);
}

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
  const gchar *cancel_text = "Cancel";
  const gchar *ok_text = "Discard";
  g_assert (MARKER_IS_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  GFile *file = marker_editor_get_file (editor);


  GtkWidget *dialog;
  if (G_IS_FILE (file))
  {
    g_autofree gchar *filename = g_file_get_basename (file);
    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW (window),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_NONE,
                                                _("<span weight='bold' size='larger'>"
                                                "Discard changes to the document '%s'?"
                                                "</span>\n\n"
                                                "The document has unsaved changes "
                                                "that will be lost if it is closed now."),
                                                filename);
  }
  else
  {
    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW (window),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_NONE,
                                                _("<span weight='bold' size='larger'>"
                                                "Discard changes to the document?"
                                                "</span>\n\n"
                                                "The document has unsaved changes "
                                                "that will be lost if it is closed now."));
  }

  gtk_dialog_add_buttons(GTK_DIALOG (dialog),
                         cancel_text,
                         GTK_RESPONSE_CANCEL,
                         ok_text,
                         GTK_RESPONSE_OK,
                         (char *)0);

  gint response = gtk_dialog_run(GTK_DIALOG(dialog));

  gtk_widget_destroy (GTK_WIDGET (dialog));

  if (response == GTK_RESPONSE_OK)
    return TRUE;

  return FALSE;
}

static void
action_fullscreen (GSimpleAction *action,
                   GVariant      *value,
                   gpointer       window)
{
    gboolean state = g_variant_get_boolean (value);

    g_simple_action_set_state (action, value);

    if (state) {
        marker_window_fullscreen (MARKER_WINDOW (window));
    }
    else {
        marker_window_unfullscreen (MARKER_WINDOW (window));
    }
}

static void
action_sidebar (GSimpleAction *action,
                GVariant      *value,
                gpointer       window)
{
    gboolean state = g_variant_get_boolean (value);

    g_simple_action_set_state (action, value);
    // save whether the sidebar is shown
    marker_prefs_set_show_sidebar (state);

    if (state) {
        marker_window_show_sidebar (MARKER_WINDOW (window));
    }
    else {
        marker_window_hide_sidebar (MARKER_WINDOW (window));
    }
    
}

static void
action_link (GSimpleAction *action,
             GVariant      *parameter,
             gpointer       window)
{
    MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    marker_source_view_insert_link (source_view);
}

static void
action_insert_image (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
    MarkerWindow *window = MARKER_WINDOW (user_data);
    MarkerEditor *editor = marker_window_get_active_editor (window);
    if (!editor) return;
    
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    if (!source_view) return;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Insert Image",
                                                      GTK_WINDOW (window),
                                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                                      "_Cancel", GTK_RESPONSE_CANCEL,
                                                      "_Insert", GTK_RESPONSE_ACCEPT,
                                                      NULL);
    
    /* Add file filters */
    GtkFileFilter *filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "Image files");
    gtk_file_filter_add_mime_type (filter, "image/png");
    gtk_file_filter_add_mime_type (filter, "image/jpeg");
    gtk_file_filter_add_mime_type (filter, "image/gif");
    gtk_file_filter_add_mime_type (filter, "image/svg+xml");
    gtk_file_filter_add_mime_type (filter, "image/webp");
    gtk_file_filter_add_pattern (filter, "*.png");
    gtk_file_filter_add_pattern (filter, "*.jpg");
    gtk_file_filter_add_pattern (filter, "*.jpeg");
    gtk_file_filter_add_pattern (filter, "*.gif");
    gtk_file_filter_add_pattern (filter, "*.svg");
    gtk_file_filter_add_pattern (filter, "*.webp");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    
    GtkFileFilter *all_filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (all_filter, "All files");
    gtk_file_filter_add_pattern (all_filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), all_filter);
    
    /* Set current folder to document's folder if available */
    GFile *doc_file = marker_editor_get_file (editor);
    if (doc_file) {
        GFile *parent = g_file_get_parent (doc_file);
        if (parent) {
            gchar *folder = g_file_get_path (parent);
            gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), folder);
            g_free (folder);
            g_object_unref (parent);
        }
    }
    
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        
        /* Calculate relative path if document has a file */
        gchar *relative_path = NULL;
        gchar *alt_text = NULL;
        
        if (doc_file) {
            GFile *img_file = g_file_new_for_path (filename);
            GFile *doc_parent = g_file_get_parent (doc_file);
            
            if (doc_parent) {
                relative_path = g_file_get_relative_path (doc_parent, img_file);
                if (!relative_path) {
                    /* If can't get relative path, use absolute */
                    relative_path = g_strdup (filename);
                }
                g_object_unref (doc_parent);
            }
            g_object_unref (img_file);
        } else {
            /* No document file, use absolute path */
            relative_path = g_strdup (filename);
        }
        
        /* Extract filename for alt text */
        alt_text = g_path_get_basename (filename);
        /* Remove extension from alt text */
        gchar *dot = strrchr (alt_text, '.');
        if (dot) *dot = '\0';
        
        /* Insert HTML img tag */
        gchar *img_tag = g_strdup_printf ("<img src=\"%s\" alt=\"%s\" width=\"800\" />", 
                                          relative_path, alt_text);
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
        gtk_text_buffer_insert_at_cursor (buffer, img_tag, -1);
        
        g_free (img_tag);
        g_free (alt_text);
        g_free (relative_path);
        g_free (filename);
    }
    
    gtk_widget_destroy (dialog);
}

static void
action_bullet_list (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
    MarkerWindow *window = MARKER_WINDOW (user_data);
    MarkerEditor *editor = marker_window_get_active_editor (window);
    if (!editor) return;
    
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    if (!source_view) return;
    
    marker_source_view_convert_to_bullet_list (source_view);
}

static void
action_numbered_list (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
    MarkerWindow *window = MARKER_WINDOW (user_data);
    MarkerEditor *editor = marker_window_get_active_editor (window);
    if (!editor) return;
    
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    if (!source_view) return;
    
    marker_source_view_convert_to_numbered_list (source_view);
}

static void
action_monospace (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       window)
{
    MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    marker_source_view_surround_selection_with (source_view, "``");
}

static void
action_italic (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       window)
{
    MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    marker_source_view_surround_selection_with (source_view, "*");
}

static void
action_bold (GSimpleAction *action,
             GVariant      *parameter,
             gpointer       window)
{
    MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    marker_source_view_surround_selection_with (source_view, "**");
}

static void
action_refresh (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       window)
{
    MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
    marker_editor_refresh_preview (editor);
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
  update_view_mode_button_icon (window, EDITOR_ONLY_MODE);
}

static void
action_preview_only_mode (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, PREVIEW_ONLY_MODE);
  update_view_mode_button_icon (window, PREVIEW_ONLY_MODE);
}

static void
action_dual_pane_mode (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, DUAL_PANE_MODE);
  update_view_mode_button_icon (window, DUAL_PANE_MODE);
}

static void
action_dual_window_mode (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  marker_editor_set_view_mode (editor, DUAL_WINDOW_MODE);
  update_view_mode_button_icon (window, DUAL_WINDOW_MODE);
}

static void
update_view_mode_button_icon (MarkerWindow *window, MarkerViewMode mode)
{
  if (!window->view_mode_btn) return;
  
  const gchar *icon_name;
  const gchar *tooltip;
  
  switch (mode) {
    case EDITOR_ONLY_MODE:
      icon_name = "text-editor-symbolic";
      tooltip = "Switch to Preview Only (Ctrl+Shift+L)";
      break;
    case PREVIEW_ONLY_MODE:
      icon_name = "view-continuous-symbolic";
      tooltip = "Switch to Dual Pane (Ctrl+Shift+L)";
      break;
    case DUAL_PANE_MODE:
      icon_name = "view-dual-symbolic";
      tooltip = "Switch to Editor Only (Ctrl+Shift+L)";
      break;
    case DUAL_WINDOW_MODE:
      icon_name = "view-fullscreen-symbolic";
      tooltip = "Switch to Editor Only (Ctrl+Shift+L)";
      break;
    default:
      icon_name = "view-dual-symbolic";
      tooltip = "Cycle view mode (Ctrl+Shift+L)";
      break;
  }
  
  gtk_button_set_image (window->view_mode_btn, 
                        gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON));
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->view_mode_btn), tooltip);
}

static void
action_toggle_view_mode (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor) return;
  
  MarkerViewMode current_mode = marker_editor_get_view_mode (editor);
  MarkerViewMode new_mode;
  
  /* Cycle through: EDITOR_ONLY -> PREVIEW_ONLY -> DUAL_PANE -> EDITOR_ONLY */
  switch (current_mode) {
    case EDITOR_ONLY_MODE:
      new_mode = PREVIEW_ONLY_MODE;
      break;
    case PREVIEW_ONLY_MODE:
      new_mode = DUAL_PANE_MODE;
      break;
    case DUAL_PANE_MODE:
      new_mode = EDITOR_ONLY_MODE;
      break;
    case DUAL_WINDOW_MODE:
      /* If in dual window mode, switch to editor only */
      new_mode = EDITOR_ONLY_MODE;
      break;
    default:
      new_mode = DUAL_PANE_MODE;
      break;
  }
  
  marker_editor_set_view_mode (editor, new_mode);
  update_view_mode_button_icon (window, new_mode);
}

static void
action_toggle_spell_check (GSimpleAction *action,
                           GVariant      *value,
                           gpointer       user_data)
{
  gboolean state = g_variant_get_boolean (value);
  g_simple_action_set_state (action, value);
  
  /* Save the spell check preference */
  marker_prefs_set_spell_check (state);
  
  /* Apply to all editors in all windows */
  GtkApplication *app = marker_get_app();
  if (!app) return;
  
  GList *windows = gtk_application_get_windows(app);
  for (GList *win_item = windows; win_item != NULL; win_item = win_item->next) {
    if (MARKER_IS_WINDOW(win_item->data)) {
      MarkerWindow *win = MARKER_WINDOW(win_item->data);
      
      /* Get all editors in this window */
      GList *children = gtk_container_get_children(GTK_CONTAINER(win->editors_stack));
      for (GList *child = children; child != NULL; child = child->next) {
        if (MARKER_IS_EDITOR(child->data)) {
          MarkerEditor *editor = MARKER_EDITOR(child->data);
          marker_editor_apply_prefs(editor);
        }
      }
      g_list_free(children);
    }
  }
}

static void
action_toggle_line_numbers (GSimpleAction *action,
                            GVariant      *value,
                            gpointer       user_data)
{
  gboolean state = g_variant_get_boolean (value);
  g_simple_action_set_state (action, value);
  
  /* Save the line numbers preference */
  marker_prefs_set_show_line_numbers (state);
  
  /* Apply to all editors in all windows */
  GtkApplication *app = marker_get_app();
  if (!app) return;
  
  GList *windows = gtk_application_get_windows(app);
  for (GList *win_item = windows; win_item != NULL; win_item = win_item->next) {
    if (MARKER_IS_WINDOW(win_item->data)) {
      MarkerWindow *win = MARKER_WINDOW(win_item->data);
      
      /* Get all editors in this window */
      GList *children = gtk_container_get_children(GTK_CONTAINER(win->editors_stack));
      for (GList *child = children; child != NULL; child = child->next) {
        if (MARKER_IS_EDITOR(child->data)) {
          MarkerEditor *editor = MARKER_EDITOR(child->data);
          marker_editor_apply_prefs(editor);
        }
      }
      g_list_free(children);
    }
  }
}

static void
action_toggle_wrap_text (GSimpleAction *action,
                         GVariant      *value,
                         gpointer       user_data)
{
  gboolean state = g_variant_get_boolean (value);
  g_simple_action_set_state (action, value);
  
  /* Save the wrap text preference */
  marker_prefs_set_wrap_text (state);
  
  /* Apply to all editors in all windows */
  GtkApplication *app = marker_get_app();
  if (!app) return;
  
  GList *windows = gtk_application_get_windows(app);
  for (GList *win_item = windows; win_item != NULL; win_item = win_item->next) {
    if (MARKER_IS_WINDOW(win_item->data)) {
      MarkerWindow *win = MARKER_WINDOW(win_item->data);
      
      /* Get all editors in this window */
      GList *children = gtk_container_get_children(GTK_CONTAINER(win->editors_stack));
      for (GList *child = children; child != NULL; child = child->next) {
        if (MARKER_IS_EDITOR(child->data)) {
          MarkerEditor *editor = MARKER_EDITOR(child->data);
          marker_editor_apply_prefs(editor);
        }
      }
      g_list_free(children);
    }
  }
}

static void
action_toggle_scroll_sync (GSimpleAction *action,
                           GVariant      *value,
                           gpointer       user_data)
{
  gboolean state = g_variant_get_boolean (value);
  g_simple_action_set_state (action, value);
  
  /* Save the scroll sync preference */
  marker_prefs_set_enable_scroll_sync (state);
  
  /* Apply to all editors in all windows */
  GtkApplication *app = marker_get_app();
  if (!app) return;
  
  GList *windows = gtk_application_get_windows(app);
  for (GList *win_item = windows; win_item != NULL; win_item = win_item->next) {
    if (MARKER_IS_WINDOW(win_item->data)) {
      MarkerWindow *win = MARKER_WINDOW(win_item->data);
      
      /* Get all editors in this window */
      GList *children = gtk_container_get_children(GTK_CONTAINER(win->editors_stack));
      for (GList *child = children; child != NULL; child = child->next) {
        if (MARKER_IS_EDITOR(child->data)) {
          MarkerEditor *editor = MARKER_EDITOR(child->data);
          marker_editor_set_scroll_sync(editor, state);
        }
      }
      g_list_free(children);
    }
  }
}

static void
on_preferences_changed (GSettings *settings,
                        gchar     *key,
                        gpointer   user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  GAction *action;
  
  if (g_strcmp0 (key, "spell-check") == 0) {
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "togglespellcheck");
    if (action) {
      gboolean state = marker_prefs_get_spell_check();
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (state));
    }
  } else if (g_strcmp0 (key, "show-line-numbers") == 0) {
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "togglelinenumbers");
    if (action) {
      gboolean state = marker_prefs_get_show_line_numbers();
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (state));
    }
  } else if (g_strcmp0 (key, "wrap-text") == 0) {
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "togglewraptext");
    if (action) {
      gboolean state = marker_prefs_get_wrap_text();
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (state));
    }
  } else if (g_strcmp0 (key, "enable-scroll-sync") == 0) {
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "togglescrollsync");
    if (action) {
      gboolean state = marker_prefs_get_enable_scroll_sync();
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (state));
    }
  } else if (g_strcmp0 (key, "show-sidebar") == 0) {
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "sidebar");
    if (action) {
      gboolean state = marker_prefs_get_show_sidebar();
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (state));
    }
  }
}


gchar *
make_markup_title(MarkerEditor *editor,
                  const char   *raw_title)
{
  gchar * markup_title;
  if (marker_editor_has_unsaved_changes(editor))
  {
    markup_title = g_strdup_printf("<i>%s</i>", raw_title);
  } else
  {
    markup_title = g_strdup_printf("%s", raw_title);
  }
  return markup_title;
}

static void
title_changed_cb (MarkerEditor *editor,
                  const gchar  *title,
                  const gchar  *raw_title,
                  gpointer      user_data)
{
  MarkerWindow *window = MARKER_WINDOW(user_data);
  gtk_header_bar_set_title (window->header_bar, title);
  GtkTreeIter iter;
  if (get_current_iter(window, &iter)){
    g_autofree gchar * markup_title = make_markup_title(editor,
                                                        raw_title);
    gtk_tree_store_set(window->documents_tree_store,
                       &iter,
                       TITLE_COLUMN,
                       markup_title, -1);
  }
}

static void
subtitle_changed_cb (MarkerEditor *editor,
                     const gchar  *subtitle,
                     gpointer      user_data)
{
  MarkerWindow *window = MARKER_WINDOW(user_data);
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
  GtkWidget * const main_paned = GTK_WIDGET (window->main_paned);

  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (header_box), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), FALSE);
  gtk_widget_show (GTK_WIDGET (window->unfullscreen_btn));

  g_object_ref (main_paned);
  gtk_container_remove (GTK_CONTAINER (vbox), main_paned);

  gtk_box_pack_start (vbox, header_bar, FALSE, TRUE, 0);
  gtk_box_pack_start (vbox, main_paned, TRUE, TRUE, 0);
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
rename_file_action_cb(GtkCellRendererText *cell,
                      gchar               *path_string,
                      gchar               *new_text,
                      gpointer             user_data)
{
  if (!new_text || sizeof(new_text) == 0)
    return;
  MarkerWindow * window = MARKER_WINDOW(user_data);
  GtkTreeIter iter;
  GtkTreeModel *model = GTK_TREE_MODEL(window->documents_tree_store);
  MarkerEditor * editor;

  if (gtk_tree_model_get_iter_from_string(model, &iter, path_string))
  {
    gtk_tree_model_get (model, &iter, EDITOR_COLUMN, &editor, -1);
    if (marker_editor_rename_file(editor, g_strdup(new_text)))
    {
      g_autofree gchar * markup_title = make_markup_title(editor,
                                                          new_text);
      gtk_tree_store_set(window->documents_tree_store,
                         &iter,
                         TITLE_COLUMN,
                         markup_title, -1);
      if (editor == window->active_editor)
      {
        g_autofree gchar *title = marker_editor_get_title (editor);
        g_autofree gchar *subtitle = marker_editor_get_subtitle (editor);

        gtk_header_bar_set_title (window->header_bar, title);
        gtk_header_bar_set_subtitle (window->header_bar, subtitle);
        
        /* Update view mode button icon */
        MarkerViewMode mode = marker_editor_get_view_mode (editor);
        update_view_mode_button_icon (window, mode);
      }
    }
  }
}



static void
tree_selection_changed_cb(GtkTreeSelection *selection,
                          gpointer          data)
{
  MarkerWindow * window = MARKER_WINDOW(data);
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *name;
  MarkerEditor * editor;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);
    gtk_tree_model_get (model, &iter, EDITOR_COLUMN, &editor, -1);
    gtk_stack_set_visible_child_full(window->editors_stack, name, GTK_STACK_TRANSITION_TYPE_CROSSFADE);

    window->active_editor = editor;
    g_autofree gchar *title = marker_editor_get_title (marker_window_get_active_editor (window));
    g_autofree gchar *subtitle = marker_editor_get_subtitle (marker_window_get_active_editor (window));
    gtk_header_bar_set_title (window->header_bar, title);
    gtk_header_bar_set_subtitle (window->header_bar, subtitle);
    preview_zoom_changed_cb(marker_editor_get_preview(editor),
                            window);
    
    /* Update view mode button icon */
    MarkerViewMode mode = marker_editor_get_view_mode (editor);
    update_view_mode_button_icon (window, mode);
    g_free (name);
  }
}



static gboolean
close_button_clicked(GtkTreeView *view, GtkTreeViewColumn *col, guint x, GtkCellRenderer * cell)
{

	gint               colw = 0;

	g_return_val_if_fail ( view != NULL, FALSE );

	if (col == NULL)
		return FALSE; /* not found */

	/* (2) find the cell renderer within the column */

    GtkCellRenderer *checkcell = cell;
    gint min_width=0, nat_width=0;
    gtk_cell_renderer_get_preferred_width(checkcell, GTK_WIDGET(view), &min_width, &nat_width);

    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_INT);
    g_object_get_property(G_OBJECT(col), "width", &value);
    colw = g_value_get_int(&value);


    if (x >= colw-nat_width && x < colw)
    {
    	return TRUE;
    }
	return FALSE; /* not found */
}

static gboolean
button_pressed_cb (GtkWidget *view,
                   GdkEventButton *bevent,
                   gpointer data)
{

  GtkCellRenderer * cell_renderer = GTK_CELL_RENDERER(data);
  MarkerWindow * window = MARKER_WINDOW(gtk_widget_get_ancestor(view, MARKER_TYPE_WINDOW));
  gint x;

  GtkTreePath * path = NULL;
  GtkTreeViewColumn * col = NULL;

  gboolean path_found = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(view),
                                                        bevent->x,
                                                        bevent->y,
                                                        &path,
                                                        &col,
                                                        &x, NULL);

  if (path_found && path != NULL) {
    gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)),
                                   path);
  }

  if (close_button_clicked(GTK_TREE_VIEW(view), col, x, cell_renderer))
    marker_window_close_current_document(window);
  
  if (path)
    gtk_tree_path_free(path);
  
  guint32 delta = bevent->time - window->last_click_;
  window->last_click_ = bevent->time;
  if (bevent->type == GDK_2BUTTON_PRESS || delta < 500)
  {
    return FALSE;
  }
  return TRUE;
}

static void
marker_window_init (MarkerWindow *window)
{
  { // SETUP ACTIONS //
    GAction *action = NULL;
    GtkApplication *app = marker_get_app ();

    action = G_ACTION (g_simple_action_new ("refresh", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_refresh), window);
    const gchar *refresh_accels[] = { "<Ctrl>r", NULL };
    gtk_application_set_accels_for_action (app, "win.refresh", refresh_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("sketcher", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_open_sketcher), window);
    const gchar *sketcher_accels[] = { "<Ctrl>d", NULL };
    gtk_application_set_accels_for_action (app, "win.sketcher", sketcher_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);
    
    action = G_ACTION (g_simple_action_new ("insertimage", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_insert_image), window);
    const gchar *insertimage_accels[] = { "<Ctrl><Shift>i", NULL };
    gtk_application_set_accels_for_action (app, "win.insertimage", insertimage_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("bulletlist", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_bullet_list), window);
    const gchar *bulletlist_accels[] = { "<Ctrl><Alt>l", NULL };
    gtk_application_set_accels_for_action (app, "win.bulletlist", bulletlist_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("numberedlist", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_numbered_list), window);
    const gchar *numberedlist_accels[] = { "<Ctrl><Alt>n", NULL };
    gtk_application_set_accels_for_action (app, "win.numberedlist", numberedlist_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("find", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_search), window);
    const gchar *find_accels[] = { "<Ctrl>f", NULL };
    gtk_application_set_accels_for_action (app, "win.find", find_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("gotoline", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_go_to_line), window);
    const gchar *gotoline_accels[] = { "<Ctrl>g", NULL };
    gtk_application_set_accels_for_action (app, "win.gotoline", gotoline_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("link", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_link), window);
    const gchar *link_accels[] = { "<Ctrl>k", NULL };
    gtk_application_set_accels_for_action (app, "win.link", link_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("italic", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_italic), window);
    const gchar *italic_accels[] = { "<Ctrl>i", NULL };
    gtk_application_set_accels_for_action (app, "win.italic", italic_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("bold", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_bold), window);
    const gchar *bold_accels[] = { "<Ctrl>b", NULL };
    gtk_application_set_accels_for_action (app, "win.bold", bold_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("monospace", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_monospace), window);
    const gchar *monospace_accels[] = { "<Ctrl>m", NULL };
    gtk_application_set_accels_for_action (app, "win.monospace", monospace_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("new", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_create_new_window), NULL);
    const gchar *new_accels[] = { "<Shift><Ctrl>n", NULL };
    gtk_application_set_accels_for_action (app, "win.new", new_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("neweditor", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_new_editor), window);
    const gchar *neweditor_accels[] = { "<Ctrl>n", NULL };
    gtk_application_set_accels_for_action (app, "win.neweditor", neweditor_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);
    
    action = G_ACTION (g_simple_action_new ("closedocument", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_close_current_document), window);
    const gchar *closedocument_accels[] = { "<Ctrl>w", NULL };
    gtk_application_set_accels_for_action (app, "win.closedocument", closedocument_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("close", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_try_close), window);
    const gchar *close_accels[] = { "<Shift><Ctrl>w", NULL };
    gtk_application_set_accels_for_action (app, "win.close", close_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("zoomoriginal", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_zoom_original), window);
    const gchar *zoomoriginal_accels[] = { "<Ctrl>equal", NULL };
    gtk_application_set_accels_for_action (app, "win.zoomoriginal", zoomoriginal_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("zoomin", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_zoom_in), window);
    const gchar *zoomin_accels[] = { "<Ctrl>plus", NULL };
    gtk_application_set_accels_for_action (app, "win.zoomin", zoomin_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("zoomout", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_zoom_out), window);
    const gchar *zoomout_accels[] = { "<Ctrl>minus", NULL };
    gtk_application_set_accels_for_action (app, "win.zoomout", zoomout_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("editoronlymode", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_editor_only_mode), window);
    const gchar *editoronlymode_accels[] = { "<Ctrl>1", NULL };
    gtk_application_set_accels_for_action (app, "win.editoronlymode", editoronlymode_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("previewonlymode", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_preview_only_mode), window);
    const gchar *previewonlymode_accels[] = { "<Ctrl>2", NULL };
    gtk_application_set_accels_for_action (app, "win.previewonlymode", previewonlymode_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("dualpanemode", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_dual_pane_mode), window);
    const gchar *dualpanemode_accels[] = { "<Ctrl>3", NULL };
    gtk_application_set_accels_for_action (app, "win.dualpanemode", dualpanemode_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("dualwindowmode", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_dual_window_mode), window);
    const gchar *dualwindowmode_accels[] = { "<Ctrl>4", NULL };
    gtk_application_set_accels_for_action (app, "win.dualwindowmode", dualwindowmode_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("toggleviewmode", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_toggle_view_mode), window);
    const gchar *toggleviewmode_accels[] = { "<Ctrl><Shift>l", NULL };
    gtk_application_set_accels_for_action (app, "win.toggleviewmode", toggleviewmode_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    gboolean spell_check_state = marker_prefs_get_spell_check();
    action = G_ACTION (g_simple_action_new_stateful ("togglespellcheck", NULL, g_variant_new_boolean (spell_check_state)));
    g_signal_connect (G_SIMPLE_ACTION (action), "change-state", G_CALLBACK (action_toggle_spell_check), window);
    const gchar *togglespellcheck_accels[] = { "F7", NULL };
    gtk_application_set_accels_for_action (app, "win.togglespellcheck", togglespellcheck_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);
    
    gboolean line_numbers_state = marker_prefs_get_show_line_numbers();
    action = G_ACTION (g_simple_action_new_stateful ("togglelinenumbers", NULL, g_variant_new_boolean (line_numbers_state)));
    g_signal_connect (G_SIMPLE_ACTION (action), "change-state", G_CALLBACK (action_toggle_line_numbers), window);
    g_action_map_add_action (G_ACTION_MAP (window), action);
    
    gboolean wrap_text_state = marker_prefs_get_wrap_text();
    action = G_ACTION (g_simple_action_new_stateful ("togglewraptext", NULL, g_variant_new_boolean (wrap_text_state)));
    g_signal_connect (G_SIMPLE_ACTION (action), "change-state", G_CALLBACK (action_toggle_wrap_text), window);
    g_action_map_add_action (G_ACTION_MAP (window), action);
    
    gboolean scroll_sync_state = marker_prefs_get_enable_scroll_sync();
    action = G_ACTION (g_simple_action_new_stateful ("togglescrollsync", NULL, g_variant_new_boolean (scroll_sync_state)));
    g_signal_connect (G_SIMPLE_ACTION (action), "change-state", G_CALLBACK (action_toggle_scroll_sync), window);
    g_action_map_add_action (G_ACTION_MAP (window), action);
    

    action = G_ACTION (g_simple_action_new ("open", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_open_file), window);
    const gchar *open_accels[] = { "<Ctrl>o", NULL }; 
    gtk_application_set_accels_for_action (app, "win.open", open_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("save", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_save_active_file), window);
    const gchar *save_accels[] = { "<Ctrl>s", NULL };
    gtk_application_set_accels_for_action (app, "win.save", save_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("saveas", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_window_save_active_file_as), window);
    const gchar *saveas_accels[] = { "<Ctrl><Shift>s", NULL };
    gtk_application_set_accels_for_action (app, "win.saveas", saveas_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("export", NULL));
    g_signal_connect_swapped (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (marker_exporter_show_export_dialog), window);
    const gchar *export_accels[] = { "<Ctrl>e", NULL };
    gtk_application_set_accels_for_action (app, "win.export", export_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new ("print", NULL));
    g_signal_connect (G_SIMPLE_ACTION (action), "activate", G_CALLBACK (action_print), window);
    const gchar *print_accels[] = { "<Ctrl>p", NULL };
    gtk_application_set_accels_for_action (app, "win.print", print_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    action = G_ACTION (g_simple_action_new_stateful ("fullscreen", NULL, g_variant_new_boolean (FALSE)));
    g_signal_connect (G_SIMPLE_ACTION (action), "change-state", G_CALLBACK (action_fullscreen), window);
    const gchar *fullscreen_accels[] = { "F11", NULL };
    gtk_application_set_accels_for_action (app, "win.fullscreen", fullscreen_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);

    gboolean sidebar_visible = marker_prefs_get_show_sidebar();
    action = G_ACTION (g_simple_action_new_stateful ("sidebar", NULL, g_variant_new_boolean (sidebar_visible)));
    g_signal_connect (G_SIMPLE_ACTION (action), "change-state", G_CALLBACK (action_sidebar), window);
    const gchar *sidebar_accels[] =  { "<Ctrl><Shift>b", "F12", NULL };
    gtk_application_set_accels_for_action (app, "win.sidebar", sidebar_accels);
    g_action_map_add_action (G_ACTION_MAP (window), action);
  }

  /** Add marker icon theme to the default icon theme **/
  gtk_icon_theme_append_search_path (gtk_icon_theme_get_default(), ICONS_DIR);

  window->is_fullscreen = FALSE;

  window->sidebar_visible = marker_prefs_get_show_sidebar();
  window->editors_counter = 0;
  window->last_click_ = 0;
  
  /* Initialize GSettings and connect change signals */
  window->editor_settings = g_settings_new ("com.github.fabiocolacio.marker.preferences.editor");
  window->window_settings = g_settings_new ("com.github.fabiocolacio.marker.preferences.window");
  
  g_signal_connect (window->editor_settings, "changed", G_CALLBACK (on_preferences_changed), window);
  g_signal_connect (window->window_settings, "changed", G_CALLBACK (on_preferences_changed), window);

  GtkBuilder *builder = gtk_builder_new ();

  /** VBox **/
  GtkBox *vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->vbox = vbox;
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));
  gtk_widget_show (GTK_WIDGET (vbox));


  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-window-main-view.ui", NULL);

  /** DOCUMENTS TREE **/
  GtkTreeView * documents_tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "documents_tree_view"));
  GtkTreeStore
  *documents_store = gtk_tree_store_new(N_COLUMNS,
                                        G_TYPE_STRING,
                                        GDK_TYPE_PIXBUF,
                                        G_TYPE_STRING,
                                        MARKER_TYPE_EDITOR);



  gtk_tree_view_set_model(documents_tree_view, GTK_TREE_MODEL(documents_store));
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, "Documents");

  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer, "editable", FALSE, NULL);
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_set_attributes(column, renderer,
                                      "markup", TITLE_COLUMN,
                                      NULL);

  g_signal_connect (renderer, "edited",
                     G_CALLBACK(rename_file_action_cb),
                     window);

  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_set_attributes(column, renderer,
                                       "pixbuf", ICON_COLUMN,
                                       NULL);

  gtk_tree_view_append_column (documents_tree_view, column);

  window->documents_tree_store = documents_store;
  window->documents_tree_view = documents_tree_view;


  GtkTreeSelection *select;
  select = gtk_tree_view_get_selection (documents_tree_view);

  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (select), "changed",
                    G_CALLBACK (tree_selection_changed_cb),
                    window);

  g_signal_connect (documents_tree_view, "button-press-event",
                    G_CALLBACK(button_pressed_cb),
                    renderer);

  /** EDITOR STACKS **/
  window->editors_stack = GTK_STACK(gtk_builder_get_object(builder, "documents_stack"));
  gtk_widget_show(GTK_WIDGET(window->editors_stack));

  /** MAIN PANED **/
  GtkWidget * main_paned = GTK_WIDGET(gtk_builder_get_object(builder, "main_paned"));
  gtk_box_pack_start (vbox, main_paned, TRUE, TRUE, 0);
  gtk_paned_set_position(GTK_PANED(main_paned), 0);
  window->paned1 = gtk_paned_get_child1 (GTK_PANED (main_paned));
  window->paned2 = gtk_paned_get_child2 (GTK_PANED (main_paned));
  gtk_widget_show(main_paned);

  window->main_paned = GTK_PANED(main_paned);

  /* Apply sidebar preference now that UI elements are initialized */
  if (marker_prefs_get_show_sidebar())
  {
    /* Show sidebar if preference is to show it */
    marker_window_show_sidebar(window);
  }
  else
  {
    /* Hide sidebar if preference is to not show it */
    marker_window_hide_sidebar(window);
  }

  /** HeaderBar **/
  GtkBox *header_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->header_box = header_box;
  gtk_window_set_titlebar (GTK_WINDOW (window), GTK_WIDGET (header_box));
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-headerbar.ui", NULL);
  GtkHeaderBar *header_bar = GTK_HEADER_BAR (gtk_builder_get_object (builder, "header_bar"));
  window->header_bar = header_bar;

  GtkButton *unfullscreen_btn = GTK_BUTTON (gtk_builder_get_object (builder, "unfullscreen_btn"));
  window->unfullscreen_btn = unfullscreen_btn;
  g_signal_connect_swapped (unfullscreen_btn, "clicked", G_CALLBACK (marker_window_unfullscreen), window);
  gtk_header_bar_set_show_close_button (header_bar, TRUE);
  gtk_box_pack_start (header_box, GTK_WIDGET (header_bar), FALSE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (header_box));

  /** View Mode Button **/
  window->view_mode_btn = GTK_BUTTON (gtk_builder_get_object (builder, "view_mode_btn"));
  
  /** Spell Check Button **/
  window->spell_check_btn = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "spell_check_btn"));
  
  /** Line Numbers Button **/
  window->line_numbers_btn = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "line_numbers_btn"));
  
  /** Wrap Text Button **/
  window->wrap_text_btn = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "wrap_text_btn"));
  
  /** Sidebar Button **/
  window->sidebar_btn = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "sidebar_btn"));
  
  /** Popover **/
  GtkMenuButton *menu_btn = GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn"));
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-gear-popover.ui", NULL);
  GtkWidget *popover = GTK_WIDGET (gtk_builder_get_object (builder, "gear_menu_popover"));
  window->zoom_original_btn = GTK_BUTTON (gtk_builder_get_object (builder, "zoom_original_btn"));
  gtk_menu_button_set_use_popover (menu_btn, TRUE);
  gtk_menu_button_set_popover (menu_btn, popover);
  gtk_menu_button_set_direction (menu_btn, GTK_ARROW_DOWN);

  if (!marker_has_app_menu ())
  {
    GtkWidget *extra_items_start = GTK_WIDGET (gtk_builder_get_object (builder, "appmenu_popover_items_start"));
    GtkWidget *extra_items_end = GTK_WIDGET (gtk_builder_get_object (builder, "appmenu_popover_items_end"));
    gtk_widget_set_visible(extra_items_start, TRUE);
    gtk_widget_set_visible(extra_items_end, TRUE);
    GtkApplication* app = marker_get_app ();
    g_action_map_add_action_entries (G_ACTION_MAP(app),
                                     APP_MENU_ACTION_ENTRIES,
                                     APP_MENU_ACTION_ENTRIES_LEN,
                                     window);
  }

  /** Window **/
  /* Don't hide sidebar here - let the preference handling below take care of it */
  guint width = marker_prefs_get_window_width();
  guint height = marker_prefs_get_window_height();
  g_print ("window size loaded from the preferences: %d x %d\n", width, height);
  if (width == 0)
  {
    marker_prefs_set_window_width(900);
  }
  if (height == 0)
  {
    marker_prefs_set_window_height(600);
  }
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  gint pos_x = 0, pos_y = 0;
  marker_prefs_get_window_position( &pos_x, &pos_y);
  g_print ("window position loaded from the preferences: %d, %d\n", pos_x, pos_y);
  // require restored window position to be positive
  if (pos_y >= 0 && pos_x >= 0)
  {
    gtk_window_move (GTK_WINDOW(window), pos_x, pos_y);
  }

  /* Defer sidebar preference application until after UI is built */
  g_signal_connect(window, "delete-event", G_CALLBACK(window_deleted_event_cb), window);

  g_object_unref (builder);
}

static void
marker_window_constructed (GObject *object)
{
  G_OBJECT_CLASS (marker_window_parent_class)->constructed (object);
  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (object), FALSE);
}

static void
marker_window_finalize (GObject *object)
{
  MarkerWindow *window = MARKER_WINDOW (object);
  
  if (window->editor_settings) {
    g_object_unref (window->editor_settings);
    window->editor_settings = NULL;
  }
  
  if (window->window_settings) {
    g_object_unref (window->window_settings);
    window->window_settings = NULL;
  }
  
  G_OBJECT_CLASS (marker_window_parent_class)->finalize (object);
}

static void
marker_window_class_init (MarkerWindowClass *class)
{
  G_OBJECT_CLASS (class)->constructed = marker_window_constructed;
  G_OBJECT_CLASS (class)->finalize = marker_window_finalize;
}


void
marker_window_add_editor(MarkerWindow *window,
                         MarkerEditor *editor)
{
  window->active_editor = editor;
  gchar * name = g_strnfill(8,0);
  g_sprintf(name, "edit%u", window->editors_counter);

  gtk_stack_add_named(window->editors_stack, GTK_WIDGET(editor), name);
  marker_editor_refresh_preview(editor);
  gtk_widget_show(GTK_WIDGET(editor));
  gtk_stack_set_visible_child_full(window->editors_stack, name, GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  g_autofree gchar *title = marker_editor_get_title (marker_window_get_active_editor (window));
  g_autofree gchar *subtitle = marker_editor_get_subtitle (marker_window_get_active_editor (window));
  gtk_header_bar_set_title (window->header_bar, title);
  gtk_header_bar_set_subtitle (window->header_bar, subtitle);
  
  /* Update view mode button icon */
  MarkerViewMode mode = marker_editor_get_view_mode (editor);
  update_view_mode_button_icon (window, mode);


  GtkTreeIter   iter;

  gtk_tree_store_append (window->documents_tree_store, &iter, NULL);  /* Acquire an iterator */

  GdkPixbuf * icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
                                              "window-close",
                                              16,
                                              GTK_ICON_LOOKUP_FORCE_SYMBOLIC, NULL);

  gtk_tree_store_set (window->documents_tree_store, &iter,
                      TITLE_COLUMN, marker_editor_get_title(editor),
                      ICON_COLUMN, icon,
                      NAME_COLUMN, name,
                      EDITOR_COLUMN, editor,
                      -1);

  gtk_tree_selection_select_iter(gtk_tree_view_get_selection(window->documents_tree_view),
                                 &iter);

  if (window->editors_counter >= 1)
  {
    marker_window_show_sidebar (window);
  }

  g_signal_connect(editor, "title-changed",
                   G_CALLBACK(title_changed_cb),
                   window);
  g_signal_connect(editor, "subtitle-changed",
                   G_CALLBACK(subtitle_changed_cb),
                   window);
  g_signal_connect(marker_editor_get_preview(editor), "zoom-changed",
                   G_CALLBACK(preview_zoom_changed_cb),
                   window);
  preview_zoom_changed_cb(marker_editor_get_preview(editor),
                          window);
  window->editors_counter ++;

  GtkWidget *source_view = GTK_WIDGET (marker_editor_get_source_view (editor));
  gtk_widget_grab_focus (source_view);
}

void
marker_window_new_editor (MarkerWindow *window)
{
  MarkerEditor * editor = marker_editor_new();
  if (window->untitled_files)
  {
    marker_editor_rename_file(editor, g_strdup_printf("Untitled_%u.md", window->untitled_files));
  }
  window->untitled_files ++;
  marker_window_add_editor(window, editor);
}

void
marker_window_new_editor_from_file (MarkerWindow *window,
                                    GFile        *file)
{
  GList *children = gtk_container_get_children (GTK_CONTAINER (window->editors_stack));
  bool duplicate = false;

  for (GList *current = children; current != NULL; current = current->next) {
    GFile *editor_file = marker_editor_get_file (MARKER_EDITOR (current->data));
    if (editor_file != NULL) {
      char *uri1, *uri2;
      uri1 = g_file_get_uri (editor_file);
      uri2 = g_file_get_uri (file);
      if (g_strcmp0 (uri1, uri2) == 0) {
        GtkTreeModel *model = GTK_TREE_MODEL (window->documents_tree_store);
        GtkTreeIter iter;

        duplicate = true;

        g_free (uri1);
        g_free (uri2);

        /**
         * Now that we've found a duplicate, show the relevant editor.
         */
        if (gtk_tree_model_get_iter_first (model, &iter)) {
          do {
            MarkerEditor *editor;

            gtk_tree_model_get (model, &iter, EDITOR_COLUMN, &editor, -1);

            if (editor == MARKER_EDITOR (current->data)) {
              GtkTreeSelection *selection;

              selection = gtk_tree_view_get_selection (window->documents_tree_view);
              gtk_tree_selection_select_iter (selection, &iter);
              break;
            }
          } while (gtk_tree_model_iter_next (model, &iter));
        }
        break;
      }

      g_free (uri1);
      g_free (uri2);
    }
  }

  g_list_free (children);

  if (!duplicate) {
    MarkerEditor *editor = NULL;
    MarkerSourceView *source_view = NULL;
    g_autofree gchar *md = NULL;
    GFile *active_file = NULL;
   
    editor = marker_window_get_active_editor (window);

    if (editor != NULL) {
        active_file = marker_editor_get_file (editor);
        source_view = marker_editor_get_source_view (editor);
        md = marker_source_view_get_text (source_view, false);

        if (strcmp (md, "") == 0 && active_file == NULL) {
          window->editors_counter ++;
          marker_window_close_current_document (window);
          window->editors_counter --;
        }
    }
    editor = marker_editor_new_from_file(file);
    marker_window_add_editor(window, editor);
  }
}

MarkerWindow *
marker_window_new (GtkApplication *app)
{
  MarkerWindow *window = g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
  marker_window_new_editor(window);
  return window;
}

MarkerWindow *
marker_window_new_from_file (GtkApplication *app,
                             GFile          *file)
{
  MarkerWindow *window = g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
  marker_window_new_editor_from_file(window, file);
  return window;
}

void
marker_window_open_file (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  g_autoptr (GtkFileChooserNative) dialog = gtk_file_chooser_native_new (_("Open"),
                                                              GTK_WINDOW (window),
                                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                                              _("_Open"), _("_Cancel"));
  gint response = gtk_native_dialog_run (GTK_NATIVE_DIALOG (dialog));

  if (response == GTK_RESPONSE_ACCEPT)
  {
    GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
    marker_window_new_editor_from_file(window, file);
  }
}

void
marker_window_open_file_in_new_window (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  g_autoptr (GtkFileChooserNative) dialog = gtk_file_chooser_native_new (_("Open"),
                                                              GTK_WINDOW (window),
                                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                                              _("_Open"), _("_Cancel"));

  gint response = gtk_native_dialog_run (GTK_NATIVE_DIALOG (dialog));

  if (response == GTK_RESPONSE_ACCEPT)
  {
    GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
    marker_create_new_window_from_file(file);
  }
}

void
marker_window_save_active_file_as (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  g_autoptr (GtkFileChooserNative) dialog = gtk_file_chooser_native_new (_("Save As"),
                                                                         GTK_WINDOW (window),
                                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                                         _("_Save"), _("_Cancel"));

  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);


  gint response = gtk_native_dialog_run (GTK_NATIVE_DIALOG (dialog));

  if (response == GTK_RESPONSE_ACCEPT)
  {
    GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
    marker_editor_save_file_as (marker_window_get_active_editor (window), file);
  }
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
  return window->active_editor;
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

  gboolean status = TRUE;

  gboolean has_unsaved = FALSE;
  GtkTreeModel * model = GTK_TREE_MODEL(window->documents_tree_store);
  gint rows = gtk_tree_model_iter_n_children (model, NULL);

  // Save window size and position in the preferences
  gint width, height;
  gtk_window_get_size (GTK_WINDOW (window), &width, &height);
  g_print ("saved window size: %d x %d\n", width, height);
  marker_prefs_set_window_width (width);
  marker_prefs_set_window_height (height);

  gint pos_x = 0, pos_y = 0;
  gtk_window_get_position(GTK_WINDOW (window), &pos_x, &pos_y);
  g_print ("saved window position: %d, %d\n", pos_x, pos_y);
  marker_prefs_set_window_position (pos_x, pos_y);

  guint editor_width = marker_editor_get_pane_width (window->active_editor);
  /* Ensure minimum width of 300 to prevent issues with dual pane mode */
  if (editor_width < 300) {
    editor_width = 450; /* Use default width */
  }
  g_print ("saved editor pane width: %d\n", editor_width);
  marker_prefs_set_editor_pane_width (editor_width);

  if (rows > 0)
  {
    /** If there are documents open check for unsaved ones**/
    gint i;
    GtkTreeIter iter;
    for (i = 0; i < rows; i++)
    {
      gtk_tree_model_get_iter(model, &iter, gtk_tree_path_new_from_indices (i, -1));
      MarkerEditor *editor;
      gtk_tree_model_get (model, &iter, EDITOR_COLUMN, &editor, -1);
      has_unsaved = has_unsaved || marker_editor_has_unsaved_changes (editor);
    }

    if (has_unsaved)
      status = show_unsaved_documents_warning (window);

    MarkerEditor *editor = marker_window_get_active_editor (window);
    if (status)
    {
      marker_editor_closing(editor);
      gtk_widget_destroy (GTK_WIDGET (window));
    }

  }else {
    /** Else just close **/
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

  if (marker_editor_has_unsaved_changes (editor))
    status = show_unsaved_documents_warning (window);

  if (status)
  {
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection * selection = gtk_tree_view_get_selection (window->documents_tree_view);

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      marker_editor_closing (editor);
      gtk_tree_store_remove (window->documents_tree_store, &iter);
      gtk_widget_destroy (GTK_WIDGET (editor));
      window->editors_counter--;

      if (window->editors_counter == 1)
      {
        marker_window_hide_sidebar (window);
      }
      else if (window->editors_counter < 1)
      {
        marker_window_try_close (window);
      }

      /** Select the last available row if no new is automatically selected **/
      if (!gtk_tree_selection_get_selected (selection, &model, &iter)){
        gint rows = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(window->documents_tree_store), NULL);
        if (rows)
        {
          gtk_tree_selection_select_path (selection, gtk_tree_path_new_from_indices (rows - 1, -1));
        }
      }
    }
    else{
      /** close if model is empty **/
      if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL(window->documents_tree_store), &iter))
      {
        marker_window_try_close (window);
      }
    }
  }
}

void
marker_window_toggle_sidebar (MarkerWindow *window)
{
    if (window->sidebar_visible) {
        marker_window_hide_sidebar (window);
    }
    else {
        marker_window_show_sidebar (window);
    }
}

void
marker_window_hide_sidebar (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  
  if (!window->sidebar_visible)
    return;
    
  if (!window->paned1 || !window->main_paned)
    return;
    
  if (!GTK_IS_WIDGET (window->paned1) || !GTK_IS_PANED (window->main_paned))
    return;
  
  /* Remove sidebar from paned to truly hide it */
  if (gtk_widget_get_parent (window->paned1) == GTK_WIDGET (window->main_paned)) {
    g_object_ref (window->paned1);
    gtk_container_remove (GTK_CONTAINER (window->main_paned), window->paned1);
    gtk_paned_set_position (window->main_paned, 0);
    window->sidebar_visible = FALSE;
    
    /* Update the action state to keep it in sync */
    GAction *action = g_action_map_lookup_action (G_ACTION_MAP (window), "sidebar");
    if (action) {
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (FALSE));
    }
  }
}

void
marker_window_show_sidebar (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  
  if (!window->paned1 || !window->main_paned)
    return;
    
  if (!GTK_IS_WIDGET (window->paned1) || !GTK_IS_PANED (window->main_paned))
    return;
  
  gint current_pos = gtk_paned_get_position (window->main_paned);
  
  /* If sidebar is marked visible but position is 0, fix it */
  if (window->sidebar_visible && current_pos == 0) {
    gtk_paned_set_position (window->main_paned, 200);
    return;
  }
  
  if (window->sidebar_visible && current_pos > 0)
    return;
  
  /* Add sidebar back to paned if not already there */
  if (gtk_widget_get_parent (window->paned1) == NULL) {
    gtk_paned_add1 (window->main_paned, window->paned1);
    if (g_object_is_floating (window->paned1)) {
      g_object_ref_sink (window->paned1);
    }
    g_object_unref (window->paned1);
    gtk_paned_set_position (window->main_paned, 200);
    window->sidebar_visible = TRUE;
    
    /* Update the action state to keep it in sync */
    GAction *action = g_action_map_lookup_action (G_ACTION_MAP (window), "sidebar");
    if (action) {
      g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (TRUE));
    }
  } else {
    gtk_paned_set_position (window->main_paned, 200);
    window->sidebar_visible = TRUE;
  }
}

void
marker_window_search (MarkerWindow       *window)
{
  if (window->active_editor)
  {
    marker_editor_toggle_search_bar(window->active_editor);
  }
}

void
marker_window_go_to_line (MarkerWindow       *window)
{
  if (window->active_editor)
  {
    marker_editor_go_to_line(window->active_editor);
  }
}

void 
marker_window_refresh_all_preview(MarkerWindow       *window)
{
  GtkTreeModel * model = GTK_TREE_MODEL(window->documents_tree_store);
  gint rows = gtk_tree_model_iter_n_children (model, NULL);

  if (rows > 0)
  {
    /** If there are documents open check for unsaved ones**/
    gint i;
    GtkTreeIter iter;
    for (i = 0; i < rows; i++)
    {
      gtk_tree_model_get_iter(model, &iter, gtk_tree_path_new_from_indices (i, -1));
      MarkerEditor *editor;
      gtk_tree_model_get (model, &iter, EDITOR_COLUMN, &editor, -1);
      if (editor) {
        marker_editor_refresh_preview (editor);  
      }
    }
  }
}
