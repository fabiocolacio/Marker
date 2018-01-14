/*
 * marker-editor.h
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

#ifndef __MARKER_EDITOR_H__
#define __MARKER_EDITOR_H__

#include <glib-object.h>
#include <gio/gio.h>

#include "marker-preview.h"
#include "marker-source-view.h"

G_BEGIN_DECLS

#define MARKER_TYPE_EDITOR (marker_editor_get_type ())

G_DECLARE_FINAL_TYPE (MarkerEditor, marker_editor, MARKER, EDITOR, GObject)

typedef enum
{
  EDITOR_ONLY_MODE,
  PREVIEW_ONLY_MODE,
  DUAL_PANE_MODE,
  DUAL_WINDOW_MODE
} MarkerEditorViewMode;

MarkerEditor        *marker_editor_new                           (void);
MarkerEditor        *marker_editor_new_from_file                 (GFile              *file);
MarkerPreview       *marker_editor_get_preview                   (MarkerEditor       *editor);
MarkerSourceView    *marker_editor_get_source_view               (MarkerEditor       *editor);
GFile               *marker_editor_get_file                      (MarkerEditor       *editor);
gboolean             marker_editor_has_unsaved_changes           (MarkerEditor       *editor);
void                 marker_editor_save                          (MarkerEditor       *editor);
void                 marker_editor_save_as                       (MarkerEditor       *editor);
void                 marker_editor_export                        (MarkerEditor       *editor);
void                 marker_editor_print                         (MarkerEditor       *editor);
void                 marker_editor_close                         (MarkerEditor       *editor);
void                 marker_editor_set_view_mode                 (MarkerEditor       *editor,
                                                                  MarkerEditorViewMode view_mode);
MarkerEditorViewMode marker_editor_get_view_mode                 (MarkerEditor        editor);

G_END_DECLS

#endif
