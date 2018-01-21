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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define P_SIZE_S 3
#define P_SIZE_M 6
#define P_SIZE_L 12

#define F_SIZE_S 12
#define F_SIZE_M 14
#define F_SIZE_L 18

struct _MarkerSketcherWindow
{
  GtkWindow        parent_instance;
  
  GtkPopover* text_popover;
  GtkEntry* text_entry;
  GdkCursor* cursor;
  
  GtkDrawingArea* drawing_area;
  cairo_surface_t*  surface;
  
  GList* history;
  GList* future;
  
  gboolean status;
  gdouble pos_x;
  gdouble pos_y;

  gdouble size;
  SketchTool tool;
  GdkRGBA color;
  
  char * base_file;
  MarkerSourceView * source_view;
};

G_DEFINE_TYPE(MarkerSketcherWindow, marker_sketcher_window, GTK_TYPE_WINDOW)

static char noname[11] = "Untitled.md";

void clean_surface_list(GList * list)
{
  if (list->next)
  {
    clean_surface_list(list->next);
  }
  cairo_surface_destroy(list->data);
}

static void
clear_surface (cairo_surface_t * surface)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgba (cr, 1, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);
}


static gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(data);
  if (w->surface)
  {
    cairo_surface_t * destination = gdk_window_create_similar_surface (gtk_widget_get_window(widget),
                                                                        CAIRO_CONTENT_COLOR_ALPHA,
                                                                        gtk_widget_get_allocated_width (widget),
                                                                        gtk_widget_get_allocated_height (widget));
    clear_surface(destination);
    cairo_t *cr = cairo_create (destination);
    cairo_set_source_surface (cr, w->surface, 0, 0);
    cairo_paint (cr);
    cairo_surface_destroy(w->surface);
    w->surface = destination;
  }
  else{
    w->surface = gdk_window_create_similar_surface (gtk_widget_get_window(widget),
                                                    CAIRO_CONTENT_COLOR_ALPHA,
                                                    gtk_widget_get_allocated_width (widget),
                                                    gtk_widget_get_allocated_height (widget));
  
    clear_surface (w->surface);
  }
  return TRUE;
}


static gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer   data)
{
  MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(data);
  cairo_set_source_surface (cr, w->surface, 0, 0);
  cairo_paint (cr);
  return FALSE;
}


static void
draw_text(MarkerSketcherWindow *w)
{
  GtkWidget* widget = GTK_WIDGET(w->drawing_area);
  gdouble x = w->pos_x;
  gdouble y = w->pos_y;
  const gchar * text = gtk_entry_get_text(w->text_entry);
  if (!widget || !w->surface)
  {
    return;
  }
  
  cairo_t *cr;
  
  cr = cairo_create (w->surface);
  if (w->size == P_SIZE_S)
    cairo_set_font_size(cr, F_SIZE_S);
  else if (w->size == P_SIZE_M)
    cairo_set_font_size(cr, F_SIZE_M);
  else
    cairo_set_font_size(cr, F_SIZE_L);
  cairo_set_source_rgb(cr, w->color.red, w->color.green, w->color.blue);
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, text);
  cairo_destroy(cr);
  gtk_widget_queue_draw(widget);
}
          

static void
draw_brush (GtkWidget *widget,
            gdouble    x,
            gdouble    y,
            MarkerSketcherWindow * w)
{
  if (!widget || !w->surface)
  {
    return;
  }
  
  cairo_t *cr;
  cr = cairo_create (w->surface);
  if (w->tool == PEN)
    cairo_set_source_rgba(cr, w->color.red, w->color.green, w->color.blue, w->color.alpha);
  else if (w->tool == ERASER)
    cairo_set_source_rgb(cr, 1, 1, 1);
  if (!w->status)
  {

    cairo_arc(cr, x, y, w->size/2, 0, 2.0*M_PI) ;
    cairo_fill (cr);

    cairo_destroy (cr);
    gtk_widget_queue_draw(widget); //, x - size, y - size, 2*size, 2*size);
  } else
  {
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    cairo_set_line_width(cr, w->size);
    cairo_move_to(cr, w->pos_x, w->pos_y);
    cairo_line_to(cr, x, y);
    cairo_stroke(cr);
    gdouble l = x > w->pos_x ? w->pos_x : x;
    gdouble t = y > w->pos_y ? w->pos_y : y;

    cairo_destroy(cr);
    gtk_widget_queue_draw_area (widget, l - w->size, t - w->size, fabs(x-w->pos_x) + 2*w->size, fabs(y-w->pos_y) + 2*w->size);

  }
  w->status = TRUE;

  w->pos_x = x;
  w->pos_y = y;
}

