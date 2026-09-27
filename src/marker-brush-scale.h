/*
 * marker-brush-scale.h
 *
 * Copyright (C) 2017 - 2026 Marker Project
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

#ifndef __MARKER_BRUSH_SCALE_H__
#define __MARKER_BRUSH_SCALE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MARKER_BRUSH_SCALE_MIN 1.0
#define MARKER_BRUSH_SCALE_MAX 30.0

#define MARKER_TYPE_BRUSH_SCALE (marker_brush_scale_get_type ())

G_DECLARE_FINAL_TYPE (MarkerBrushScale, marker_brush_scale, MARKER, BRUSH_SCALE, GtkWidget)

/*
 * A horizontal slider for picking a brush width. The trough is a wedge that
 * widens towards the maximum, and the handle is a dot painted in the current
 * brush colour whose diameter is exactly the selected width, so the handle
 * itself previews the stroke.
 */

GtkWidget           *marker_brush_scale_new                      (void);

gdouble              marker_brush_scale_get_value                (MarkerBrushScale   *scale);
void                 marker_brush_scale_set_value                (MarkerBrushScale   *scale,
                                                                  gdouble             value);

const GdkRGBA       *marker_brush_scale_get_color                (MarkerBrushScale   *scale);
void                 marker_brush_scale_set_color                (MarkerBrushScale   *scale,
                                                                  const GdkRGBA      *color);

G_END_DECLS

#endif
