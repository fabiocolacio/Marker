/*
 * marker-sketcher-window.h
 *
 * Copyright (C) 2017 - 2018 Marker Project
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

#ifndef __MARKER_SKETCHER_WINDOW_H__
#define __MARKER_SKETCHER_WINDOW_H__

#include <gtk/gtk.h>
#include <gio/gio.h>

#include "marker-source-view.h"

G_BEGIN_DECLS

typedef enum
{
  PEN,
  ERASER,
  TEXT
} SketchTool;

#define MARKER_TYPE_SKETCHER_WINDOW (marker_sketcher_window_get_type ())

G_DECLARE_FINAL_TYPE (MarkerSketcherWindow, marker_sketcher_window, MARKER, SKETCHER_WINDOW, GtkWindow)

MarkerSketcherWindow   *marker_sketcher_window_new              (GtkApplication     *app);
MarkerSketcherWindow   *marker_sketcher_window_show             (GtkWindow          *parent,
                                                                 GFile              *file,
                                                                 MarkerSourceView   *source_view);

G_END_DECLS

#endif