static void
add_history(MarkerSketcherWindow * w)
{
  GtkWidget * widget = GTK_WIDGET(w->drawing_area);
  cairo_surface_t * destination = cairo_surface_create_similar(w->surface, 
                                                               CAIRO_CONTENT_COLOR_ALPHA,
                                                               gtk_widget_get_allocated_width (widget),
                                                               gtk_widget_get_allocated_height (widget ));
  cairo_t *cr = cairo_create (destination);
  cairo_set_source_surface (cr, w->surface, 0, 0);
  cairo_paint (cr);
  
  w->history = g_list_append(w->history, destination);
  if (w->future)
  {
    clean_surface_list(w->future);
    g_list_free(w->future);
    w->future = NULL;
  }
}

static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(data);
  /* paranoia check, in case we haven't gotten a configure event */
  if (w->surface == NULL)
    return FALSE;

  if (event->button == GDK_BUTTON_PRIMARY)
  {
    if (w->tool == PEN || w->tool == ERASER){
      
      if (!w->status)
      {
        add_history(w);
      }
      draw_brush (widget, event->x, event->y, w);
    } else {
      GdkRectangle rect;
      rect.x = event->x;
      rect.y = event->y;
      rect.width = 1;
      rect.height = 1;
      w->pos_x = event->x;
      w->pos_y = event->y;
      gtk_popover_set_pointing_to(w->text_popover, &rect);
      gtk_popover_popup(w->text_popover);
    }
  }

  return TRUE;
}

static gboolean
button_release_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       gpointer        data)
{

  if (event->button == GDK_BUTTON_PRIMARY)
  {
    MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(data);
    w->status = FALSE;
  }
  /* We've handled the event, stop processing */
  return TRUE;
}

/* Handle motion events by continuing to draw if button 1 is
 * still held down. The ::motion-notify signal handler receives
 * a GdkEventMotion struct which contains this information.
 */
static gboolean
motion_notify_event_cb (GtkWidget      *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(data);
  /* paranoia check, in case we haven't gotten a configure event */
  if (w->surface == NULL)
    return FALSE;

  if (event->state & GDK_BUTTON1_MASK)
    draw_brush (widget, event->x, event->y, w);
  gdk_window_set_cursor(gtk_widget_get_window(widget), w->cursor);
  /* We've handled it, stop processing */
  return TRUE;
}

static void
clean(MarkerSketcherWindow * window)
{
  
  if (window->surface)
    cairo_surface_destroy (window->surface);
  if (window->history)
  {
    clean_surface_list(window->history);
    g_list_free(window->history);
  }
  
  if (window->future)
  {
    clean_surface_list(window->future);
    g_list_free(window->future);
  }
  gtk_widget_hide(GTK_WIDGET(window));
  gtk_widget_destroy(GTK_WIDGET(window));
  
  window->history = NULL;
  window->future = NULL;
  window->surface = NULL;
}

static void
close_cb(GtkButton * widget,
         gpointer    user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  clean(window);
}         

static void
pen_cb(GtkButton * button,
       gpointer    user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  window->tool = PEN;
  window->cursor = gdk_cursor_new_from_name (gtk_widget_get_display(GTK_WIDGET(window)), "crosshair");
  g_object_unref(window->cursor);
}

static void
eraser_cb(GtkButton * button,
       gpointer    user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  window->tool = ERASER;
  window->cursor = gdk_cursor_new_from_name (gtk_widget_get_display(GTK_WIDGET(window)), "cell");
  g_object_unref(window->cursor);
}

