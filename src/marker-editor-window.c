#include <gtksourceview/gtksource.h>
#include <webkit/webkitwebview.h>

#include "marker-editor-window.h"

struct _MarkerEditorWindow
{
  GtkApplicationWindow parent_instance;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_APPLICATION_WINDOW)


static void
init_ui(MarkerEditorWindow* window)
{
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(window), "Untitled.md");
  
  GtkBuilder* builder =
    gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/editor-window.ui");

  GtkBox* vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));

  { // Tool Bar //
    GtkWidget* header_bar =
      GTK_WIDGET(gtk_builder_get_object(builder, "header_bar"));
    gtk_box_pack_start(vbox, header_bar, FALSE, TRUE, 0);
  }

  { // Paned Editor //
    GtkWidget* scrolled_window;
    GtkPaned* paned = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_paned_set_wide_handle(paned, TRUE);
    gtk_paned_set_position(paned, 450);
    
    GtkWidget* source_view = gtk_source_view_new();
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), source_view);
    gtk_paned_add1(paned, scrolled_window);
    
    GtkWidget* web_view = webkit_web_view_new();
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), web_view);
    gtk_paned_add2(paned, scrolled_window);
    
    gtk_box_pack_start(vbox, GTK_WIDGET(paned), TRUE, TRUE, 0);
  }
  
  g_object_unref(builder);
}

static void
marker_editor_window_init(MarkerEditorWindow* window)
{
  init_ui(window);
}

static void
marker_editor_window_class_init(MarkerEditorWindowClass* class)
{

}

MarkerEditorWindow*
marker_editor_window_new(GtkApplication* app)
{
  return g_object_new(MARKER_TYPE_EDITOR_WINDOW, "application", app, NULL);
}

