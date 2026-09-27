/*
 * marker-editor.c
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

#include <string.h>

#include <locale.h>
#include <glib/gi18n.h>

#include "marker-prefs.h"
#include "marker-string.h"

#include "marker-editor.h"

struct _MarkerEditor
{
  GtkBox                parent_instance;

  GFile                *file;
  GFileMonitor         *file_monitor;
  gchar                *title;
  gboolean              unsaved_changes;

  GtkPaned             *paned;
  GtkBox               *vbox;           /* owned: search bar + source view */
  GtkSearchEntry       *search_entry;
  GtkSearchBar         *search_bar;
  MarkerPreview        *preview;        /* owned: may live in the paned or in its own window */
  MarkerSourceView     *source_view;
  GtkScrolledWindow    *source_scroll;
  MarkerViewMode        view_mode;

  gboolean              search_active;
  gboolean              needs_refresh;
  guint                 timer_id;

  gboolean              has_search_iter;
  GtkTextIter           search_iter;
};

G_DEFINE_FINAL_TYPE (MarkerEditor, marker_editor, GTK_TYPE_BOX)

static void
emit_signal_title_changed (MarkerEditor *editor)
{
  g_autofree gchar *title = marker_editor_get_title (editor);
  g_autofree gchar *raw_title = marker_editor_get_raw_title (editor);
  g_signal_emit_by_name (editor, "title-changed", title, raw_title);
}

static void
emit_signal_subtitle_changed (MarkerEditor *editor)
{
  g_autofree gchar *subtitle = marker_editor_get_subtitle (editor);
  g_signal_emit_by_name (editor, "subtitle-changed", subtitle);
}

static gboolean
refresh_timeout_cb (gpointer user_data)
{
  MarkerEditor *editor = user_data;
  if (editor->needs_refresh)
    marker_editor_refresh_preview (editor);
  return G_SOURCE_CONTINUE;
}

static void
load_file_into_buffer (MarkerEditor *editor,
                       GFile        *file,
                       gboolean      block_changed)
{
  g_autofree gchar *file_contents = NULL;
  gsize file_size = 0;
  g_autoptr (GError) err = NULL;

  if (!g_file_load_contents (file, NULL, &file_contents, &file_size, NULL, &err))
  {
    g_warning ("Unable to load file: %s", err ? err->message : "unknown error");
    return;
  }

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->source_view));
  gtk_text_buffer_begin_irreversible_action (buffer);
  marker_source_view_set_text (editor->source_view, file_contents, file_size);
  gtk_text_buffer_end_irreversible_action (buffer);
}

static void
file_changed_cb (GFileMonitor      *monitor,
                 GFile             *file,
                 GFile             *other_file,
                 GFileMonitorEvent  event_type,
                 gpointer           user_data)
{
  MarkerEditor *editor = MARKER_EDITOR (user_data);
  g_assert (MARKER_IS_EDITOR (editor));

  if (file && (event_type == G_FILE_MONITOR_EVENT_CHANGED ||
               event_type == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT ||
               event_type == G_FILE_MONITOR_EVENT_CREATED))
  {
    load_file_into_buffer (editor, file, FALSE);
    editor->unsaved_changes = FALSE;
  }
}

static void
buffer_changed_cb (GtkTextBuffer *buffer,
                   gpointer       user_data)
{
  MarkerEditor *editor = user_data;
  if (editor->view_mode != PREVIEW_ONLY_MODE)
    editor->unsaved_changes = TRUE;
  editor->needs_refresh = TRUE;
  editor->has_search_iter = FALSE;
  emit_signal_title_changed (editor);
}

static gboolean
preview_window_close_request_cb (GtkWindow *preview_window,
                                 gpointer   user_data)
{
  MarkerEditor *editor = MARKER_EDITOR (user_data);
  /* Closing the detached preview switches back to the editor-only layout */
  marker_editor_set_view_mode (editor, EDITOR_ONLY_MODE);
  return TRUE;
}

