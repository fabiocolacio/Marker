/*
 * marker-sketcher-window.c
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

#include "marker-sketcher-window.h"
#include "marker-brush-scale.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <glib/gi18n.h>

/* Text size grows with the brush size: 3 px brush -> 13 px text, 12 px -> 22 px */
#define FONT_SIZE(brush) (10.0 + (brush))

struct _MarkerSketcherWindow
{
  GtkWindow             parent_instance;

  GtkPopover           *text_popover;
  GtkEntry             *text_entry;
  GtkColorDialogButton *color_button;
  MarkerBrushScale     *brush_scale;

  GtkDrawingArea       *drawing_area;
  cairo_surface_t      *surface;

  GList                *history;
  GList                *future;

  gboolean              status;
  gdouble               pos_x;
  gdouble               pos_y;

  gdouble               size;
  SketchTool            tool;
  GdkRGBA               color;

  gchar                *base_file;
  MarkerSourceView     *source_view;
};

G_DEFINE_FINAL_TYPE (MarkerSketcherWindow, marker_sketcher_window, GTK_TYPE_WINDOW)

static const gchar noname[] = "Untitled.md";

static void
free_surface_list (GList *list)
{
  g_list_free_full (list, (GDestroyNotify) cairo_surface_destroy);
}

static void
clear_surface (cairo_surface_t *surface)
{
  cairo_t *cr = cairo_create (surface);
  cairo_set_source_rgba (cr, 1, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);
}

static cairo_surface_t *
copy_surface (cairo_surface_t *source,
              int              width,
              int              height)
{
  cairo_surface_t *destination = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  clear_surface (destination);
  if (source)
  {
    cairo_t *cr = cairo_create (destination);
    cairo_set_source_surface (cr, source, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);
  }
  return destination;
}

static void
resize_cb (GtkDrawingArea *area,
           gint            width,
           gint            height,
           gpointer        data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);

  if (width <= 0 || height <= 0)
    return;

  cairo_surface_t *destination = copy_surface (w->surface, width, height);
  g_clear_pointer (&w->surface, cairo_surface_destroy);
  w->surface = destination;
}

static void
draw_cb (GtkDrawingArea *area,
         cairo_t        *cr,
         gint            width,
         gint            height,
         gpointer        data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);

  if (!w->surface)
    return;

  cairo_set_source_surface (cr, w->surface, 0, 0);
  cairo_paint (cr);
}

static void
draw_text (MarkerSketcherWindow *w)
{
  const gchar *text = gtk_editable_get_text (GTK_EDITABLE (w->text_entry));

  if (!w->surface || !text || !*text)
    return;

  cairo_t *cr = cairo_create (w->surface);
  cairo_set_font_size (cr, FONT_SIZE (w->size));
  cairo_set_source_rgba (cr, w->color.red, w->color.green, w->color.blue, w->color.alpha);
  cairo_move_to (cr, w->pos_x, w->pos_y);
  cairo_show_text (cr, text);
  cairo_destroy (cr);

  gtk_widget_queue_draw (GTK_WIDGET (w->drawing_area));
}

static void
draw_brush (MarkerSketcherWindow *w,
            gdouble               x,
            gdouble               y)
{
  if (!w->surface)
    return;

  cairo_t *cr = cairo_create (w->surface);

  if (w->tool == ERASER)
    cairo_set_source_rgb (cr, 1, 1, 1);
  else
    cairo_set_source_rgba (cr, w->color.red, w->color.green, w->color.blue, w->color.alpha);

  if (!w->status)
  {
    cairo_arc (cr, x, y, w->size / 2, 0, 2.0 * M_PI);
    cairo_fill (cr);
  }
  else
  {
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width (cr, w->size);
    cairo_move_to (cr, w->pos_x, w->pos_y);
    cairo_line_to (cr, x, y);
    cairo_stroke (cr);
  }

  cairo_destroy (cr);
  gtk_widget_queue_draw (GTK_WIDGET (w->drawing_area));

  w->status = TRUE;
  w->pos_x = x;
  w->pos_y = y;
}

