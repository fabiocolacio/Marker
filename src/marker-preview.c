/*
 * marker-preview.c
 *
 * Copyright (C) 2017-2020 Fabio Colacio
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

#include <string.h>
#include <stdlib.h>
#include <libintl.h>

#include <glib.h>
#include <time.h>

#include "marker-markdown.h"
#include "marker-prefs.h"

#include "marker-string.h"

#include "marker-preview.h"
#include "marker.h"

#define MAX_ZOOM  4.0
#define MIN_ZOOM  0.1

#define SCROLL_STEP 25
#define SCROLL_STEP_SCRIPT "window.scrollBy(%d,%d);"
#define SCROLL_SCRIPT "window.scrollTo(%d,%d);"

/* Injected at document end: keeps the preview scrolled to the editor cursor.
 * Replaces the GTK3-era web-process extension (WebKitGTK 6.0 has no DOM API). */
#define SCROLL_TO_CURSOR_SCRIPT \
  "(function(){var e=document.getElementById('cursor_pos');" \
  "if(e){e.scrollIntoView({block:'center',inline:'nearest'});}})();"

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)

struct _MarkerPreview
{
  WebKitWebView parent_instance;
};

G_DEFINE_FINAL_TYPE (MarkerPreview, marker_preview, WEBKIT_TYPE_WEB_VIEW)

static gboolean
open_uri (WebKitWebView        *web_view,
          WebKitPolicyDecision *decision)
{
  WebKitNavigationPolicyDecision *nav_dec = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
  WebKitNavigationAction *action = webkit_navigation_policy_decision_get_navigation_action (nav_dec);
  WebKitURIRequest *request = webkit_navigation_action_get_request (action);

  /* Open only http(s) requests in the default browser */
  if (webkit_uri_request_get_http_method (request) != NULL)
  {
    const gchar *uri = webkit_uri_request_get_uri (request);
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (web_view));
    g_autoptr (GtkUriLauncher) launcher = gtk_uri_launcher_new (uri);
    gtk_uri_launcher_launch (launcher, GTK_IS_WINDOW (root) ? GTK_WINDOW (root) : NULL, NULL, NULL, NULL);
    webkit_policy_decision_ignore (decision);
    return TRUE;
  }
  return FALSE;
}

static gboolean
decide_policy_cb (WebKitWebView            *web_view,
                  WebKitPolicyDecision     *decision,
                  WebKitPolicyDecisionType  type,
                  gpointer                  user_data)
{
  switch (type)
  {
    case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
    case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
      return open_uri (web_view, decision);
    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
      webkit_policy_decision_use (decision);
      return TRUE;
    default:
      /* Making no decision results in webkit_policy_decision_use(). */
      return FALSE;
  }
}

static gboolean
context_menu_cb (WebKitWebView       *web_view,
                 WebKitContextMenu   *context_menu,
                 WebKitHitTestResult *hit_test_result,
                 gpointer             user_data)
{
  /* No context menu in the preview */
  return TRUE;
}

static gboolean
key_pressed_cb (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                gpointer               user_data)
{
  MarkerPreview *preview = MARKER_PREVIEW (user_data);

  if ((state & GDK_CONTROL_MASK) != 0)
  {
    switch (keyval)
    {
      case GDK_KEY_plus:
      case GDK_KEY_KP_Add:
        marker_preview_zoom_in (preview);
        return TRUE;

      case GDK_KEY_minus:
      case GDK_KEY_KP_Subtract:
        marker_preview_zoom_out (preview);
        return TRUE;

      case GDK_KEY_0:
      case GDK_KEY_KP_0:
        marker_preview_zoom_original (preview);
        return TRUE;
    }
  }
  else
  {
    switch (keyval)
    {
      case GDK_KEY_j:
        marker_preview_scroll_down (preview);
        return TRUE;

      case GDK_KEY_k:
        marker_preview_scroll_up (preview);
        return TRUE;

      case GDK_KEY_h:
        marker_preview_scroll_left (preview);
        return TRUE;

      case GDK_KEY_l:
        marker_preview_scroll_right (preview);
        return TRUE;

      case GDK_KEY_g:
        marker_preview_scroll_to_top (preview);
        return TRUE;

      case GDK_KEY_G:
        marker_preview_scroll_to_bottom (preview);
        return TRUE;
    }
  }

  return FALSE;
}

