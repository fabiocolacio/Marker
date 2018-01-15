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

#include "marker-prefs.h"

#include "marker-editor.h"

struct _MarkerEditor
{
  GtkBox               parent_instance;
  
  GFile                *file;
  
  GtkPaned             *paned;
  MarkerPreview        *preview;
  MarkerSourceView     *source_view;
  GtkScrolledWindow    *source_scroll;
  MarkerViewMode        view_mode;
  
  gboolean              needs_refresh;
};

G_DEFINE_TYPE (MarkerEditor, marker_editor, GTK_TYPE_BOX);

static gboolean
refresh_timeout_cb (gpointer user_data)
{
  MarkerEditor *editor = user_data;
  if (editor->needs_refresh)
    marker_editor_refresh_preview (editor);
  return G_SOURCE_CONTINUE;
}

static void
buffer_changed_cb (GtkTextBuffer *buffer,
                   gpointer user_data)
{
  MarkerEditor *editor = user_data;
  editor->needs_refresh = TRUE;
}

static void
marker_editor_init (MarkerEditor *editor)
{
  editor->file = NULL;
  
  editor->paned = GTK_PANED (gtk_paned_new (GTK_ORIENTATION_HORIZONTAL));
  gtk_widget_show (GTK_WIDGET (editor->paned));
  gtk_box_pack_start (GTK_BOX (editor), GTK_WIDGET (editor->paned), TRUE, TRUE, 0);
  
  editor->preview = marker_preview_new ();
  gtk_widget_show (GTK_WIDGET (editor->preview));
  
  editor->source_view = marker_source_view_new ();
  gtk_widget_show (GTK_WIDGET (editor->source_view));
  editor->source_scroll = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
  gtk_widget_show (GTK_WIDGET (editor->source_scroll));
  gtk_container_add (GTK_CONTAINER (editor->source_scroll), GTK_WIDGET (editor->source_view));
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->source_view));
  g_signal_connect (buffer, "changed", G_CALLBACK (buffer_changed_cb), editor);
  
  editor->view_mode = marker_prefs_get_default_view_mode ();
  editor->needs_refresh = TRUE;
  
  marker_editor_set_view_mode (editor, editor->view_mode);
  gtk_widget_show (GTK_WIDGET (editor));
  
  g_timeout_add (20, refresh_timeout_cb, editor);
}

static void
marker_editor_class_init (MarkerEditorClass *class)
{

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
  MarkerEditor *editor = g_object_new (MARKER_TYPE_EDITOR,
                                       "orientation", GTK_ORIENTATION_VERTICAL,
                                       "spacing", 0, NULL);
  marker_editor_open_file (editor, file);
  return editor;
}

void
marker_editor_refresh_preview (MarkerEditor *editor)
{
  g_return_if_fail (MARKER_IS_EDITOR (editor));
  
  editor->needs_refresh = FALSE;
  
  gchar *markdown = marker_source_view_get_text (editor->source_view);
  
  const char* css_theme = (marker_prefs_get_use_css_theme()) ? marker_prefs_get_css_theme() : NULL;
  gchar *uri = (G_IS_FILE(editor->file)) ? g_file_get_uri(editor->file) : NULL;

  marker_preview_render_markdown(editor->preview, markdown, css_theme, uri);
  
  if (uri) g_free(uri);
  g_free (markdown);
}

MarkerViewMode
marker_editor_get_view_mode (MarkerEditor *editor)
{
  g_return_val_if_fail (MARKER_IS_EDITOR (editor), DUAL_PANE_MODE);
  return editor->view_mode;
}

void
marker_editor_set_view_mode (MarkerEditor   *editor,
                             MarkerViewMode  view_mode)
{
  g_return_if_fail (MARKER_IS_EDITOR (editor));

  editor->view_mode = view_mode;
  
  GtkWidget * const paned = GTK_WIDGET (editor->paned);
  GtkWidget * const preview = GTK_WIDGET (editor->preview);
  GtkWidget * const source_scroll = GTK_WIDGET (editor->source_scroll);
  GtkContainer *parent;
  
  parent = GTK_CONTAINER (gtk_widget_get_parent (preview));
  if (parent)
  {
    g_object_ref (preview);
    gtk_container_remove (parent, preview);
  }
  
  parent = GTK_CONTAINER (gtk_widget_get_parent (source_scroll));
  if (parent)
  {
    g_object_ref (source_scroll);
    gtk_container_remove (parent, source_scroll);
  }
  
  switch (view_mode)
  {
    case EDITOR_ONLY_MODE:
      gtk_paned_add1 (GTK_PANED (paned), source_scroll);
      break;
    
    case PREVIEW_ONLY_MODE:
      gtk_paned_add2 (GTK_PANED (paned), preview);
      break;
    
    case DUAL_WINDOW_MODE:
    case DUAL_PANE_MODE:
      gtk_paned_add1 (GTK_PANED (paned), source_scroll);
      gtk_paned_add2 (GTK_PANED (paned), preview);
      break;
  }
}

void
marker_editor_open_file (MarkerEditor *editor,
                        GFile        *file)
{
  g_return_if_fail (MARKER_IS_EDITOR (editor));
  
  if (G_IS_FILE (editor->file))
    g_object_unref (editor->file);
  
  editor->file = file;
  
  gchar *file_contents = NULL;
  gsize file_size = 0;
  GError *err = NULL;
  
  g_file_load_contents (file, NULL, &file_contents, &file_size, NULL, &err);
  
  if (err)
  {
    g_error_free (err);
  }
  else
  {
    MarkerSourceView *source_view = editor->source_view;
    GtkSourceBuffer *buffer =
      GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));
    gtk_source_buffer_begin_not_undoable_action (buffer);
    marker_source_view_set_text (source_view, file_contents, file_size);
    gtk_source_buffer_end_not_undoable_action (buffer);
    g_free (file_contents);
  }
}
