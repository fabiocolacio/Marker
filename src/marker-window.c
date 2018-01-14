/*
 * marker-window.c
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

#include "marker.h"
#include "marker-editor.h"

#include "marker-window.h"

struct _MarkerWindow
{
  GtkApplicationWindow  parent_instance;
  
  GtkHeaderBar         *header_bar;
  GtkButton            *zoom_original_btn;
  
  GtkBox               *vbox;
  MarkerEditor         *editor;
};

G_DEFINE_TYPE (MarkerWindow, marker_window, GTK_TYPE_APPLICATION_WINDOW);

static void
init_ui (MarkerWindow *window)
{
  GtkBuilder *builder = gtk_builder_new ();

  GtkBox *vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->vbox = vbox;
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));
  
  MarkerEditor *editor = marker_editor_new ();
  window->editor = editor;
  gtk_box_pack_start (vbox, GTK_WIDGET (editor), TRUE, TRUE, 0);
  
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-headerbar.ui", NULL);
  GtkHeaderBar *header_bar = GTK_HEADER_BAR (gtk_builder_get_object (builder, "header_bar"));
  window->header_bar = header_bar;
  gtk_header_bar_set_show_close_button (header_bar, TRUE);
  gtk_window_set_titlebar (GTK_WINDOW (window), GTK_WIDGET (header_bar));
  
  GtkMenuButton *menu_btn = GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn")); 
  
  if (marker_has_app_menu ())
  {
    gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-gear-popover.ui", NULL);
      
    GtkWidget *popover = GTK_WIDGET (gtk_builder_get_object (builder, "gear_menu_popover"));
    window->zoom_original_btn = GTK_BUTTON (gtk_builder_get_object (builder, "zoom_original_btn"));

    gtk_menu_button_set_use_popover (menu_btn, TRUE);
    gtk_menu_button_set_popover (menu_btn, popover);
  }
  else
  {
    gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-gear-popover-full.ui", NULL);
    
    GtkWidget *popover = GTK_WIDGET (gtk_builder_get_object (builder, "gear_menu_popover_full"));
    window->zoom_original_btn = GTK_BUTTON (gtk_builder_get_object (builder, "zoom_original_btn"));
    
    gtk_menu_button_set_use_popover (menu_btn, TRUE);
    gtk_menu_button_set_popover (menu_btn, popover);
    
    GtkApplication* app = marker_get_app ();
    g_action_map_add_action_entries (G_ACTION_MAP(app),
                                     APP_MENU_ACTION_ENTRIES,
                                     APP_MENU_ACTION_ENTRIES_LEN,
                                     window);
  }
  
  gtk_widget_show (GTK_WIDGET (vbox));
  gtk_widget_show (GTK_WIDGET (editor));

  g_object_unref (builder);
}

static gboolean
key_pressed_cb (GtkWidget *widget,
                GdkEvent  *event,
                gpointer   user_data)
{
  return FALSE;
}

static void
marker_window_init (MarkerWindow *window)
{ 
  init_ui (window);
}

static void
marker_window_class_init (MarkerWindowClass *class)
{
  GTK_WIDGET_CLASS (class)->key_press_event = key_pressed_cb;
}

MarkerWindow *
marker_window_new (GtkApplication *app)
{
  return g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
}

MarkerWindow *
marker_window_new_from_file (GtkApplication *app,
                             GFile          *file)
{
  return g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
}
