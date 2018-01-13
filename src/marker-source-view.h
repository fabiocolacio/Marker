/*
 * marker-source-view.h
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MARKER_SOURCE_VIEW_H__
#define __MARKER_SOURCE_VIEW_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define MARKER_TYPE_SOURCE_VIEW (marker_source_view_get_type ())

G_DECLARE_FINAL_TYPE (MarkerSourceView, marker_source_view, MARKER, SOURCE_VIEW, GtkSourceView)

MarkerSourceView *   marker_source_view_new                      (void);
gboolean             marker_source_view_get_modified             (MarkerSourceView   *source_view);
void                 marker_source_view_set_modified             (MarkerSourceView   *source_view,
                                                                  gboolean            modified);
gchar *              marker_source_view_get_text                 (MarkerSourceView   *source_view);
void                 marker_source_view_set_text                 (MarkerSourceView   *source_view,
                                                                  const char         *text,
                                                                  size_t              size);
void                 marker_source_view_set_language             (MarkerSourceView   *source_view,
                                                                  const gchar        *language);
void                 marker_source_view_set_syntax_theme         (MarkerSourceView   *source_view,
                                                                  const char         *syntax_theme);
void                 marker_source_view_surround_selection_with  (MarkerSourceView   *source_view,
                                                                  const char         *insertion);
void                 marker_source_view_set_spell_check          (MarkerSourceView   *source_view,
                                                                  gboolean            state);
void                 marker_source_view_set_spell_check_lang     (MarkerSourceView   *source_view,
                                                                  const gchar        *lang);

G_END_DECLS

#endif
