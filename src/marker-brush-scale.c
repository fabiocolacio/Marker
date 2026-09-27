/*
 * marker-brush-scale.c
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

#include "marker-brush-scale.h"

#include <math.h>
#include <glib/gi18n.h>

/* Geometry, in logical pixels */
#define KNOB_RING        8.0   /* ring added around the dot so tiny brushes stay grabbable */
#define KNOB_MIN_DOT     6.0   /* the ring never shrinks below this dot size */
#define KNOB_MAX         (MARKER_BRUSH_SCALE_MAX + KNOB_RING)
#define WIDGET_HEIGHT    ((int) KNOB_MAX + 2)
#define WIDGET_MIN_WIDTH 120
#define WIDGET_NAT_WIDTH 168
#define TROUGH_THIN      1.0   /* half-height of the wedge at the minimum */
#define TROUGH_THICK     4.0   /* half-height of the wedge at the maximum */
#define STEP_SMALL       1.0
#define STEP_LARGE       5.0

struct _MarkerBrushScale
{
  GtkWidget  parent_instance;

  GtkWidget *knob;
  gdouble    value;
  GdkRGBA    color;
};

static gdouble  knob_diameter   (MarkerBrushScale *self);
static gdouble  value_to_x      (MarkerBrushScale *self);
static gdouble  x_to_value      (MarkerBrushScale *self,
                                 gdouble           x);
static void     update_tooltip  (MarkerBrushScale *self);
static void     step            (MarkerBrushScale *self,
                                 gdouble           delta);

G_DEFINE_FINAL_TYPE (MarkerBrushScale, marker_brush_scale, GTK_TYPE_WIDGET)

enum
{
  PROP_0,
  PROP_VALUE,
  PROP_COLOR,
  N_PROPS
};

static GParamSpec *props[N_PROPS];

static const gchar css[] =
  "brushscale { border-radius: 9999px; outline-offset: -2px; }"
  "brushscale > .knob {"
  "  background-color: @window_bg_color;"
  "  border: 1px solid alpha(currentColor, 0.28);"
  "  border-radius: 9999px;"
  "  box-shadow: 0 1px 2px alpha(black, 0.18);"
  "}"
  "brushscale > .knob:hover { background-color: mix(@window_bg_color, currentColor, 0.06); }"
  "brushscale > .knob:active { background-color: mix(@window_bg_color, currentColor, 0.12); }"
  "brushscale:disabled > .knob { box-shadow: none; }";

/* ---- Geometry helpers -------------------------------------------------- */

static gdouble
knob_diameter (MarkerBrushScale *self)
{
  return MAX (self->value, KNOB_MIN_DOT) + KNOB_RING;
}

static void
track_extent (GtkWidget *widget,
              gdouble   *x0,
              gdouble   *x1)
{
  const gdouble pad = KNOB_MAX / 2.0 + 1.0;
  *x0 = pad;
  *x1 = gtk_widget_get_width (widget) - pad;
}

static gdouble
value_to_x (MarkerBrushScale *self)
{
  gdouble x0, x1;
  track_extent (GTK_WIDGET (self), &x0, &x1);
  gdouble t = (self->value - MARKER_BRUSH_SCALE_MIN) / (MARKER_BRUSH_SCALE_MAX - MARKER_BRUSH_SCALE_MIN);
  return x0 + t * (x1 - x0);
}

static gdouble
x_to_value (MarkerBrushScale *self,
            gdouble           x)
{
  gdouble x0, x1;
  track_extent (GTK_WIDGET (self), &x0, &x1);
  gdouble t = CLAMP ((x - x0) / (x1 - x0), 0.0, 1.0);
  return MARKER_BRUSH_SCALE_MIN + t * (MARKER_BRUSH_SCALE_MAX - MARKER_BRUSH_SCALE_MIN);
}

/* ---- Value helpers ----------------------------------------------------- */

