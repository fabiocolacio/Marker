#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <JavaScriptCore/JSValueRef.h>

#include "marker-markdown.h"

#include "marker-preview.h"

struct _MarkerPreview
{
  WebKitWebView parent_instance;
  
  gdouble scrollX;
  gdouble scrollY;
  
  int script;
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
  WebKitSettings* settings = webkit_settings_new();
  webkit_settings_set_enable_javascript(settings, TRUE);
  webkit_web_view_set_settings(WEBKIT_WEB_VIEW(preview), settings);
  preview->scrollX = 0.0;
  preview->scrollY = 0.0;
  preview->script = 0;
}

void
update_scrollx(GObject*       object,
               GAsyncResult*  result,
               gpointer       user_data)
{
  WebKitJavascriptResult *js_result;
  JSValueRef              value;
  JSGlobalContextRef      context;
  GError                 *error = NULL;

  js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
  if (!js_result)
  {
    g_warning("Error running javascript: %s", error->message);
    g_error_free(error);
    return;
  }

  context = webkit_javascript_result_get_global_context (js_result);
  value = webkit_javascript_result_get_value (js_result);
  if (JSValueIsNumber (context, value))
  {
    double val = JSValueToNumber(context, value, NULL);
    MARKER_PREVIEW(object)->scrollX = val;
  }
  else
  {
    g_warning("Error running javascript: unexpected return value");
  }
  
  webkit_javascript_result_unref (js_result);
  MARKER_PREVIEW(object)->script -= 1;
}

static void
update_scrolly(GObject*       object,
               GAsyncResult*  result,
               gpointer       user_data)
{
  WebKitJavascriptResult *js_result;
  JSValueRef              value;
  JSGlobalContextRef      context;
  GError                 *error = NULL;

  js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
  if (!js_result)
  {
    g_warning("Error running javascript: %s", error->message);
    g_error_free(error);
    return;
  }

  context = webkit_javascript_result_get_global_context (js_result);
  value = webkit_javascript_result_get_value (js_result);
  if (JSValueIsNumber (context, value))
  {
    double val = JSValueToNumber(context, value, NULL);
    MARKER_PREVIEW(object)->scrollY = val;
  }
  else
  {
    g_warning("Error running javascript: unexpected return value");
  }
  
  webkit_javascript_result_unref (js_result);
  MARKER_PREVIEW(object)->script -= 1;
}

static void
updated_scroll_pos(GObject*       object,
               GAsyncResult*  result,
               gpointer       user_data)
{
  MARKER_PREVIEW(object)->script -= 1;
}

void
marker_preview_update_scroll_pos(MarkerPreview* web_view)
{
  webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(web_view), "window.scrollX", NULL, update_scrollx, NULL);
  webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(web_view), "window.scrollY", NULL, update_scrolly, NULL);
  web_view->script += 2;
}

void
marker_preview_scroll_to_pos(MarkerPreview* preview, gdouble x, gdouble y)
{
  gchar* command = NULL;
  command = g_strdup_printf("window.scroll(%f, %f);", x, y);
  
  if (command)
  {
    webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(preview), command, NULL, updated_scroll_pos, NULL);
    preview->script += 1;
    g_free(command);
  }
}

static void
load_changed(WebKitWebView*  preview,
             WebKitLoadEvent event)
{
  MarkerPreview* prev = MARKER_PREVIEW(preview);
  switch (event)
  {
    case WEBKIT_LOAD_STARTED:
      marker_preview_update_scroll_pos(prev);
      break;
      
    case WEBKIT_LOAD_REDIRECTED:
      break;
      
    case WEBKIT_LOAD_COMMITTED:
      break;
      
    case WEBKIT_LOAD_FINISHED:
      marker_preview_scroll_to_pos(prev, prev->scrollX, prev->scrollY);
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
  char* html = marker_markdown_to_html(markdown, strlen(markdown), css_theme);
  const char* uri = (base_uri) ? base_uri : "file://";
  WebKitWebView* web_view = WEBKIT_WEB_VIEW(preview);
  webkit_web_view_load_html(web_view, html, uri);
  free(html);
}



