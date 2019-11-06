/*
 * marker-preview.c
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

#include <string.h>
#include <stdlib.h>

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

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)

struct _MarkerPreview
{
  WebKitWebView parent_instance;
};

G_DEFINE_TYPE(MarkerPreview, marker_preview, WEBKIT_TYPE_WEB_VIEW)

static gboolean
open_uri (WebKitPolicyDecision *decision) {
  WebKitNavigationPolicyDecision *nav_dec = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
  WebKitNavigationAction *action = webkit_navigation_policy_decision_get_navigation_action (nav_dec);
  WebKitURIRequest *request = webkit_navigation_action_get_request (action);
  /* Open only http requests in default browser */
  /* FIXME: Open also other request like ftp in default browser */
  if (webkit_uri_request_get_http_method(request) != NULL) {
    const gchar * uri = webkit_uri_request_get_uri(request);
    GtkApplication * app = marker_get_app();
    GList* windows = gtk_application_get_windows(app);
    time_t now = time(0);
    gtk_show_uri_on_window (windows->data, uri, now, NULL);
    webkit_policy_decision_ignore(decision);
    return TRUE;
  }
  return FALSE;
}

static gboolean
navigate(WebKitPolicyDecision *decision)
{
  /** TODO FIX internal navigation
  WebKitNavigationPolicyDecision * nav_dec = WEBKIT_NAVIGATION_POLICY_DECISION(decision);

  const gchar * uri =webkit_uri_request_get_uri(webkit_navigation_action_get_request (webkit_navigation_policy_decision_get_navigation_action (nav_dec)));
  g_print(">> %s\n", uri);
  **/

  /* if request is http ignore default policy and open uri in default browser*/
  return open_uri (decision);
}

static gboolean
decide_policy_cb (WebKitWebView *web_view,
                  WebKitPolicyDecision *decision,
                  WebKitPolicyDecisionType type)
{
    switch (type) {
    case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
      return navigate(decision);
    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
      webkit_policy_decision_use (decision);
      break;
    case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
      return navigate(decision);
    default:  
      /* Making no decision results in webkit_policy_decision_use(). */
      return FALSE;
    }
    return TRUE;
}


static gboolean
context_menu_cb  (WebKitWebView       *web_view,
                  WebKitContextMenu   *context_menu,
                  GdkEvent            *event,
                  WebKitHitTestResult *hit_test_result,
                  gpointer             user_data)
{
  return TRUE;
}

static void
initialize_web_extensions_cb (WebKitWebContext *context,
                              gpointer          user_data)
{
  /* Web Extensions get a different ID for each Web Process */
  static guint32 unique_id = 0;

  webkit_web_context_set_web_extensions_directory (
     context, WEB_EXTENSIONS_DIRECTORY);
  webkit_web_context_set_web_extensions_initialization_user_data (
     context, g_variant_new_uint32 (unique_id++));
}

gboolean
key_press_event_cb (GtkWidget *widget,
                    GdkEvent  *event,
                    gpointer   user_data)
{
  g_return_val_if_fail (MARKER_IS_PREVIEW (widget), FALSE);
  MarkerPreview *preview = MARKER_PREVIEW (widget);

  GdkEventKey *key_event = (GdkEventKey *) event;

  if ((key_event->state & GDK_CONTROL_MASK) != 0)
  {
    switch (key_event->keyval)
    {
      case GDK_KEY_plus:
        marker_preview_zoom_in (preview);
        break;

      case GDK_KEY_minus:
        marker_preview_zoom_out (preview);
        break;

      case GDK_KEY_0:
        marker_preview_zoom_original (preview);
        break;
    }
  }
  else
  {
    switch (key_event->keyval)
    {
      case GDK_KEY_j:
        marker_preview_scroll_down (preview);
        break;

      case GDK_KEY_k:
        marker_preview_scroll_up (preview);
        break;

      case GDK_KEY_h:
        marker_preview_scroll_left (preview);
        break;

      case GDK_KEY_l:
        marker_preview_scroll_right (preview);
        break;

      case GDK_KEY_g:
        marker_preview_scroll_to_top (preview);
        break;

      case GDK_KEY_G:
        marker_preview_scroll_to_bottom (preview);
        break;
    }
  }

  return FALSE;
}

gboolean
scroll_event_cb (GtkWidget *widget,
                 GdkEvent  *event,
                 gpointer   user_data)
{
  g_return_val_if_fail (MARKER_IS_PREVIEW (widget), FALSE);
  MarkerPreview *preview = MARKER_PREVIEW (widget);

  GdkEventScroll *scroll_event = (GdkEventScroll *) event;
  guint state = scroll_event->state;
  if ((state & GDK_CONTROL_MASK) != 0)
  {
    gdouble delta_y = scroll_event->delta_y;

    if (delta_y > 0)
    {
      marker_preview_zoom_out (preview);
    }
    else if (delta_y < 0)
    {
      marker_preview_zoom_in (preview);
    }
  }

  return FALSE;
}

