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

gboolean
marker_source_view_get_modified(MarkerSourceView* source_view);

gboolean
marker_source_view_set_modified(MarkerSourceView* source_view,
                                gboolean          modified);

gchar*
marker_source_view_get_text(MarkerSourceView* source_view);

void
marker_source_view_set_text(MarkerSourceView* source_view,
                            const char*       text,
                            size_t            size);

void
marker_source_view_set_language(MarkerSourceView* source_view,
                                const gchar*      language);
                                
void
marker_source_view_set_syntax_theme(MarkerSourceView* source_view,
                                    const char*       syntax_theme);

void
marker_source_view_surround_selection_with(MarkerSourceView* source_view,
                                           const char*       insertion);

G_END_DECLS

#endif