static gboolean
scroll_cb (GtkEventControllerScroll *controller,
           gdouble                   dx,
           gdouble                   dy,
           gpointer                  user_data)
{
  MarkerPreview *preview = MARKER_PREVIEW (user_data);
  GdkModifierType state = gtk_event_controller_get_current_event_state (GTK_EVENT_CONTROLLER (controller));

  if ((state & GDK_CONTROL_MASK) != 0)
  {
    if (dy > 0)
      marker_preview_zoom_out (preview);
    else if (dy < 0)
      marker_preview_zoom_in (preview);
    return TRUE;
  }

  return FALSE;
}

static void
pdf_print_failed_cb (WebKitPrintOperation *print_op,
                     GError               *err,
                     gpointer              user_data)
{
  g_printerr ("print failed with error: %s\n", err->message);
}

static void
scroll_js_finished_cb (GObject      *object,
                       GAsyncResult *result,
                       gpointer      user_data)
{
  g_autoptr (GError) error = NULL;
  JSCValue *value = webkit_web_view_evaluate_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);

  if (error != NULL)
  {
    g_warning ("Error running scroll script: %s", error->message);
    return;
  }

  g_clear_object (&value);
}

static void
run_script (MarkerPreview *preview,
            const gchar   *script)
{
  webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW (preview), script, -1,
                                       NULL, NULL, NULL, scroll_js_finished_cb, NULL);
}

void
marker_preview_set_zoom_level (MarkerPreview *preview,
                               gdouble        zoom_level)
{
  g_return_if_fail (MARKER_IS_PREVIEW (preview));
  webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (preview), zoom_level);
  g_signal_emit_by_name (preview, "zoom-changed");
}

static void
marker_preview_init (MarkerPreview *preview)
{
  gtk_widget_set_hexpand (GTK_WIDGET (preview), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (preview), TRUE);
}

/* WebKitWebView creates its settings and user content manager in
 * constructed(), so everything touching them has to happen after chaining up. */
static void
marker_preview_constructed (GObject *object)
{
  G_OBJECT_CLASS (marker_preview_parent_class)->constructed (object);

  MarkerPreview *preview = MARKER_PREVIEW (object);
  WebKitWebView *web_view = WEBKIT_WEB_VIEW (preview);

  /* Local KaTeX / highlight.js / mermaid live under file://SCRIPTS_DIR */
  WebKitSettings *settings = webkit_web_view_get_settings (web_view);
  webkit_settings_set_allow_file_access_from_file_urls (settings, TRUE);
  webkit_settings_set_allow_universal_access_from_file_urls (settings, TRUE);

  /* Scroll-to-cursor user script */
  WebKitUserContentManager *ucm = webkit_web_view_get_user_content_manager (web_view);
  WebKitUserScript *script = webkit_user_script_new (SCROLL_TO_CURSOR_SCRIPT,
                                                     WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                                                     WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END,
                                                     NULL, NULL);
  webkit_user_content_manager_add_script (ucm, script);
  webkit_user_script_unref (script);

  g_signal_connect (web_view, "decide-policy", G_CALLBACK (decide_policy_cb), NULL);
  g_signal_connect (web_view, "context-menu", G_CALLBACK (context_menu_cb), NULL);

  GtkEventController *key = gtk_event_controller_key_new ();
  gtk_event_controller_set_propagation_phase (key, GTK_PHASE_CAPTURE);
  g_signal_connect (key, "key-pressed", G_CALLBACK (key_pressed_cb), preview);
  gtk_widget_add_controller (GTK_WIDGET (preview), key);

  GtkEventController *scroll = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
  gtk_event_controller_set_propagation_phase (scroll, GTK_PHASE_CAPTURE);
  g_signal_connect (scroll, "scroll", G_CALLBACK (scroll_cb), preview);
  gtk_widget_add_controller (GTK_WIDGET (preview), scroll);
}