static void
search_text_changed (GtkSearchEntry *entry,
                     MarkerEditor   *editor)
{
  GtkSourceSearchContext *context = marker_source_get_search_context (editor->source_view);
  GtkSourceSearchSettings *settings = gtk_source_search_context_get_settings (context);
  gtk_source_search_settings_set_search_text (settings, gtk_editable_get_text (GTK_EDITABLE (entry)));
}

static void
search_next (GtkSearchEntry *entry,
             MarkerEditor   *editor)
{
  GtkSourceSearchContext *context = marker_source_get_search_context (editor->source_view);
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (gtk_source_search_context_get_buffer (context));

  GtkTextIter iter;
  GtkTextIter close;
  gtk_text_buffer_get_end_iter (buffer, &close);

  if (editor->has_search_iter)
  {
    iter = editor->search_iter;
    if (gtk_text_iter_compare (&iter, &close) == 0)
      gtk_text_buffer_get_start_iter (buffer, &iter);
  }
  else
  {
    gtk_text_buffer_get_start_iter (buffer, &iter);
  }

  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_start_iter (buffer, &end);

  gtk_source_search_context_forward (context, &iter, &start, &end, NULL);

  if (gtk_text_iter_compare (&start, &end) != 0)
  {
    gtk_text_buffer_select_range (buffer, &start, &end);
    gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor->source_view), &start, 0, TRUE, 0, 0);
    editor->search_iter = end;
  }
  else
  {
    gtk_text_buffer_get_start_iter (buffer, &editor->search_iter);
  }
  editor->has_search_iter = TRUE;
}

static void
search_previous (GtkSearchEntry *entry,
                 MarkerEditor   *editor)
{
  GtkSourceSearchContext *context = marker_source_get_search_context (editor->source_view);
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (gtk_source_search_context_get_buffer (context));

  GtkTextIter iter;
  GtkTextIter close;
  gtk_text_buffer_get_start_iter (buffer, &close);

  if (editor->has_search_iter)
  {
    iter = editor->search_iter;
    if (gtk_text_iter_compare (&iter, &close) == 0)
      gtk_text_buffer_get_end_iter (buffer, &iter);
  }
  else
  {
    gtk_text_buffer_get_end_iter (buffer, &iter);
  }

  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_start_iter (buffer, &end);

  gtk_source_search_context_backward (context, &iter, &start, &end, NULL);

  if (gtk_text_iter_compare (&start, &end) != 0)
  {
    gtk_text_buffer_select_range (buffer, &start, &end);
    gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor->source_view), &start, 0, TRUE, 0, 0);
    editor->search_iter = start;
  }
  else
  {
    gtk_text_buffer_get_end_iter (buffer, &editor->search_iter);
  }
  editor->has_search_iter = TRUE;
}

static void
search_stopped (GtkSearchEntry *entry,
                MarkerEditor   *editor)
{
  if (editor->search_active)
    marker_editor_toggle_search_bar (editor);
  gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
}

/**
 * detach_preview:
 *
 * Removes the preview from wherever it currently lives (paned or detached
 * window). The editor keeps its own reference, so the widget survives.
 */
static void
detach_preview (MarkerEditor *editor)
{
  GtkWidget *preview = GTK_WIDGET (editor->preview);
  GtkWidget *parent = gtk_widget_get_parent (preview);

  if (!parent)
    return;

  if (GTK_IS_PANED (parent))
  {
    gtk_paned_set_end_child (GTK_PANED (parent), NULL);
  }
  else if (GTK_IS_WINDOW (parent))
  {
    gtk_window_set_child (GTK_WINDOW (parent), NULL);
    gtk_window_destroy (GTK_WINDOW (parent));
  }
  else
  {
    g_warning ("preview has an unexpected parent %s", G_OBJECT_TYPE_NAME (parent));
  }
}

