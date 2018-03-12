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

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "marker-preview.h"
#include "marker-source-view.h"

G_BEGIN_DECLS

#define MARKER_TYPE_EDITOR (marker_editor_get_type ())

G_DECLARE_FINAL_TYPE (MarkerEditor, marker_editor, MARKER, EDITOR, GtkBox)

typedef enum
{
  EDITOR_ONLY_MODE,
  PREVIEW_ONLY_MODE,
  DUAL_PANE_MODE,
  DUAL_WINDOW_MODE
} MarkerViewMode;

MarkerEditor        *marker_editor_new                           (void);
MarkerEditor        *marker_editor_new_from_file                 (GFile              *file);
MarkerViewMode       marker_editor_get_view_mode                 (MarkerEditor       *editor);
void                 marker_editor_set_view_mode                 (MarkerEditor       *editor,
                                                                  MarkerViewMode      view_mode);
void                 marker_editor_refresh_preview               (MarkerEditor       *editor);
void                 marker_editor_open_file                     (MarkerEditor       *editor,
                                                                  GFile              *file);
void                 marker_editor_save_file                     (MarkerEditor       *editor);
void                 marker_editor_save_file_as                  (MarkerEditor       *editor,
                                                                  GFile              *file);
gboolean             marker_editor_rename_file                   (MarkerEditor       *editor,
                                                                  gchar*              name);
GFile               *marker_editor_get_file                      (MarkerEditor       *editor);
gboolean             marker_editor_has_unsaved_changes           (MarkerEditor       *editor);
gchar               *marker_editor_get_title                     (MarkerEditor       *editor);
gchar               *marker_editor_get_raw_title                 (MarkerEditor       *editor);
gchar               *marker_editor_get_subtitle                  (MarkerEditor       *editor);
MarkerPreview       *marker_editor_get_preview                   (MarkerEditor       *editor);
MarkerSourceView    *marker_editor_get_source_view               (MarkerEditor       *editor);
void                 marker_editor_apply_prefs                   (MarkerEditor       *editor);
void                 marker_editor_closing                       (MarkerEditor       *editor);

void                 marker_editor_toggle_search_bar             (MarkerEditor       *editor);
GtkSearchBar        *marker_editor_get_search_bar                (MarkerEditor       *editor);

G_END_DECLS

#endif
