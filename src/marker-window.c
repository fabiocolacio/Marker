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
  
  GtkBox               *header_box;
  GtkHeaderBar         *header_bar;
  GtkButton            *zoom_original_btn;
  
  gboolean              is_fullscreen;
  GtkButton            *unfullscreen_btn;
  
  GtkBox               *vbox;
  MarkerEditor         *editor;
};

G_DEFINE_TYPE (MarkerWindow, marker_window, GTK_TYPE_APPLICATION_WINDOW);

static gboolean
key_pressed_cb (GtkWidget   *widget,
                GdkEventKey *event,
                gpointer     user_data)
{
  MarkerWindow *window = MARKER_WINDOW (widget);

  switch (event->keyval)
  {
    case GDK_KEY_F11:
      marker_window_toggle_fullscreen (window);
      break;
  }

  return FALSE;
}

void
marker_window_fullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));  
  g_return_if_fail (!marker_window_is_fullscreen (window));
  
  window->is_fullscreen = TRUE;
  gtk_window_fullscreen (GTK_WINDOW (window));
  
  GtkBox * const header_box = window->header_box;
  GtkBox * const vbox = window->vbox;
  GtkWidget * const header_bar = GTK_WIDGET (window->header_bar);
  GtkWidget * const editor = GTK_WIDGET (window->editor);
  
  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (header_box), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), FALSE);
  gtk_widget_show (GTK_WIDGET (window->unfullscreen_btn));
  
  g_object_ref (editor);
  gtk_container_remove (GTK_CONTAINER (vbox), editor);
  
  gtk_box_pack_start (vbox, header_bar, FALSE, TRUE, 0);
  gtk_box_pack_start (vbox, editor, TRUE, TRUE, 0);
}

void
marker_window_unfullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  g_return_if_fail (marker_window_is_fullscreen (window));
  
  window->is_fullscreen = FALSE;
  gtk_window_unfullscreen (GTK_WINDOW (window));
  
  GtkBox * const vbox = window->vbox;
  GtkBox * const header_box = window->header_box;
  GtkWidget * const header_bar = GTK_WIDGET (window->header_bar);
  
  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (vbox), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), TRUE);
  gtk_widget_hide (GTK_WIDGET (window->unfullscreen_btn));
  
  gtk_box_pack_start (header_box, header_bar, FALSE, TRUE, 0);
}

static void
marker_window_init (MarkerWindow *window)
{ 
  window->is_fullscreen = FALSE;
  
    GtkBuilder *builder = gtk_builder_new ();

  /** VBox **/
  GtkBox *vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->vbox = vbox;
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));
  gtk_widget_show (GTK_WIDGET (vbox));
  
  /** Editor **/
  MarkerEditor *editor = marker_editor_new ();
  window->editor = editor;
  gtk_box_pack_start (vbox, GTK_WIDGET (editor), TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (editor));
  
  /** HeaderBar **/
  GtkBox *header_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->header_box = header_box;
  gtk_window_set_titlebar (GTK_WINDOW (window), GTK_WIDGET (header_box));
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-headerbar.ui", NULL);
  GtkHeaderBar *header_bar = GTK_HEADER_BAR (gtk_builder_get_object (builder, "header_bar"));
  window->header_bar = header_bar;
  GtkButton *unfullscreen_btn = GTK_BUTTON (gtk_builder_get_object (builder, "unfullscreen_btn"));
  window->unfullscreen_btn = unfullscreen_btn;
  g_signal_connect_swapped (unfullscreen_btn, "clicked", G_CALLBACK (marker_window_unfullscreen), window);
  gtk_header_bar_set_show_close_button (header_bar, TRUE);
  gtk_box_pack_start (header_box, GTK_WIDGET (header_bar), FALSE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (header_box));
  
  /** Popover **/
  GtkMenuButton *menu_btn = GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn")); 
  gtk_builder_add_from_resource (builder, "/com/github/fabiocolacio/marker/ui/marker-gear-popover.ui", NULL);
  GtkWidget *popover = GTK_WIDGET (gtk_builder_get_object (builder, "gear_menu_popover"));
  window->zoom_original_btn = GTK_BUTTON (gtk_builder_get_object (builder, "zoom_original_btn"));
  gtk_menu_button_set_use_popover (menu_btn, TRUE);
  gtk_menu_button_set_popover (menu_btn, popover);
  gtk_menu_button_set_direction (menu_btn, GTK_ARROW_DOWN);
  if (!marker_has_app_menu ())
  {
    GtkWidget *extra_items = GTK_WIDGET (gtk_builder_get_object (builder, "appmenu_popover_items"));
    GtkBox *popover_vbox = GTK_BOX (gtk_builder_get_object (builder, "gear_menu_popover_vbox"));
    gtk_box_pack_end (popover_vbox, extra_items, FALSE, FALSE, 0);
    GtkApplication* app = marker_get_app ();
    g_action_map_add_action_entries (G_ACTION_MAP(app),
                                     APP_MENU_ACTION_ENTRIES,
                                     APP_MENU_ACTION_ENTRIES_LEN,
                                     window);
  }
  
  /** Window **/
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  
  g_object_unref (builder);
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

void
marker_window_toggle_fullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  if (marker_window_is_fullscreen (window))
  {
    marker_window_unfullscreen (window);
  }
  else
  {
    marker_window_fullscreen (window);
  }
}

gboolean
marker_window_is_fullscreen (MarkerWindow *window)
{
  return window->is_fullscreen;
}