static void
marker_editor_init (MarkerEditor *editor)
{
  editor->file = NULL;
  editor->file_monitor = NULL;
  editor->title = g_strdup (_("Untitled.md"));
  editor->unsaved_changes = FALSE;
  editor->search_active = FALSE;
  editor->has_search_iter = FALSE;

  editor->paned = GTK_PANED (gtk_paned_new (GTK_ORIENTATION_HORIZONTAL));
  gtk_widget_set_hexpand (GTK_WIDGET (editor->paned), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (editor->paned), TRUE);
  gtk_paned_set_resize_start_child (editor->paned, TRUE);
  gtk_paned_set_resize_end_child (editor->paned, TRUE);
  gtk_paned_set_shrink_start_child (editor->paned, FALSE);
  gtk_paned_set_shrink_end_child (editor->paned, FALSE);

  editor->vbox = GTK_BOX (g_object_ref_sink (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0)));
  gtk_widget_set_hexpand (GTK_WIDGET (editor->vbox), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (editor->vbox), TRUE);

  /** SEARCH BAR **/
  editor->search_entry = GTK_SEARCH_ENTRY (gtk_search_entry_new ());
  editor->search_bar = GTK_SEARCH_BAR (gtk_search_bar_new ());
  gtk_search_bar_set_child (editor->search_bar, GTK_WIDGET (editor->search_entry));
  gtk_search_bar_connect_entry (editor->search_bar, GTK_EDITABLE (editor->search_entry));
  gtk_search_bar_set_search_mode (editor->search_bar, FALSE);
  gtk_search_bar_set_show_close_button (editor->search_bar, TRUE);
  gtk_box_append (editor->vbox, GTK_WIDGET (editor->search_bar));

  g_signal_connect (editor->search_entry, "search-changed", G_CALLBACK (search_text_changed), editor);
  g_signal_connect (editor->search_entry, "activate", G_CALLBACK (search_next), editor);
  g_signal_connect (editor->search_entry, "next-match", G_CALLBACK (search_next), editor);
  g_signal_connect (editor->search_entry, "previous-match", G_CALLBACK (search_previous), editor);
  g_signal_connect (editor->search_entry, "stop-search", G_CALLBACK (search_stopped), editor);

  /** SOURCE VIEW **/
  editor->source_view = marker_source_view_new ();
  editor->source_scroll = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new ());
  gtk_widget_set_vexpand (GTK_WIDGET (editor->source_scroll), TRUE);
  gtk_scrolled_window_set_child (editor->source_scroll, GTK_WIDGET (editor->source_view));
  gtk_box_append (editor->vbox, GTK_WIDGET (editor->source_scroll));

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->source_view));
  g_signal_connect (buffer, "changed", G_CALLBACK (buffer_changed_cb), editor);

  /** PREVIEW **/
  editor->preview = g_object_ref_sink (marker_preview_new ());

  gtk_paned_set_position (editor->paned, 450);
  gtk_box_append (GTK_BOX (editor), GTK_WIDGET (editor->paned));

  editor->view_mode = -1;
  editor->needs_refresh = FALSE;
  marker_editor_set_view_mode (editor, marker_prefs_get_default_view_mode ());

  editor->timer_id = g_timeout_add (20, refresh_timeout_cb, editor);

  marker_editor_apply_prefs (editor);
}

static void
marker_editor_dispose (GObject *object)
{
  MarkerEditor *editor = MARKER_EDITOR (object);

  g_clear_handle_id (&editor->timer_id, g_source_remove);

  if (editor->file_monitor)
  {
    g_file_monitor_cancel (editor->file_monitor);
    g_clear_object (&editor->file_monitor);
  }

  if (editor->preview)
  {
    detach_preview (editor);
    g_clear_object (&editor->preview);
  }

  g_clear_object (&editor->vbox);
  g_clear_object (&editor->file);

  G_OBJECT_CLASS (marker_editor_parent_class)->dispose (object);
}

