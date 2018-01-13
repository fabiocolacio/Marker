/*
 * marker-utils.h
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

#ifndef __MARKER_UTILS_H__
#define __MARKER_UTILS_H__

#include <gtk/gtk.h>

void                 marker_utils_surround_selection_with        (GtkTextBuffer      *buffer,
                                                                  char               *insertion);
int                  marker_utils_rfind                          (char                query,
                                                                  char               *str);
int                  marker_utils_get_current_time_seconds       (void);
int                  marker_utils_str_starts_with                (char               *str,
                                                                  char               *sub_str);
int                  marker_utils_str_ends_with                  (char               *str,
                                                                  char               *sub_str);
char*                marker_utils_combo_box_get_active_str       (GtkComboBox        *combo_box);
void                 marker_utils_combo_box_set_model            (GtkComboBox        *combo_box,
                                                                  GtkTreeModel       *model);
char*                marker_utils_allocate_string                (char               *str);
char*                marker_utils_escape_file_path               (char               *filename);

#endif