static void
text_cb(GtkButton * button,
       gpointer    user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  window->tool = TEXT;
  window->cursor = gdk_cursor_new_from_name (gtk_widget_get_display(GTK_WIDGET(window)), "text");
  g_object_unref(window->cursor);
}

static void
small_cb(GtkButton* button,
         gpointer   user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  window->size = P_SIZE_S;
}

static void
medium_cb(GtkButton* button,
         gpointer   user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  window->size = P_SIZE_M;
}

static void
large_cb(GtkButton* button,
         gpointer   user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  window->size = P_SIZE_L;
}

static void
color_set_cb(GtkColorButton*  button,
             gpointer         user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(button), &window->color);
}

char *
create_unic_name(char * base)
{
  int n = strlen(base)+10;
  GRand * rand = g_rand_new();
  int uuid = g_rand_int_range(rand, 0, 9999);
  char * buffer = malloc(n*sizeof(char));
  memset(buffer, 0, n);
  sprintf(buffer, "%s.%d.png", base, uuid);
  return buffer;
}

static void
insert_sketch_cb(GtkButton *  button,
                 gpointer     user_data)
{
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(user_data);

  char * fpath = create_unic_name(window->base_file);
  cairo_surface_write_to_png(window->surface, fpath);
  marker_source_view_insert_image(window->source_view, fpath);

  clean(window);
}

static void
redo(MarkerSketcherWindow *window)
{  
  GList * last =  g_list_last(window->future);
  if (last)
  {
    window->history = g_list_append(window->history, window->surface);
    window->surface = last->data;
    gtk_widget_queue_draw(GTK_WIDGET(window->drawing_area));
    window->future = g_list_remove_link(window->future, last);
  }
}

static void
undo(MarkerSketcherWindow* window)
{
  GList * last =  g_list_last(window->history);
  if (last)
  {
    window->future = g_list_append(window->future, window->surface);
    window->surface = last->data;
    

    gtk_widget_queue_draw(GTK_WIDGET(window->drawing_area));
    
    window->history = g_list_remove_link(window->history, last);
  }
}

static gboolean
key_pressed(GtkWidget   *widget,
            GdkEventKey *event,
            gpointer     user_data)
{
  gboolean ctrl_pressed = (event->state &  GDK_CONTROL_MASK) != 0;
  MarkerSketcherWindow * window = MARKER_SKETCHER_WINDOW(widget);
  if (ctrl_pressed)
  {
    switch (event->keyval)
    {
    case GDK_KEY_z:
      undo(window);
      break;
    case GDK_KEY_Z:
      redo(window);
      break;
    }
  }
  return FALSE;
}

static void
add_text_cb(GtkWidget *button,
            gpointer  *user_data)
{
  MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(user_data);
  gtk_popover_popdown(w->text_popover);
  add_history(w);
  draw_text(w);
  gtk_entry_set_text(w->text_entry,"");
}

static void
close_text_cb(GtkButton *button,
              gpointer  *user_data)
{
  MarkerSketcherWindow * w = MARKER_SKETCHER_WINDOW(user_data);
  gtk_popover_popdown(w->text_popover);
  gtk_entry_set_text(w->text_entry,"");
}

