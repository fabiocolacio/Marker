#ifndef __MARKER_PREVIEW_H__
#define __MARKER_PREVIEW_H__

#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define MARKER_TYPE_PREVIEW (marker_preview_get_type ())

G_DECLARE_FINAL_TYPE(MarkerPreview,
                     marker_preview,
                     MARKER,
                     PREVIEW,
                     WebKitWebView)

MarkerPreview*
marker_preview_new(void);

G_END_DECLS

#endif