static void
update_tooltip (MarkerBrushScale *self)
{
  g_autofree gchar *text = g_strdup_printf (_("Brush size: %d px"), (int) self->value);
  gtk_widget_set_tooltip_text (GTK_WIDGET (self), text);
  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_VALUE_NOW, self->value,
                                  GTK_ACCESSIBLE_PROPERTY_VALUE_TEXT, text,
                                  -1);
}

static void
step (MarkerBrushScale *self,
      gdouble           delta)
{
  marker_brush_scale_set_value (self, self->value + delta);
}

/* ---- Input ------------------------------------------------------------- */

static void
drag_begin_cb (GtkGestureDrag *gesture,
               gdouble         x,
               gdouble         y,
               gpointer        data)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (data);
  gtk_widget_grab_focus (GTK_WIDGET (self));
  gtk_widget_set_state_flags (self->knob, GTK_STATE_FLAG_ACTIVE, FALSE);
  marker_brush_scale_set_value (self, x_to_value (self, x));
}

static void
drag_update_cb (GtkGestureDrag *gesture,
                gdouble         offset_x,
                gdouble         offset_y,
                gpointer        data)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (data);
  gdouble start_x, start_y;
  gtk_gesture_drag_get_start_point (gesture, &start_x, &start_y);
  marker_brush_scale_set_value (self, x_to_value (self, start_x + offset_x));
}

static void
drag_end_cb (GtkGestureDrag *gesture,
             gdouble         offset_x,
             gdouble         offset_y,
             gpointer        data)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (data);
  gtk_widget_unset_state_flags (self->knob, GTK_STATE_FLAG_ACTIVE);
}

static gboolean
scrolled_cb (GtkEventControllerScroll *controller,
           gdouble                   dx,
           gdouble                   dy,
           gpointer                  data)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (data);
  gdouble delta = fabs (dx) > fabs (dy) ? dx : -dy;

  if (delta == 0)
    return GDK_EVENT_PROPAGATE;

  step (self, delta > 0 ? STEP_SMALL : -STEP_SMALL);
  return GDK_EVENT_STOP;
}

static gboolean
key_pressed_cb (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                gpointer               data)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (data);
  gboolean rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  switch (keyval)
  {
    case GDK_KEY_Left:
    case GDK_KEY_KP_Left:
      step (self, rtl ? STEP_SMALL : -STEP_SMALL);
      return GDK_EVENT_STOP;
    case GDK_KEY_Right:
    case GDK_KEY_KP_Right:
      step (self, rtl ? -STEP_SMALL : STEP_SMALL);
      return GDK_EVENT_STOP;
    case GDK_KEY_Down:
    case GDK_KEY_KP_Down:
    case GDK_KEY_minus:
    case GDK_KEY_KP_Subtract:
      step (self, -STEP_SMALL);
      return GDK_EVENT_STOP;
    case GDK_KEY_Up:
    case GDK_KEY_KP_Up:
    case GDK_KEY_plus:
    case GDK_KEY_equal:
    case GDK_KEY_KP_Add:
      step (self, STEP_SMALL);
      return GDK_EVENT_STOP;
    case GDK_KEY_Page_Down:
    case GDK_KEY_KP_Page_Down:
      step (self, -STEP_LARGE);
      return GDK_EVENT_STOP;
    case GDK_KEY_Page_Up:
    case GDK_KEY_KP_Page_Up:
      step (self, STEP_LARGE);
      return GDK_EVENT_STOP;
    case GDK_KEY_Home:
    case GDK_KEY_KP_Home:
      marker_brush_scale_set_value (self, MARKER_BRUSH_SCALE_MIN);
      return GDK_EVENT_STOP;
    case GDK_KEY_End:
    case GDK_KEY_KP_End:
      marker_brush_scale_set_value (self, MARKER_BRUSH_SCALE_MAX);
      return GDK_EVENT_STOP;
  }
  return GDK_EVENT_PROPAGATE;
}

