/*
 * marker-window.h
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

#ifndef __MARKER_WINDOW_H__
#define __MARKER_WINDOW_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MARKER_TYPE_WINDOW (marker_window_get_type ())

G_DECLARE_FINAL_TYPE (MarkerWindow, marker_window, MARKER, WINDOW, GtkApplicationWindow)

MarkerWindow        *marker_window_new                           (GtkApplication     *app);
MarkerWindow        *marker_window_new_from_file                 (GtkApplication     *app,
                                                                  GFile              *file);
void                 marker_window_fullscreen                    (MarkerWindow       *window);
void                 marker_window_unfullscreen                  (MarkerWindow       *window);
void                 marker_window_toggle_fullscreen             (MarkerWindow       *window);
gboolean             marker_window_is_fullscreen                 (MarkerWindow       *window);
MarkerEditor        *marker_window_get_active_editor             (MarkerWindow       *window);
void                 marker_window_open_file                     (MarkerWindow       *window);
void                 marker_window_save_active_file              (MarkerWindow       *window);
void                 marker_window_save_active_file_as           (MarkerWindow       *window);

G_END_DECLS

#endif