static void
add_history (MarkerSketcherWindow *w)
{
  if (!w->surface)
    return;

  int width = cairo_image_surface_get_width (w->surface);
  int height = cairo_image_surface_get_height (w->surface);

  w->history = g_list_append (w->history, copy_surface (w->surface, width, height));

  if (w->future)
  {
    free_surface_list (w->future);
    w->future = NULL;
  }
}

static void
drag_begin_cb (GtkGestureDrag *gesture,
               gdouble         start_x,
               gdouble         start_y,
               gpointer        data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);

  if (w->surface == NULL)
    return;

  if (w->tool == PEN || w->tool == ERASER)
  {
    if (!w->status)
      add_history (w);
    draw_brush (w, start_x, start_y);
  }
  else
  {
    GdkRectangle rect = { (int) start_x, (int) start_y, 1, 1 };
    w->pos_x = start_x;
    w->pos_y = start_y;
    gtk_popover_set_pointing_to (w->text_popover, &rect);
    gtk_popover_popup (w->text_popover);
    gtk_widget_grab_focus (GTK_WIDGET (w->text_entry));
  }
}

static void
drag_update_cb (GtkGestureDrag *gesture,
                gdouble         offset_x,
                gdouble         offset_y,
                gpointer        data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);
  gdouble start_x, start_y;

  if (w->surface == NULL || w->tool == TEXT)
    return;

  gtk_gesture_drag_get_start_point (gesture, &start_x, &start_y);
  draw_brush (w, start_x + offset_x, start_y + offset_y);
}

static void
drag_end_cb (GtkGestureDrag *gesture,
             gdouble         offset_x,
             gdouble         offset_y,
             gpointer        data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);
  w->status = FALSE;
}

static void
set_tool (MarkerSketcherWindow *w,
          SketchTool            tool,
          const gchar          *cursor)
{
  w->tool = tool;
  gtk_widget_set_cursor_from_name (GTK_WIDGET (w->drawing_area), cursor);
}

static void
pen_toggled_cb (GtkToggleButton *button, gpointer data)
{
  if (gtk_toggle_button_get_active (button))
    set_tool (MARKER_SKETCHER_WINDOW (data), PEN, "crosshair");
}

static void
eraser_toggled_cb (GtkToggleButton *button, gpointer data)
{
  if (gtk_toggle_button_get_active (button))
    set_tool (MARKER_SKETCHER_WINDOW (data), ERASER, "cell");
}

static void
text_toggled_cb (GtkToggleButton *button, gpointer data)
{
  if (gtk_toggle_button_get_active (button))
    set_tool (MARKER_SKETCHER_WINDOW (data), TEXT, "text");
}

static void
brush_size_changed_cb (MarkerBrushScale *scale,
                       GParamSpec       *pspec,
                       gpointer          data)
{
  MARKER_SKETCHER_WINDOW (data)->size = marker_brush_scale_get_value (scale);
}

static void
color_changed_cb (GtkColorDialogButton *button,
                  GParamSpec           *pspec,
                  gpointer              data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);
  w->color = *gtk_color_dialog_button_get_rgba (button);
  marker_brush_scale_set_color (w->brush_scale, &w->color);
}

static gchar *
create_unique_name (const gchar *base)
{
  g_autoptr (GRand) rand = g_rand_new ();
  int uuid = g_rand_int_range (rand, 0, 9999);
  return g_strdup_printf ("%s.%d.png", base, uuid);
}

static void
insert_sketch_cb (GtkButton *button,
                  gpointer   data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);

  if (w->surface)
  {
    g_autofree gchar *fpath = create_unique_name (w->base_file);
    cairo_surface_write_to_png (w->surface, fpath);
    marker_source_view_insert_image (w->source_view, fpath);
  }

  gtk_window_destroy (GTK_WINDOW (w));
}

static void
close_cb (GtkButton *button,
          gpointer   data)
{
  gtk_window_destroy (GTK_WINDOW (data));
}

static void
redo (MarkerSketcherWindow *w)
{
  GList *last = g_list_last (w->future);
  if (last)
  {
    w->history = g_list_append (w->history, w->surface);
    w->surface = last->data;
    w->future = g_list_delete_link (w->future, last);
    gtk_widget_queue_draw (GTK_WIDGET (w->drawing_area));
  }
}

