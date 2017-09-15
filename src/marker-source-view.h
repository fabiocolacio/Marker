#ifndef __MARKER_SOURCE_VIEW_H__
#define __MARKER_SOURCE_VIEW_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define MARKER_TYPE_SOURCE_VIEW (marker_source_view_get_type ())

G_DECLARE_FINAL_TYPE(MarkerSourceView,
                     marker_source_view,
                     MARKER,
                     SOURCE_VIEW,
                     GtkSourceView)

MarkerSourceView*
marker_source_view_new(void);

gchar*
marker_source_view_get_text(MarkerSourceView* source_view);

void
marker_source_view_set_text(MarkerSourceView* source_view,
                            char*             text,
                            size_t            size);

void
marker_source_view_set_language(MarkerSourceView* source_view,
                                const gchar*      language);

G_END_DECLS

#endif