static void
marker_preview_class_init (MarkerPreviewClass *class)
{
  G_OBJECT_CLASS (class)->constructed = marker_preview_constructed;

  g_signal_newv ("zoom-changed",
                 G_TYPE_FROM_CLASS (class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                 NULL, NULL, NULL, NULL,
                 G_TYPE_NONE, 0, NULL);
}

MarkerPreview *
marker_preview_new (void)
{
  MarkerPreview *obj = g_object_new (MARKER_TYPE_PREVIEW, NULL);
  webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (obj), makrer_prefs_get_zoom_level ());
  return obj;
}

void
marker_preview_zoom_out (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);

  gdouble val = webkit_web_view_get_zoom_level (view) - 0.1;
  val = max (val, MIN_ZOOM);

  marker_prefs_set_zoom_level (val);
  webkit_web_view_set_zoom_level (view, val);

  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_zoom_original (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);

  gdouble zoom = 1.0;

  marker_prefs_set_zoom_level (zoom);
  webkit_web_view_set_zoom_level (view, zoom);

  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_zoom_in (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);

  gdouble val = webkit_web_view_get_zoom_level (view) + 0.1;
  val = min (val, MAX_ZOOM);

  marker_prefs_set_zoom_level (val);
  webkit_web_view_set_zoom_level (view, val);

  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_render_markdown (MarkerPreview *preview,
                                const char    *markdown,
                                const char    *css_theme,
                                const char    *base_uri,
                                int            cursor)
{
  MarkerMathJSMode katex_mode = MATHJS_OFF;
  if (marker_prefs_get_use_mathjs ())
    katex_mode = MATHJS_LOCAL;

  MarkerHighlightMode highlight_mode = HIGHLIGHT_OFF;
  if (marker_prefs_get_use_highlight ())
    highlight_mode = HIGHLIGHT_LOCAL;

  MarkerMermaidMode mermaid_mode = MERMAID_OFF;
  if (marker_prefs_get_use_mermaid ())
    mermaid_mode = MERMAID_LOCAL;

  g_autofree char *base_folder = NULL;
  if (base_uri)
    base_folder = marker_string_filename_get_path (base_uri);

  char *html = marker_markdown_to_html (markdown,
                                        strlen (markdown),
                                        base_folder,
                                        katex_mode,
                                        highlight_mode,
                                        mermaid_mode,
                                        css_theme,
                                        cursor);

  g_autofree gchar *uri = NULL;
  if (base_uri)
    uri = g_filename_to_uri (base_uri, NULL, NULL);
  if (!uri)
    uri = g_filename_to_uri (g_get_home_dir (), NULL, NULL);

  webkit_web_view_load_html (WEBKIT_WEB_VIEW (preview), html, uri);

  free (html);
}

WebKitPrintOperationResponse
marker_preview_run_print_dialog (MarkerPreview *preview,
                                 GtkWindow     *parent)
{
  g_autoptr (WebKitPrintOperation) print_op = webkit_print_operation_new (WEBKIT_WEB_VIEW (preview));
  g_signal_connect (print_op, "failed", G_CALLBACK (pdf_print_failed_cb), NULL);
  return webkit_print_operation_run_dialog (print_op, parent);
}

