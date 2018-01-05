#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include "marker-markdown.h"
#include "marker-prefs.h"

#include "marker-preview.h"

struct _MarkerPreview
{
  WebKitWebView parent_instance;
};

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
  MarkerMathJaxMode mathjax_mode = MATHJAX_OFF;
  if (marker_prefs_get_use_mathjax()) {
    mathjax_mode = MATHJAX_LOCAL;
  }
  char* html = marker_markdown_to_html(markdown, strlen(markdown), mathjax_mode, css_theme);
  const char* uri = (base_uri) ? base_uri : "file://";
  WebKitWebView* web_view = WEBKIT_WEB_VIEW(preview);
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

