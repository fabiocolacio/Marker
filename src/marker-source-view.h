/*
 * marker-source-view.h
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

#ifndef __MARKER_SOURCE_VIEW_H__
#define __MARKER_SOURCE_VIEW_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define MARKER_TYPE_SOURCE_VIEW (marker_source_view_get_type ())

G_DECLARE_FINAL_TYPE (MarkerSourceView, marker_source_view, MARKER, SOURCE_VIEW, GtkSourceView)

MarkerSourceView        *marker_source_view_new                      (void);
gboolean                 marker_source_view_get_modified             (MarkerSourceView   *source_view);
void                     marker_source_view_set_modified             (MarkerSourceView   *source_view,
                                                                      gboolean            modified);
gchar                   *marker_source_view_get_text                 (MarkerSourceView   *source_view,
                                                                      gboolean            include_position);
void                     marker_source_view_set_text                 (MarkerSourceView   *source_view,
                                                                      const char         *text,
                                                                      size_t              size);
void                     marker_source_view_set_language             (MarkerSourceView   *source_view,
                                                                      const gchar        *language);
void                     marker_source_view_set_syntax_theme         (MarkerSourceView   *source_view,
                                                                      const char         *syntax_theme);
void                     marker_source_view_surround_selection_with  (MarkerSourceView   *source_view,
                                                                      const char         *insertion);
void                     marker_source_view_insert_link              (MarkerSourceView   *source_view);
void                     marker_source_view_insert_image             (MarkerSourceView   *source_view,
                                                                      const char         *image_path);
void                     marker_source_view_set_spell_check          (MarkerSourceView   *source_view,
                                                                      gboolean            state);
void                     marker_source_view_set_spell_check_lang     (MarkerSourceView   *source_view,
                                                                      const gchar        *lang);
int                      marker_source_view_get_cursor_position      (MarkerSourceView   *source_view);
void                     marker_source_view_update_font              (MarkerSourceView   *source_view);
void                     marker_source_view_convert_to_bullet_list   (MarkerSourceView   *source_view);
void                     marker_source_view_convert_to_numbered_list (MarkerSourceView   *source_view);
void                     marker_source_view_align_table              (MarkerSourceView   *source_view);

GtkSourceSearchContext  *marker_source_get_search_context            (MarkerSourceView   *source_view);

G_END_DECLS

#endif