/* ---- Layout & drawing -------------------------------------------------- */

static void
measure (GtkWidget      *widget,
         GtkOrientation  orientation,
         int             for_size,
         int            *minimum,
         int            *natural,
         int            *minimum_baseline,
         int            *natural_baseline)
{
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
  {
    *minimum = WIDGET_MIN_WIDTH;
    *natural = WIDGET_NAT_WIDTH;
  }
  else
  {
    *minimum = *natural = WIDGET_HEIGHT;
  }
}

static void
size_allocate (GtkWidget *widget,
               int        width,
               int        height,
               int        baseline)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (widget);
  const gint d = (gint) ceil (knob_diameter (self));
  const gint cx = (gint) round (value_to_x (self));
  const gint cy = height / 2;
  GtkAllocation alloc = { cx - d / 2, cy - d / 2, d, d };

  gtk_widget_size_allocate (self->knob, &alloc, -1);
}

static void
wedge_path (cairo_t *cr,
            gdouble  x0,
            gdouble  x1,
            gdouble  cy)
{
  cairo_new_path (cr);
  cairo_arc_negative (cr, x0, cy, TROUGH_THIN, M_PI / 2, -M_PI / 2);
  cairo_line_to (cr, x1, cy - TROUGH_THICK);
  cairo_arc (cr, x1, cy, TROUGH_THICK, -M_PI / 2, M_PI / 2);
  cairo_close_path (cr);
}

static void
snapshot (GtkWidget   *widget,
          GtkSnapshot *snapshot)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (widget);
  const gdouble width = gtk_widget_get_width (widget);
  const gdouble height = gtk_widget_get_height (widget);
  const gdouble cy = height / 2.0;
  const gdouble knob_x = value_to_x (self);
  gdouble x0, x1;
  GdkRGBA fg;

  track_extent (widget, &x0, &x1);
  gtk_widget_get_color (widget, &fg);

  /* Trough: a wedge that widens towards the maximum. The part behind the knob
   * is painted in the brush colour, the rest in a faint foreground tint. */
  cairo_t *cr = gtk_snapshot_append_cairo (snapshot, &GRAPHENE_RECT_INIT (0, 0, width, height));

  wedge_path (cr, x0, x1, cy);
  cairo_set_source_rgba (cr, fg.red, fg.green, fg.blue, fg.alpha * 0.18);
  cairo_fill (cr);

  cairo_save (cr);
  cairo_rectangle (cr, 0, 0, knob_x, height);
  cairo_clip (cr);
  wedge_path (cr, x0, x1, cy);
  cairo_set_source_rgba (cr, fg.red, fg.green, fg.blue, fg.alpha * 0.18);
  cairo_fill_preserve (cr);
  gdk_cairo_set_source_rgba (cr, &self->color);
  cairo_fill (cr);
  cairo_restore (cr);

  cairo_destroy (cr);

  /* Handle: a themed ring, with the brush stroke previewed at true size. */
  gtk_widget_snapshot_child (widget, self->knob, snapshot);

  cr = gtk_snapshot_append_cairo (snapshot, &GRAPHENE_RECT_INIT (0, 0, width, height));
  cairo_arc (cr, knob_x, cy, self->value / 2.0, 0, 2 * M_PI);
  gdk_cairo_set_source_rgba (cr, &self->color);
  cairo_fill (cr);
  cairo_destroy (cr);
}

