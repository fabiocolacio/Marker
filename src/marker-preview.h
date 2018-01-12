#ifndef __MARKER_PREVIEW_H__
#define __MARKER_PREVIEW_H__

#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define MARKER_TYPE_PREVIEW (marker_preview_get_type ())

G_DECLARE_FINAL_TYPE (MarkerPreview,
                      marker_preview,
                      MARKER,
                      PREVIEW,
                      WebKitWebView)

MarkerPreview *
marker_preview_new (void);

void
marker_preview_zoom_out (MarkerPreview *preview);

void
marker_preview_zoom_original (MarkerPreview *preview);

void
marker_preview_zoom_in (MarkerPreview *preview);

void
marker_preview_render_markdown (MarkerPreview *preview,
                                const char    *markdown,
                                const char    *css_theme,
                                const char    *base_uri);

WebKitPrintOperationResponse
marker_preview_run_print_dialog (MarkerPreview *preview,
                                 GtkWindow     *parent);

void
marker_preview_print_pdf (MarkerPreview *preview,
                          const char    *outfile);

G_END_DECLS

#endif