static void
init_ui (MarkerSketcherWindow * window)
{
  GtkBuilder* builder =
  gtk_builder_new_from_resource(
    "/com/github/fabiocolacio/marker/ui/marker-sketcher-window.ui");
  
  GtkDrawingArea * drawing_area = GTK_DRAWING_AREA(gtk_drawing_area_new());
  window->drawing_area = drawing_area;

  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
 
  GtkBox * vbox = GTK_BOX(gtk_builder_get_object(builder, "vbox"));
  gtk_box_pack_start(vbox, GTK_WIDGET(drawing_area),TRUE,TRUE, 5);

  GtkHeaderBar * hbar = GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));
  
  GtkPopover * text_popover = GTK_POPOVER(gtk_builder_get_object(builder, "text_popover"));
  GtkEntry * text_entry = GTK_ENTRY(gtk_builder_get_object(builder, "text_entry"));
  window->text_entry = text_entry;
  window->text_popover = text_popover;
  gtk_popover_set_relative_to(text_popover, GTK_WIDGET(drawing_area));
  gtk_popover_set_modal (text_popover, TRUE);
  gtk_popover_set_position(text_popover, GTK_POS_LEFT);
  
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
  gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(hbar));
  gtk_widget_show (GTK_WIDGET (hbar));
  
  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area, "draw",
                      G_CALLBACK (draw_cb), window);
  g_signal_connect (drawing_area,"configure-event",
                      G_CALLBACK (configure_event_cb), window);

  /* Event signals */
  g_signal_connect (drawing_area, "motion-notify-event",
                      G_CALLBACK (motion_notify_event_cb), window);
  g_signal_connect (drawing_area, "button-press-event",
                      G_CALLBACK (button_press_event_cb), window);
  g_signal_connect (drawing_area, "button-release-event",
                      G_CALLBACK (button_release_event_cb), window);
  g_signal_connect (window, "key-press-event",
                      G_CALLBACK(key_pressed), window);
  /* Ask to receive events the drawing area doesn't normally
  * subscribe to. In particular, we need to ask for the
  * button press and motion notify events that want to handle.
  */
  gtk_widget_set_events (GTK_WIDGET(drawing_area), gtk_widget_get_events (GTK_WIDGET(drawing_area))
                                      | GDK_BUTTON_PRESS_MASK
                                      | GDK_BUTTON_RELEASE_MASK
                                      | GDK_POINTER_MOTION_MASK);

                                  
  gtk_widget_show_all(GTK_WIDGET(window));

  gtk_builder_add_callback_symbol(builder,
                                  "close_cb",
                                  G_CALLBACK(close_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "pen_cb",
                                  G_CALLBACK(pen_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "eraser_cb",
                                  G_CALLBACK(eraser_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "text_cb",
                                  G_CALLBACK(text_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "color_set_cb",
                                  G_CALLBACK(color_set_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "insert_sketch_cb",
                                  G_CALLBACK(insert_sketch_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "small_cb",
                                  G_CALLBACK(small_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "medium_cb",
                                  G_CALLBACK(medium_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "large_cb",
                                  G_CALLBACK(large_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "close_text_cb",
                                  G_CALLBACK(close_text_cb));
  gtk_builder_add_callback_symbol(builder,
                                  "add_text_cb",
                                  G_CALLBACK(add_text_cb));

  gtk_builder_connect_signals(builder, window);
  g_object_unref(builder);
}


MarkerSketcherWindow*  
marker_sketcher_window_show(GtkWindow* parent, GFile * file, MarkerSourceView * source_view)
{
  GtkApplication * app = gtk_window_get_application(parent);
  MarkerSketcherWindow * w= marker_sketcher_window_new(app);
  gtk_window_set_transient_for(GTK_WINDOW(w), parent);
  w->source_view = source_view;
  
  if (file)
    w->base_file = g_file_get_path(file);
  else
  {
    char * current_dir = g_get_current_dir ();
    int n = strlen(current_dir) + 15;
    w->base_file = malloc(n*sizeof(char));
    memset(w->base_file, 0, n);
    sprintf(w->base_file, "%s/%s", current_dir, noname);
  }
  
  gtk_window_present(GTK_WINDOW(w));
  return w;
}

MarkerSketcherWindow*  
marker_sketcher_window_new (GtkApplication * app)
{
    return g_object_new(MARKER_TYPE_SKETCHER_WINDOW, "application", app, NULL);
}


static void
marker_sketcher_window_class_init(MarkerSketcherWindowClass* class)
{
  
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
  sketcher->size = P_SIZE_M;
  
  sketcher->color.alpha = 1;
  sketcher->color.blue = 0;
  sketcher->color.green = 0;
  sketcher->color.red = 0;
  
  sketcher->cursor = gdk_cursor_new_from_name (gtk_widget_get_display(GTK_WIDGET(sketcher)), "crosshair");
  g_object_unref(sketcher->cursor);
  init_ui(sketcher);
}