/* ---- Parent class overrides -------------------------------------------- */

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (object);

  switch (prop_id)
  {
    case PROP_VALUE:
      g_value_set_double (value, self->value);
      break;
    case PROP_COLOR:
      g_value_set_boxed (value, &self->color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (object);

  switch (prop_id)
  {
    case PROP_VALUE:
      marker_brush_scale_set_value (self, g_value_get_double (value));
      break;
    case PROP_COLOR:
      marker_brush_scale_set_color (self, g_value_get_boxed (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
dispose (GObject *object)
{
  MarkerBrushScale *self = MARKER_BRUSH_SCALE (object);
  g_clear_pointer (&self->knob, gtk_widget_unparent);
  G_OBJECT_CLASS (marker_brush_scale_parent_class)->dispose (object);
}

static void
marker_brush_scale_class_init (MarkerBrushScaleClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  object_class->get_property = get_property;
  object_class->set_property = set_property;
  object_class->dispose = dispose;

  widget_class->measure = measure;
  widget_class->size_allocate = size_allocate;
  widget_class->snapshot = snapshot;

  props[PROP_VALUE] =
    g_param_spec_double ("value", NULL, NULL,
                         MARKER_BRUSH_SCALE_MIN, MARKER_BRUSH_SCALE_MAX, 6.0,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
  props[PROP_COLOR] =
    g_param_spec_boxed ("color", NULL, NULL, GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (object_class, N_PROPS, props);

  gtk_widget_class_set_css_name (widget_class, "brushscale");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_SLIDER);

  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (provider, css);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);
}

static void
marker_brush_scale_init (MarkerBrushScale *self)
{
  self->value = 6.0;
  self->color = (GdkRGBA) { 0, 0, 0, 1 };

  gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);
  gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_CENTER);
  gtk_widget_set_cursor_from_name (GTK_WIDGET (self), "pointer");

  self->knob = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_add_css_class (self->knob, "knob");
  gtk_widget_set_parent (self->knob, GTK_WIDGET (self));

  GtkGesture *drag = gtk_gesture_drag_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
  g_signal_connect (drag, "drag-begin", G_CALLBACK (drag_begin_cb), self);
  g_signal_connect (drag, "drag-update", G_CALLBACK (drag_update_cb), self);
  g_signal_connect (drag, "drag-end", G_CALLBACK (drag_end_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (drag));

  GtkEventController *scroll =
    gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES | GTK_EVENT_CONTROLLER_SCROLL_DISCRETE);
  g_signal_connect (scroll, "scroll", G_CALLBACK (scrolled_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), scroll);

  GtkEventController *key = gtk_event_controller_key_new ();
  g_signal_connect (key, "key-pressed", G_CALLBACK (key_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), key);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_VALUE_MIN, MARKER_BRUSH_SCALE_MIN,
                                  GTK_ACCESSIBLE_PROPERTY_VALUE_MAX, MARKER_BRUSH_SCALE_MAX,
                                  -1);
  update_tooltip (self);
}

/* ---- Public API -------------------------------------------------------- */

void
marker_brush_scale_set_value (MarkerBrushScale *self,
                              gdouble           value)
{
  g_return_if_fail (MARKER_IS_BRUSH_SCALE (self));

  value = CLAMP (round (value), MARKER_BRUSH_SCALE_MIN, MARKER_BRUSH_SCALE_MAX);
  if (value == self->value)
    return;

  self->value = value;
  update_tooltip (self);
  gtk_widget_queue_allocate (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

gdouble
marker_brush_scale_get_value (MarkerBrushScale *self)
{
  g_return_val_if_fail (MARKER_IS_BRUSH_SCALE (self), MARKER_BRUSH_SCALE_MIN);
  return self->value;
}

void
marker_brush_scale_set_color (MarkerBrushScale *self,
                              const GdkRGBA    *color)
{
  g_return_if_fail (MARKER_IS_BRUSH_SCALE (self));
  g_return_if_fail (color != NULL);

  if (gdk_rgba_equal (&self->color, color))
    return;

  self->color = *color;
  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR]);
}

const GdkRGBA *
marker_brush_scale_get_color (MarkerBrushScale *self)
{
  g_return_val_if_fail (MARKER_IS_BRUSH_SCALE (self), NULL);
  return &self->color;
}

GtkWidget *
marker_brush_scale_new (void)
{
  return g_object_new (MARKER_TYPE_BRUSH_SCALE, NULL);
}
