
#include "marker-source-view.h"

struct _MarkerSourceView
{
  GtkSourceView parent_instance;
};

G_DEFINE_TYPE(MarkerSourceView, marker_source_view, GTK_SOURCE_TYPE_VIEW)

gchar*
marker_source_view_get_text(MarkerSourceView* source_view)
{
  if (GTK_SOURCE_IS_VIEW(source_view))
  {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(source_view);
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  }
  return NULL;
}

void
marker_source_view_set_language(MarkerSourceView* source_view,
                                const gchar*      language_name)
{
  if (GTK_SOURCE_IS_VIEW(source_view))
  {
    GtkSourceLanguageManager* manager =
      gtk_source_language_manager_get_default();
      
    GtkSourceLanguage* language =
      gtk_source_language_manager_get_language(manager, language_name);
      
    GtkSourceBuffer* buffer =
      GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view)));
    
    gtk_source_buffer_set_language(buffer, language);
    
    g_object_unref(manager);
  }
}

static void
marker_source_view_init(MarkerSourceView* source_view)
{
  marker_source_view_set_language(source_view, "markdown");
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(source_view), TRUE);
}

static void
marker_source_view_class_init(MarkerSourceViewClass* class)
{
  
}

MarkerSourceView*
marker_source_view_new(void)
{
  return g_object_new(MARKER_TYPE_SOURCE_VIEW, NULL);
}