static void
load_changed_cb (WebKitWebView   *preview,
                 WebKitLoadEvent  event)
{
  switch (event)
  {
    case WEBKIT_LOAD_STARTED:
      break;

    case WEBKIT_LOAD_REDIRECTED:
      break;

    case WEBKIT_LOAD_COMMITTED:
      break;

    case WEBKIT_LOAD_FINISHED:
      break;
  }
}

static void
pdf_print_failed_cb (WebKitPrintOperation* print_op,
                     GError*               err,
                     gpointer              user_data)
{
  g_printerr("print failed with error: %s\n", err->message);
}

static void
scroll_js_finished_cb (GObject      *object,
                       GAsyncResult *result,
                       gpointer      user_data)
{
  WebKitJavascriptResult *js_result;
  GError *error = NULL;

  js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
  if (error != NULL) {
    g_print ("Error running scroll script: %s", error->message);
    g_error_free (error);
    return;
  } 

  webkit_javascript_result_unref (js_result);
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
  g_signal_connect (webkit_web_context_get_default (),
                    "initialize-web-extensions",
                    G_CALLBACK (initialize_web_extensions_cb),
                    NULL);

  g_signal_connect (preview, "scroll-event", G_CALLBACK (scroll_event_cb), NULL);
  g_signal_connect (preview, "key-press-event", G_CALLBACK (key_press_event_cb), NULL);
}

static void
marker_preview_class_init (MarkerPreviewClass *class)
{
  g_signal_newv ("zoom-changed",
                 G_TYPE_FROM_CLASS (class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                 NULL, NULL, NULL, NULL,
                 G_TYPE_NONE, 0, NULL);


  WEBKIT_WEB_VIEW_CLASS(class)->load_changed = load_changed_cb;
}

MarkerPreview*
marker_preview_new(void)
{
  MarkerPreview * obj =  g_object_new(MARKER_TYPE_PREVIEW, NULL);
  webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (obj), makrer_prefs_get_zoom_level ());
  

  /***
  WebKitSettings * settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(obj));
  webkit_settings_set_enable_write_console_messages_to_stdout(settings, TRUE);
  webkit_web_view_set_settings(WEBKIT_WEB_VIEW(obj), settings);
  ***/

  return obj;
}

void
marker_preview_zoom_out (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);

  gdouble val = webkit_web_view_get_zoom_level (view) - 0.1;
  val = max (val, MIN_ZOOM);

  marker_prefs_set_zoom_level(val);
  webkit_web_view_set_zoom_level(view, val);

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

  marker_prefs_set_zoom_level(val);
  webkit_web_view_set_zoom_level(view, val);

  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_render_markdown(MarkerPreview* preview,
                               const char*    markdown,
                               const char*    css_theme,
                               const char*    base_uri,
                               int            cursor)
{
  MarkerMathJSMode katex_mode = MATHJS_OFF;
  if (marker_prefs_get_use_mathjs()) {
    katex_mode = MATHJS_LOCAL;
  }
  MarkerHighlightMode highlight_mode = HIGHLIGHT_OFF;
  if (marker_prefs_get_use_highlight()){
    highlight_mode = HIGHLIGHT_LOCAL;
  }
  MarkerMermaidMode mermaid_mode = MERMAID_OFF;
  if (marker_prefs_get_use_mermaid())
  {
    mermaid_mode = MERMAID_LOCAL;
  }

  char * base_folder = NULL;
  if (base_uri)
    base_folder = marker_string_filename_get_path(base_uri);
  char* html = marker_markdown_to_html(markdown,
                                       strlen(markdown),
                                       base_folder,
                                       katex_mode,
                                       highlight_mode,
                                       mermaid_mode,
                                       css_theme,
                                       cursor);

  WebKitWebView* web_view = WEBKIT_WEB_VIEW(preview);

  g_signal_connect(web_view,
                   "decide-policy",
                   G_CALLBACK(decide_policy_cb),
                   NULL);
  g_signal_connect(web_view,
                   "context-menu",
                   G_CALLBACK(context_menu_cb),
                   NULL);

  gchar * uri;
  if (base_uri) {
    uri = g_filename_to_uri  (g_locale_from_utf8(base_uri, strlen(base_uri), NULL, NULL, NULL), NULL, NULL);
  }else {
    uri = g_strdup("file:///internal.html");
  }
  webkit_web_view_load_html(web_view 
                            ,html
                            ,uri);
  g_free(uri);
  free(html);
}