static void
marker_editor_finalize (GObject *object)
{
  MarkerEditor *editor = MARKER_EDITOR (object);
  g_clear_pointer (&editor->title, g_free);
  G_OBJECT_CLASS (marker_editor_parent_class)->finalize (object);
}

static void
marker_editor_class_init (MarkerEditorClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->dispose = marker_editor_dispose;
  object_class->finalize = marker_editor_finalize;

  g_signal_new ("title-changed",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                0, NULL, NULL, NULL,
                G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  g_signal_new ("subtitle-changed",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                0, NULL, NULL, NULL,
                G_TYPE_NONE, 1, G_TYPE_STRING);
}

MarkerEditor *
marker_editor_new (void)
{
  return g_object_new (MARKER_TYPE_EDITOR,
                       "orientation", GTK_ORIENTATION_VERTICAL,
                       "spacing",     0,
                       NULL);
}

MarkerEditor *
marker_editor_new_from_file (GFile *file)
{
  MarkerEditor *editor = marker_editor_new ();
  marker_editor_open_file (editor, file);
  return editor;
}

void
marker_editor_refresh_preview (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  editor->needs_refresh = FALSE;

  g_autofree gchar *markdown = marker_source_view_get_text (editor->source_view, FALSE);
  int cursor = marker_source_view_get_cursor_position (editor->source_view);

  g_autofree gchar *css_theme = (marker_prefs_get_use_css_theme ()) ? marker_prefs_get_css_theme () : NULL;
  g_autofree gchar *uri = (G_IS_FILE (editor->file)) ? g_file_get_path (editor->file) : NULL;

  marker_preview_render_markdown (editor->preview, markdown, css_theme, uri, cursor);
}

MarkerViewMode
marker_editor_get_view_mode (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->view_mode;
}

void
marker_editor_set_view_mode (MarkerEditor   *editor,
                             MarkerViewMode  view_mode)
{
  g_assert (MARKER_IS_EDITOR (editor));

  if (editor->view_mode != view_mode)
  {
    if (view_mode == PREVIEW_ONLY_MODE && editor->file)
    {
      if (!editor->file_monitor)
      {
        editor->file_monitor = g_file_monitor_file (editor->file, G_FILE_MONITOR_NONE, NULL, NULL);
        if (editor->file_monitor)
          g_signal_connect (editor->file_monitor, "changed", G_CALLBACK (file_changed_cb), editor);
      }
    }
    else if (editor->view_mode == PREVIEW_ONLY_MODE && editor->file_monitor)
    {
      g_file_monitor_cancel (editor->file_monitor);
      g_clear_object (&editor->file_monitor);
    }
  }
  editor->view_mode = view_mode;

  GtkPaned *paned = editor->paned;
  GtkWidget *preview = GTK_WIDGET (editor->preview);
  GtkWidget *source_box = GTK_WIDGET (editor->vbox);

  detach_preview (editor);

  if (gtk_widget_get_parent (source_box) == GTK_WIDGET (paned))
    gtk_paned_set_start_child (paned, NULL);

  switch (view_mode)
  {
    case EDITOR_ONLY_MODE:
      gtk_paned_set_start_child (paned, source_box);
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
      break;

    case PREVIEW_ONLY_MODE:
      gtk_paned_set_end_child (paned, preview);
      gtk_widget_grab_focus (preview);
      break;

    case DUAL_PANE_MODE:
      gtk_paned_set_start_child (paned, source_box);
      gtk_paned_set_end_child (paned, preview);
      gtk_paned_set_position (paned, marker_prefs_get_editor_pane_width ());
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
      break;

    case DUAL_WINDOW_MODE:
    {
      gtk_paned_set_start_child (paned, source_box);

      GtkWindow *preview_window = GTK_WINDOW (gtk_window_new ());
      gtk_window_set_title (preview_window, _("Preview"));
      gtk_window_set_default_size (preview_window, 500, 600);
      gtk_window_set_child (preview_window, preview);
      g_signal_connect (preview_window, "close-request",
                        G_CALLBACK (preview_window_close_request_cb), editor);

      GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (editor));
      if (GTK_IS_WINDOW (root))
        gtk_window_set_transient_for (preview_window, GTK_WINDOW (root));

      gtk_window_present (preview_window);
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
      break;
    }
  }
}

