/*
 * marker.h
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

#ifndef __MARKER_H__
#define __MARKER_H__

#include <gtk/gtk.h>

GtkApplication*
marker_get_app();

void
marker_create_new_window();

void
marker_create_new_window_from_file(GFile* file);

void
marker_quit();

void
marker_prefs_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data);

void
marker_about_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data);

void
marker_quit_cb(GSimpleAction*  action,
               GVariant*       parameter,
               gpointer        user_data);

void
new_cb(GSimpleAction *action,
       GVariant      *parameter,
       gpointer       user_data);

void
marker_shortcuts_cb(GSimpleAction* action,
                    GVariant*      parameter,
                    gpointer       user_data);

gboolean
marker_has_app_menu();

extern const int APP_MENU_ACTION_ENTRIES_LEN;

extern const GActionEntry APP_MENU_ACTION_ENTRIES[];

#endif
