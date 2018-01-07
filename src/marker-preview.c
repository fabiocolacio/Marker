#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <time.h>

#include "marker-markdown.h"
#include "marker-prefs.h"

#include "marker-preview.h"
#include "marker.h"


struct _MarkerPreview
{
  WebKitWebView parent_instance;
};

static void
initialize_web_extensions (WebKitWebContext *context,
                           gpointer          user_data)
{
  /* Web Extensions get a different ID for each Web Process */
  static guint32 unique_id = 0;

  webkit_web_context_set_web_extensions_directory (
     context, WEB_EXTENSIONS_DIRECTORY);
  /* webkit_web_context_set_web_extensions_initialization_user_data (
     context, g_variant_new_uint32 (unique_id++));*/
}


/* Open uri in default browser.
 */
static
void open_uri(WebKitResponsePolicyDecision *decision){
  const gchar * uri = webkit_uri_request_get_uri(webkit_response_policy_decision_get_request(decision));
  g_print("open %s\n", uri);
  GtkApplication * app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  time_t now = time(0); // Get the system time
  gtk_show_uri_on_window (windows->data,
                          uri,
                          now,NULL);
}

static gboolean
decide_policy_cb (WebKitWebView *web_view,
                  WebKitPolicyDecision *decision,
                  WebKitPolicyDecisionType type)
{
    switch (type) {
    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
        /* ignore default policy and open uri in default browser*/
        open_uri((WebKitResponsePolicyDecision*)decision);
        webkit_policy_decision_ignore(decision);
        break;
    default:
        /* Making no decision results in webkit_policy_decision_use(). */
        return FALSE;
    }
    return TRUE;
}


static gboolean
disable_menu  (WebKitWebView       *web_view,
               WebKitContextMenu   *context_menu,
               GdkEvent            *event,
               WebKitHitTestResult *hit_test_result,
               gpointer             user_data)
{
  return TRUE;
}

G_DEFINE_TYPE(MarkerPreview, marker_preview, WEBKIT_TYPE_WEB_VIEW)

MarkerPreview*
marker_preview_new(void)
{
  return g_object_new(MARKER_TYPE_PREVIEW, NULL);
}

static void
marker_preview_init(MarkerPreview* preview)
{

}

static void
load_changed(WebKitWebView*  preview,
             WebKitLoadEvent event)
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
marker_preview_class_init(MarkerPreviewClass* class)
{
  WEBKIT_WEB_VIEW_CLASS(class)->load_changed = load_changed;
}

void
marker_preview_render_markdown(MarkerPreview* preview,
                               const char*    markdown,
                               const char*    css_theme,
                               const char*    base_uri)
{
  MarkerKaTeXMode katex_mode = KATEX_OFF;
  if (marker_prefs_get_use_katex()) {
    katex_mode = KATEX_LOCAL;
  }
  MarkerHighlightMode highlight_mode = HIGHLIGHT_OFF;
  if (marker_prefs_get_use_highlight()){
    highlight_mode = HIGHLIGHT_LOCAL;
  }

  g_signal_connect (webkit_web_context_get_default (),
                   "initialize-web-extensions",
                    G_CALLBACK (initialize_web_extensions),
                    NULL);

  char* html = marker_markdown_to_html(markdown, strlen(markdown), katex_mode, highlight_mode, css_theme);
  const char* uri = (base_uri) ? base_uri : "file://";
  WebKitWebView* web_view = WEBKIT_WEB_VIEW(preview);

  g_signal_connect(web_view,
                   "decide-policy",
                   G_CALLBACK(decide_policy_cb),
                   NULL);
  g_signal_connect(web_view,
                   "context-menu",
                   G_CALLBACK(disable_menu),
                   NULL);

  webkit_web_view_load_html(web_view, html, uri);
  free(html);
}

static void
print_failed(WebKitPrintOperation* print_op,
             GError*               err,
             gpointer              user_data)
{
  g_printerr("print failed with error: %s\n", err->message);
}

static void
print_finished()
{

}

void
marker_preview_print_pdf(MarkerPreview* preview,
                         const char*    outfile)

{
    WebKitPrintOperation* print_op = NULL;
    GtkPrintSettings* print_s = NULL;
    char* uri = g_strdup_printf("file://%s", outfile);
    
    print_op = webkit_print_operation_new(WEBKIT_WEB_VIEW(preview));
    g_signal_connect(print_op, "failed", G_CALLBACK(print_failed), NULL);
    g_signal_connect(print_op, "finished", G_CALLBACK(print_finished), NULL);
    
    print_s = gtk_print_settings_new();
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_PRINTER, "Print to File");
    webkit_print_operation_set_print_settings(print_op, print_s);
    
    webkit_print_operation_print(print_op);

    g_free(uri);
}