static void
undo (MarkerSketcherWindow *w)
{
  GList *last = g_list_last (w->history);
  if (last)
  {
    w->future = g_list_append (w->future, w->surface);
    w->surface = last->data;
    w->history = g_list_delete_link (w->history, last);
    gtk_widget_queue_draw (GTK_WIDGET (w->drawing_area));
  }
}

static gboolean
key_pressed_cb (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                gpointer               data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);

  if ((state & GDK_CONTROL_MASK) == 0)
    return FALSE;

  switch (keyval)
  {
    case GDK_KEY_z:
      if (state & GDK_SHIFT_MASK)
        redo (w);
      else
        undo (w);
      return TRUE;
    case GDK_KEY_Z:
      redo (w);
      return TRUE;
    case GDK_KEY_y:
      redo (w);
      return TRUE;
  }
  return FALSE;
}

static void
add_text_cb (GtkWidget *widget,
             gpointer   data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);
  gtk_popover_popdown (w->text_popover);
  add_history (w);
  draw_text (w);
  gtk_editable_set_text (GTK_EDITABLE (w->text_entry), "");
}

static void
close_text_cb (GtkButton *button,
               gpointer   data)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (data);
  gtk_popover_popdown (w->text_popover);
  gtk_editable_set_text (GTK_EDITABLE (w->text_entry), "");
}

static void
connect_toggle (GtkBuilder  *builder,
                const gchar *id,
                GCallback    callback,
                gpointer     data)
{
  GObject *button = gtk_builder_get_object (builder, id);
  g_return_if_fail (button != NULL);
  g_signal_connect (button, "toggled", callback, data);
}

static void
connect_clicked (GtkBuilder  *builder,
                 const gchar *id,
                 GCallback    callback,
                 gpointer     data)
{
  GObject *button = gtk_builder_get_object (builder, id);
  g_return_if_fail (button != NULL);
  g_signal_connect (button, "clicked", callback, data);
}

static void
init_ui (MarkerSketcherWindow *window)
{
  g_type_ensure (MARKER_TYPE_BRUSH_SCALE);

  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/github/fabiocolacio/marker/ui/marker-sketcher-window.ui");

  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_title (GTK_WINDOW (window), _("Sketcher"));

  GtkWidget *header_bar = GTK_WIDGET (gtk_builder_get_object (builder, "header_bar"));
  gtk_window_set_titlebar (GTK_WINDOW (window), header_bar);

  /* Drawing area */
  GtkDrawingArea *drawing_area = GTK_DRAWING_AREA (gtk_drawing_area_new ());
  window->drawing_area = drawing_area;
  gtk_widget_set_hexpand (GTK_WIDGET (drawing_area), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (drawing_area), TRUE);
  gtk_widget_set_margin_start (GTK_WIDGET (drawing_area), 5);
  gtk_widget_set_margin_end (GTK_WIDGET (drawing_area), 5);
  gtk_widget_set_margin_top (GTK_WIDGET (drawing_area), 5);
  gtk_widget_set_margin_bottom (GTK_WIDGET (drawing_area), 5);
  gtk_drawing_area_set_draw_func (drawing_area, draw_cb, window, NULL);
  g_signal_connect (drawing_area, "resize", G_CALLBACK (resize_cb), window);
  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (drawing_area));

  /* Text popover, anchored to the drawing area */
  window->text_popover = GTK_POPOVER (gtk_builder_get_object (builder, "text_popover"));
  window->text_entry = GTK_ENTRY (gtk_builder_get_object (builder, "text_entry"));
  gtk_widget_set_parent (GTK_WIDGET (window->text_popover), GTK_WIDGET (drawing_area));
  gtk_popover_set_position (window->text_popover, GTK_POS_LEFT);

  /* Color */
  window->color_button = GTK_COLOR_DIALOG_BUTTON (gtk_builder_get_object (builder, "color_selection_button"));
  gtk_color_dialog_button_set_rgba (window->color_button, &window->color);
  g_signal_connect (window->color_button, "notify::rgba", G_CALLBACK (color_changed_cb), window);

  /* Brush size */
  window->brush_scale = MARKER_BRUSH_SCALE (gtk_builder_get_object (builder, "brush_scale"));
  marker_brush_scale_set_value (window->brush_scale, window->size);
  marker_brush_scale_set_color (window->brush_scale, &window->color);
  g_signal_connect (window->brush_scale, "notify::value", G_CALLBACK (brush_size_changed_cb), window);

  /* Input */
  GtkGesture *drag = gtk_gesture_drag_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
  g_signal_connect (drag, "drag-begin", G_CALLBACK (drag_begin_cb), window);
  g_signal_connect (drag, "drag-update", G_CALLBACK (drag_update_cb), window);
  g_signal_connect (drag, "drag-end", G_CALLBACK (drag_end_cb), window);
  gtk_widget_add_controller (GTK_WIDGET (drawing_area), GTK_EVENT_CONTROLLER (drag));

  GtkEventController *key = gtk_event_controller_key_new ();
  g_signal_connect (key, "key-pressed", G_CALLBACK (key_pressed_cb), window);
  gtk_widget_add_controller (GTK_WIDGET (window), key);

  /* Buttons */
  connect_toggle (builder, "pen_radio_button", G_CALLBACK (pen_toggled_cb), window);
  connect_toggle (builder, "eraser_radio_button", G_CALLBACK (eraser_toggled_cb), window);
  connect_toggle (builder, "text_radio_button", G_CALLBACK (text_toggled_cb), window);
  connect_clicked (builder, "insert_button", G_CALLBACK (insert_sketch_cb), window);
  connect_clicked (builder, "close_button", G_CALLBACK (close_cb), window);
  connect_clicked (builder, "add_text_button", G_CALLBACK (add_text_cb), window);
  connect_clicked (builder, "close_text_button", G_CALLBACK (close_text_cb), window);
  g_signal_connect (window->text_entry, "activate", G_CALLBACK (add_text_cb), window);

  set_tool (window, PEN, "crosshair");

  g_object_unref (builder);
}

