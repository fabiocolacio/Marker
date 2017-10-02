#include <string.h>
#include <stdlib.h>

#include "marker-markdown.h"

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
marker_preview_class_init(MarkerPreviewClass* class)
{

}

void
marker_preview_render_markdown(MarkerPreview* preview,
                               const char*    markdown,
                               const char*    css_theme,
                               const char*    base_uri)
{
  char* html = (css_theme)
    ? marker_markdown_to_html_with_css(markdown, strlen(markdown), css_theme)
    : marker_markdown_to_html(markdown, strlen(markdown));
  const char* uri = (base_uri) ? base_uri : "file://";
  WebKitWebView* web_view = WEBKIT_WEB_VIEW(preview);
  webkit_web_view_load_html(web_view, html, uri);
  free(html);
}