guint
marker_editor_get_pane_width (MarkerEditor *editor)
{
  if (marker_editor_get_view_mode (editor) == DUAL_PANE_MODE)
    return gtk_paned_get_position (editor->paned);
  return marker_prefs_get_editor_pane_width ();
}

void
marker_editor_open_file (MarkerEditor *editor,
                         GFile        *file)
{
  g_assert (MARKER_IS_EDITOR (editor));
  g_return_if_fail (G_IS_FILE (file));

  g_set_object (&editor->file, file);
  g_free (editor->title);
  editor->title = g_file_get_basename (file);
  editor->needs_refresh = TRUE;

  load_file_into_buffer (editor, file, FALSE);

  editor->unsaved_changes = FALSE;

  emit_signal_title_changed (editor);
  emit_signal_subtitle_changed (editor);
}

void
marker_editor_reload_file (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  GFile *file = marker_editor_get_file (editor);
  if (!file)
    return;

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->source_view));

  g_signal_handlers_block_by_func (buffer, buffer_changed_cb, editor);
  load_file_into_buffer (editor, file, TRUE);
  g_signal_handlers_unblock_by_func (buffer, buffer_changed_cb, editor);

  editor->unsaved_changes = FALSE;
  editor->needs_refresh = TRUE;
  editor->has_search_iter = FALSE;

  emit_signal_title_changed (editor);
  emit_signal_subtitle_changed (editor);
}

void
marker_editor_save_file (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  g_return_if_fail (G_IS_FILE (editor->file));

  g_autofree gchar *contents = marker_source_view_get_text (editor->source_view, FALSE);
  g_autoptr (GError) err = NULL;

  if (!g_file_replace_contents (editor->file, contents, strlen (contents), NULL, FALSE,
                                G_FILE_CREATE_NONE, NULL, NULL, &err))
  {
    g_warning ("Unable to save file: %s", err ? err->message : "unknown error");
    return;
  }

  editor->unsaved_changes = FALSE;
  emit_signal_title_changed (editor);
}

void
marker_editor_save_file_as (MarkerEditor *editor,
                            GFile        *file)
{
  g_assert (MARKER_IS_EDITOR (editor));
  g_return_if_fail (G_IS_FILE (file));

  g_set_object (&editor->file, file);
  g_free (editor->title);
  editor->title = g_file_get_basename (file);
  marker_editor_save_file (editor);
  emit_signal_subtitle_changed (editor);
}

GFile *
marker_editor_get_file (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  if (G_IS_FILE (editor->file))
    return editor->file;
  return NULL;
}

gboolean
marker_editor_has_unsaved_changes (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->unsaved_changes;
}

gchar *
marker_editor_get_title (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  if (marker_editor_has_unsaved_changes (editor))
    return g_strdup_printf ("*%s", editor->title);
  return g_strdup (editor->title);
}

gchar *
marker_editor_get_raw_title (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return g_strdup (editor->title);
}

gchar *
marker_editor_get_subtitle (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  gchar *subtitle = NULL;
  GFile *file = marker_editor_get_file (editor);

  if (G_IS_FILE (file))
  {
    g_autofree gchar *path = g_file_get_path (file);
    subtitle = marker_string_filename_get_path (path);
  }

  return subtitle;
}

MarkerPreview *
marker_editor_get_preview (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->preview;
}

MarkerSourceView *
marker_editor_get_source_view (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->source_view;
}