MarkerSketcherWindow *
marker_sketcher_window_show (GtkWindow        *parent,
                             GFile            *file,
                             MarkerSourceView *source_view)
{
  GtkApplication *app = gtk_window_get_application (parent);
  MarkerSketcherWindow *w = marker_sketcher_window_new (app);
  gtk_window_set_transient_for (GTK_WINDOW (w), parent);
  w->source_view = source_view;

  if (file)
  {
    w->base_file = g_file_get_path (file);
  }
  else
  {
    g_autofree gchar *current_dir = g_get_current_dir ();
    w->base_file = g_build_filename (current_dir, noname, NULL);
  }

  gtk_window_present (GTK_WINDOW (w));
  return w;
}

MarkerSketcherWindow *
marker_sketcher_window_new (GtkApplication *app)
{
  return g_object_new (MARKER_TYPE_SKETCHER_WINDOW, "application", app, NULL);
}

static void
marker_sketcher_window_dispose (GObject *object)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (object);

  if (w->text_popover)
  {
    gtk_widget_unparent (GTK_WIDGET (w->text_popover));
    w->text_popover = NULL;
  }

  G_OBJECT_CLASS (marker_sketcher_window_parent_class)->dispose (object);
}

static void
marker_sketcher_window_finalize (GObject *object)
{
  MarkerSketcherWindow *w = MARKER_SKETCHER_WINDOW (object);

  g_clear_pointer (&w->surface, cairo_surface_destroy);
  free_surface_list (w->history);
  free_surface_list (w->future);
  g_clear_pointer (&w->base_file, g_free);

  G_OBJECT_CLASS (marker_sketcher_window_parent_class)->finalize (object);
}

static void
marker_sketcher_window_class_init (MarkerSketcherWindowClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->dispose = marker_sketcher_window_dispose;
  object_class->finalize = marker_sketcher_window_finalize;
}

static void
marker_sketcher_window_init (MarkerSketcherWindow *sketcher)
{
  sketcher->surface = NULL;
  sketcher->history = NULL;
  sketcher->future = NULL;
  sketcher->status = FALSE;
  sketcher->pos_x = 0;
  sketcher->pos_y = 0;
  sketcher->tool = PEN;
  sketcher->size = 6;

  sketcher->color.red = 0;
  sketcher->color.green = 0;
  sketcher->color.blue = 0;
  sketcher->color.alpha = 1;

  init_ui (sketcher);
}
