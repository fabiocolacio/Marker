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

#include <marker-editor.h>

struct _MarkerEditor
{
  GtkBox               parent_instance;
  
  GFile                *file;
  
  GtkPaned             *paned;
  MarkerPreview        *preview;
  MarkerSourceView     *source_view;
  MarkerViewMode        view_mode;
};

G_DEFINE_TYPE (MarkerEditor, marker_editor, GTK_TYPE_BOX);

static void
init_ui (MarkerEditor *editor)
{
  
}

static void
marker_editor_init (MarkerEditor *editor)
{
  editor->file = NULL;
  editor->paned = GTK_PANED (gtk_paned_new (GTK_ORIENTATION_HORIZONTAL));
  editor->preview = marker_preview_new ();
  editor->source_view = marker_source_view_new ();
  editor->view_mode = marker_prefs_get_default_view_mode ();
  
  init_ui (editor);
}

static void
marker_editor_class_init (MarkerEditorClass *class)
{

}

MarkerEditor *
marker_editor_new (void)
{
  MarkerEditor *editor = g_object_new (MARKER_TYPE_EDITOR, NULL);
  return editor;
}