void
marker_editor_apply_prefs (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  GtkSourceView * const source_view = GTK_SOURCE_VIEW (marker_editor_get_source_view (editor));

  gboolean state;

  state = marker_prefs_get_show_line_numbers ();
  gtk_source_view_set_show_line_numbers (source_view, state);

  state = marker_prefs_get_wrap_text ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (source_view), (state) ? GTK_WRAP_WORD : GTK_WRAP_NONE);

  state = marker_prefs_get_show_right_margin ();
  gtk_source_view_set_show_right_margin (source_view, state);

  guint position = marker_prefs_get_right_margin_position ();
  gtk_source_view_set_right_margin_position (source_view, position);

  g_autofree gchar *lang = marker_prefs_get_spell_check_language ();
  marker_source_view_set_spell_check_lang (MARKER_SOURCE_VIEW (source_view), lang);

  state = marker_prefs_get_spell_check ();
  marker_source_view_set_spell_check (MARKER_SOURCE_VIEW (source_view), state);

  state = marker_prefs_get_highlight_current_line ();
  gtk_source_view_set_highlight_current_line (source_view, state);

  GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));
  state = marker_prefs_get_use_syntax_theme ();
  gtk_source_buffer_set_highlight_syntax (buffer, state);

  g_autofree gchar *theme = marker_prefs_get_syntax_theme ();
  marker_source_view_set_syntax_theme (MARKER_SOURCE_VIEW (source_view), theme);

  state = marker_prefs_get_auto_indent ();
  gtk_source_view_set_auto_indent (source_view, state);

  state = marker_prefs_get_replace_tabs ();
  gtk_source_view_set_insert_spaces_instead_of_tabs (source_view, state);

  state = marker_prefs_get_show_spaces ();
  GtkSourceSpaceDrawer *space_drawer = gtk_source_view_get_space_drawer (source_view);
  if (state)
  {
    gtk_source_space_drawer_set_types_for_locations (space_drawer, GTK_SOURCE_SPACE_LOCATION_ALL,
                                                     GTK_SOURCE_SPACE_TYPE_NBSP |
                                                     GTK_SOURCE_SPACE_TYPE_SPACE |
                                                     GTK_SOURCE_SPACE_TYPE_TAB);
  }
  gtk_source_space_drawer_set_enable_matrix (space_drawer, state);

  guint tab_width = marker_prefs_get_tab_width ();
  gtk_source_view_set_tab_width (source_view, tab_width);
  gtk_source_view_set_indent_width (source_view, tab_width);
}

void
marker_editor_closing (MarkerEditor *editor)
{
  g_clear_handle_id (&editor->timer_id, g_source_remove);
  editor->needs_refresh = FALSE;

  /* Close a detached preview window, if any */
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (editor->preview));
  if (GTK_IS_WINDOW (parent))
    detach_preview (editor);
}

gboolean
marker_editor_rename_file (MarkerEditor *editor,
                           const gchar  *name)
{
  g_return_val_if_fail (name != NULL && *name != '\0', FALSE);

  if (G_IS_FILE (editor->file))
  {
    g_autofree gchar *parent = marker_editor_get_subtitle (editor);
    g_autofree gchar *path = g_build_filename (parent, name, NULL);

    /* Delete old file, save under the new name */
    g_file_delete (editor->file, NULL, NULL);

    g_autoptr (GFile) file = g_file_new_for_path (path);
    marker_editor_save_file_as (editor, file);
  }
  else
  {
    g_free (editor->title);
    editor->title = g_strdup (name);
    emit_signal_title_changed (editor);
  }
  return TRUE;
}

void
marker_editor_toggle_search_bar (MarkerEditor *editor)
{
  editor->search_active = !editor->search_active;

  if (!editor->search_active)
  {
    gtk_editable_set_text (GTK_EDITABLE (editor->search_entry), "");
    editor->has_search_iter = FALSE;
  }

  gtk_search_bar_set_search_mode (editor->search_bar, editor->search_active);

  if (editor->search_active)
    gtk_widget_grab_focus (GTK_WIDGET (editor->search_entry));
}

GtkSearchBar *
marker_editor_get_search_bar (MarkerEditor *editor)
{
  return editor->search_bar;
}
