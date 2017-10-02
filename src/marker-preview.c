
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