void
marker_preview_print_pdf (MarkerPreview           *preview,
                          const char              *outfile,
                          enum scidown_paper_size  paper_size,
                          GtkPageOrientation       orientation)
{
  g_autoptr (WebKitPrintOperation) print_op = NULL;
  g_autoptr (GtkPrintSettings) print_s = NULL;
  g_autofree gchar *uri = g_filename_to_uri (outfile, NULL, NULL);

  print_op = webkit_print_operation_new (WEBKIT_WEB_VIEW (preview));
  g_signal_connect (print_op, "failed", G_CALLBACK (pdf_print_failed_cb), NULL);

  print_s = gtk_print_settings_new ();
  GtkPaperSize *gtk_paper_size = NULL;
  if (paper_size != B43 && paper_size != B169)
    gtk_paper_size = gtk_paper_size_new (paper_to_gtkstr (paper_size));
  else if (paper_size == B43)
    gtk_paper_size = gtk_paper_size_new_custom ("B43", "B43", 166, 221, GTK_UNIT_MM);
  else
    gtk_paper_size = gtk_paper_size_new_custom ("B43", "B43", 166, 294, GTK_UNIT_MM);

  g_autoptr (GtkPageSetup) gtk_page_setup = gtk_page_setup_new ();

  gtk_print_settings_set (print_s, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
  gtk_print_settings_set (print_s, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);
  gtk_print_settings_set (print_s, GTK_PRINT_SETTINGS_PRINTER, dgettext ("gtk40", "Print to File"));

  if (orientation == GTK_PAGE_ORIENTATION_PORTRAIT)
  {
    gtk_page_setup_set_paper_size (gtk_page_setup, gtk_paper_size);
    gtk_print_settings_set_paper_width (print_s, gtk_paper_size_get_width (gtk_paper_size, GTK_UNIT_MM), GTK_UNIT_MM);
    gtk_print_settings_set_paper_height (print_s, gtk_paper_size_get_height (gtk_paper_size, GTK_UNIT_MM), GTK_UNIT_MM);
  }
  else
  {
    gdouble width = gtk_paper_size_get_width (gtk_paper_size, GTK_UNIT_MM);
    gdouble height = gtk_paper_size_get_height (gtk_paper_size, GTK_UNIT_MM);
    g_autofree gchar *name = g_strdup_printf ("%s_landscape", paper_to_string (paper_size));
    GtkPaperSize *custom_size = gtk_paper_size_new_custom (name, "pdf", height, width, GTK_UNIT_MM);
    gtk_page_setup_set_paper_size (gtk_page_setup, custom_size);
    gtk_paper_size_free (custom_size);

    gtk_print_settings_set_paper_width (print_s, height, GTK_UNIT_MM);
    gtk_print_settings_set_paper_height (print_s, width, GTK_UNIT_MM);
  }
  if (paper_size == B43 || paper_size == B169)
  {
    gtk_page_setup_set_left_margin (gtk_page_setup, 0, GTK_UNIT_POINTS);
    gtk_page_setup_set_right_margin (gtk_page_setup, 0, GTK_UNIT_POINTS);
    gtk_page_setup_set_top_margin (gtk_page_setup, 0, GTK_UNIT_POINTS);
    gtk_page_setup_set_bottom_margin (gtk_page_setup, 0, GTK_UNIT_POINTS);
  }
  gtk_print_settings_set_orientation (print_s, orientation);

  webkit_print_operation_set_print_settings (print_op, print_s);
  webkit_print_operation_set_page_setup (print_op, gtk_page_setup);

  webkit_print_operation_print (print_op);

  gtk_paper_size_free (gtk_paper_size);
}

void
marker_preview_scroll_left (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, -SCROLL_STEP, 0);
  run_script (preview, script);
}

void
marker_preview_scroll_right (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, SCROLL_STEP, 0);
  run_script (preview, script);
}

void
marker_preview_scroll_up (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, 0, -SCROLL_STEP);
  run_script (preview, script);
}

void
marker_preview_scroll_down (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, 0, SCROLL_STEP);
  run_script (preview, script);
}

void
marker_preview_scroll_to_top (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_SCRIPT, 0, 0);
  run_script (preview, script);
}

void
marker_preview_scroll_to_bottom (MarkerPreview *preview)
{
  run_script (preview, "window.scrollTo(0,document.body.scrollHeight);");
}
