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
};

G_DEFINE_TYPE (MarkerEditor, marker_editor, GTK_TYPE_BOX);

static void
init_ui (MarkerEditor *editor)
{
  gtk_box_pack_start (GTK_BOX (editor), GTK_WIDGET (editor->paned), TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (editor->source_scroll), GTK_WIDGET (editor->source_view));
}

static void
marker_editor_init (MarkerEditor *editor)
{
  editor->file = NULL;
  editor->paned = GTK_PANED (gtk_paned_new (GTK_ORIENTATION_HORIZONTAL));
  editor->preview = marker_preview_new ();
  editor->source_view = marker_source_view_new ();
  editor->source_scroll = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
  editor->view_mode = marker_prefs_get_default_view_mode ();
  
  init_ui (editor);
  marker_editor_set_view_mode (editor, editor->view_mode);
}

static void
marker_editor_class_init (MarkerEditorClass *class)
{

}

MarkerEditor *
marker_editor_new (void)
{
  MarkerEditor *editor = g_object_new (MARKER_TYPE_EDITOR, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
  return editor;
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
