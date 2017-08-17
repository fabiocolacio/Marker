#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>

#include "marker-editor-window.h"

struct _MarkerEditorWindow
{
    GtkWindow parent_instance;
    
    GtkWidget* paned;
    GtkWidget* source_view;
    GtkWidget* web_view;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_WINDOW)

static void
marker_editor_window_init(MarkerEditorWindow* self)
{
    gtk_window_set_title(GTK_WINDOW(self), "Untitled.md");
    gtk_window_set_default_size(GTK_WINDOW(self), 500, 500);
    
    self->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    self->source_view = gtk_source_view_new();
    self->web_view = webkit_web_view_new();
    
    gtk_paned_add1(GTK_PANED(self->paned), self->source_view);
    gtk_paned_add2(GTK_PANED(self->paned), self->web_view);
    
    gtk_container_add(GTK_CONTAINER(self), self->paned);
}

static void
marker_editor_window_class_init(MarkerEditorWindowClass* class)
{
    
}

MarkerEditorWindow*
marker_editor_window_new(void)
{
    return g_object_new(MARKER_TYPE_EDITOR_WINDOW, NULL);
}