WebKitPrintOperationResponse
marker_preview_run_print_dialog(MarkerPreview* preview,
                                GtkWindow*     parent)
{
  WebKitPrintOperation* print_op =
    webkit_print_operation_new(WEBKIT_WEB_VIEW(preview));

  g_signal_connect(print_op, "failed", G_CALLBACK(pdf_print_failed_cb), NULL);

  return webkit_print_operation_run_dialog(print_op, parent);
}

void
marker_preview_print_pdf(MarkerPreview*     preview,
                         const char*        outfile,
                         enum scidown_paper_size paper_size,
                         GtkPageOrientation orientation)

{
    WebKitPrintOperation* print_op = NULL;
    GtkPrintSettings* print_s = NULL;
    char* uri = g_strdup_printf("file://%s", outfile);

    print_op = webkit_print_operation_new(WEBKIT_WEB_VIEW(preview));
    g_signal_connect(print_op, "failed", G_CALLBACK(pdf_print_failed_cb), NULL);

    print_s = gtk_print_settings_new();
    GtkPaperSize * gtk_paper_size = NULL;
    if (paper_size != B43 && paper_size != B169)
      gtk_paper_size = gtk_paper_size_new(paper_to_gtkstr(paper_size));
    else if (paper_size == B43)
      gtk_paper_size = gtk_paper_size_new_custom("B43", "B43", 166, 221, GTK_UNIT_MM);
    else
      gtk_paper_size = gtk_paper_size_new_custom("B43", "B43", 166, 294, GTK_UNIT_MM);

    GtkPageSetup * gtk_page_setup = gtk_page_setup_new();

    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_PRINTER, "Print to File");

    if (orientation == GTK_PAGE_ORIENTATION_PORTRAIT) {
      gtk_page_setup_set_paper_size(gtk_page_setup, gtk_paper_size);
      gtk_print_settings_set_paper_width(print_s, gtk_paper_size_get_width(gtk_paper_size, GTK_UNIT_MM), GTK_UNIT_MM);
      gtk_print_settings_set_paper_height(print_s, gtk_paper_size_get_height(gtk_paper_size, GTK_UNIT_MM), GTK_UNIT_MM);

    } else {
      gdouble width = gtk_paper_size_get_width(gtk_paper_size, GTK_UNIT_MM);
      gdouble height = gtk_paper_size_get_height(gtk_paper_size, GTK_UNIT_MM);
      GtkPaperSize * custom_size = gtk_paper_size_new_custom(g_strdup_printf("%s_landscape", paper_to_string(paper_size)),
                                                             "pdf", height, width, GTK_UNIT_MM);
      gtk_page_setup_set_paper_size(gtk_page_setup, custom_size);

      gtk_print_settings_set_paper_width(print_s, height, GTK_UNIT_MM);
      gtk_print_settings_set_paper_height(print_s, width, GTK_UNIT_MM);
    }
    if (paper_size == B43 || paper_size == B169) {
      gtk_page_setup_set_left_margin(gtk_page_setup, 0, GTK_UNIT_POINTS);
      gtk_page_setup_set_right_margin(gtk_page_setup, 0, GTK_UNIT_POINTS);
      gtk_page_setup_set_top_margin(gtk_page_setup, 0, GTK_UNIT_POINTS);
      gtk_page_setup_set_bottom_margin(gtk_page_setup, 0, GTK_UNIT_POINTS);
    }
    gtk_print_settings_set_orientation(print_s, orientation);

    webkit_print_operation_set_print_settings(print_op, print_s);
    webkit_print_operation_set_page_setup(print_op, gtk_page_setup);

    webkit_print_operation_print(print_op);

    g_free(uri);
}

void
marker_preview_scroll_left (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, -SCROLL_STEP, 0);
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (preview), script, NULL, scroll_js_finished_cb, NULL);
}

void
marker_preview_scroll_right (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, SCROLL_STEP, 0);
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (preview), script, NULL, scroll_js_finished_cb, NULL);
}

void
marker_preview_scroll_up (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, 0, -SCROLL_STEP);
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (preview), script, NULL, scroll_js_finished_cb, NULL);
}

void
marker_preview_scroll_down (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_STEP_SCRIPT, 0, SCROLL_STEP);
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (preview), script, NULL, scroll_js_finished_cb, NULL);
}

void
marker_preview_scroll_to_top (MarkerPreview *preview)
{
  g_autofree gchar *script = g_strdup_printf (SCROLL_SCRIPT, 0, 0);
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (preview), script, NULL, scroll_js_finished_cb, NULL);
}

void
marker_preview_scroll_to_bottom (MarkerPreview *preview)
{
  const gchar *script = "window.scrollTo(0,document.body.scrollHeight);";
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (preview), script, NULL, scroll_js_finished_cb, NULL);
}
