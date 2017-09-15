#include <gtksourceview/gtksource.h>
#include <webkit/webkitwebview.h>

#include <stdlib.h>
#include <string.h>

#include "marker.h"
#include "marker-source-view.h"
#include "marker-markdown.h"

#include "marker-editor-window.h"

struct _MarkerEditorWindow
{
  GtkApplicationWindow parent_instance;
  
  MarkerSourceView* source_view;
  WebKitWebView* web_view;
  GFile* file;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_APPLICATION_WINDOW)

void
marker_editor_window_refresh_preview(MarkerEditorWindow* window)
{
  WebKitWebView* web_view = window->web_view;
  gchar* markdown = marker_editor_window_get_markdown(window);
  char* html = marker_markdown_to_html(markdown, strlen(markdown));
  
  gchar* uri = NULL;
  if (G_IS_FILE(window->file)) { uri = g_file_get_uri(window->file); }
  
  webkit_web_view_load_string(web_view,
                              html,
                              "text/html",
                              "UTF-8",
                              (uri) ? uri : "file://");
  
  if (uri) { g_free(uri); }
  
  free(html);
  g_free(markdown);
}

void
marker_editor_window_open_file(MarkerEditorWindow* window,
                               GFile*              file)
{
  char* file_contents = NULL;
  gsize file_size = 0;
  GError* err = NULL;
  
  g_file_load_contents(file, NULL, &file_contents, &file_size, NULL, &err);
  
  if (err)
  {
    printf("There was a problem opening the file!\n\n%s\n", err->message);
    g_error_free(err);
  }
  else
  {
    window->file = file;
    char* filepath = g_file_get_path(file);
    gtk_window_set_title(GTK_WINDOW(window), filepath);
    g_free(filepath);
    
    marker_source_view_set_text(window->source_view, file_contents, file_size);
    g_free(file_contents);
  }
}
                               
void
marker_editor_window_save_file(MarkerEditorWindow* window,
                               GFile*              file)
{
  char* filepath = g_file_get_path(file);
  FILE* fp = NULL;
  fp = fopen(filepath, "w");
  if (fp)
  {
    char* contents = marker_editor_window_get_markdown(window);
    fputs(contents, fp);
    fclose(fp);
    
    window->file = file;
    gtk_window_set_title(GTK_WINDOW(window), filepath);
  }
  g_free(filepath);
}

static void
open_cb(GtkWidget*          widget,
        MarkerEditorWindow* window)
{
  GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
                                                  GTK_WINDOW(window),
                                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                                  "Cancel",
                                                  GTK_RESPONSE_CANCEL,
                                                  "Open",
                                                  GTK_RESPONSE_ACCEPT,
                                                  NULL);
  
  gint response = gtk_dialog_run(GTK_DIALOG(dialog));
  if (response == GTK_RESPONSE_ACCEPT)
  {
    GtkFileChooser* chooser = GTK_FILE_CHOOSER (dialog);
    char* filename = gtk_file_chooser_get_filename (chooser);
    
    GFile* file = g_file_new_for_path(filename);
    marker_create_new_window_from_file(file);
    
    g_free(filename);
  }
    
  gtk_widget_destroy(dialog);
}

static void
save_as_cb(GtkWidget*          widget,
           MarkerEditorWindow* window)
{
  GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
                                                  GTK_WINDOW(window),
                                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                                  "Cancel",
                                                  GTK_RESPONSE_CANCEL,
                                                  "Save",
                                                  GTK_RESPONSE_ACCEPT,
                                                  NULL);
        
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
  gint response = gtk_dialog_run(GTK_DIALOG(dialog));
  if (response == GTK_RESPONSE_ACCEPT)
  {
    GtkFileChooser* chooser = GTK_FILE_CHOOSER (dialog);
    char* filename = gtk_file_chooser_get_filename (chooser);
    GFile* file = g_file_new_for_path(filename);
    marker_editor_window_save_file(window, file);
        
    g_free(filename);
  }
    
gtk_widget_destroy(dialog);
}

static void
save_cb(GtkWidget*          widget,
        MarkerEditorWindow* window)
{
  if (G_IS_FILE(window->file))
  {
    marker_editor_window_save_file(window, window->file);
  }
  else
  {
    save_as_cb(widget, window);
  }
}

gchar*
marker_editor_window_get_markdown(MarkerEditorWindow* window)
{
  return marker_source_view_get_text(window->source_view);
}

static void
buffer_changed(GtkTextBuffer*      buffer,
               MarkerEditorWindow* window)
{
  marker_editor_window_refresh_preview(window);
}

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

  // Tool Bar //
  GtkWidget* header_bar =
    GTK_WIDGET(gtk_builder_get_object(builder, "header_bar"));
  
  gtk_box_pack_start(vbox, header_bar, FALSE, TRUE, 0);
  
  GtkMenuButton* menu_btn =
    GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn"));  
  
  GMenuModel* gear_menu =
    G_MENU_MODEL(gtk_builder_get_object(builder, "gear_menu"));
    
  gtk_menu_button_set_use_popover(menu_btn, TRUE);
  gtk_menu_button_set_menu_model(menu_btn, gear_menu);

  // Paned Editor //
  GtkWidget* scrolled_window;
  GtkPaned* paned = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_HORIZONTAL));
  gtk_paned_set_wide_handle(paned, TRUE);
  gtk_paned_set_position(paned, 450);
  gtk_box_pack_start(vbox, GTK_WIDGET(paned), TRUE, TRUE, 0);
  
  // Source View //
  GtkWidget* source_view = GTK_WIDGET(marker_source_view_new());
  window->source_view = MARKER_SOURCE_VIEW(source_view);
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_window), source_view);
  gtk_paned_add1(paned, scrolled_window);
  gtk_widget_grab_focus(source_view);
  GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  g_signal_connect(buf, "changed", G_CALLBACK(buffer_changed), window);
  
  // Web View //
  GtkWidget* web_view = webkit_web_view_new();
  window->web_view = WEBKIT_WEB_VIEW(web_view);
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_window), web_view);
  gtk_paned_add2(paned, scrolled_window);
  
  gtk_builder_add_callback_symbol(builder, "open_cb", G_CALLBACK(open_cb));
  gtk_builder_add_callback_symbol(builder, "save_cb", G_CALLBACK(save_cb));
  gtk_builder_connect_signals(builder, window);
  
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

MarkerEditorWindow*
marker_editor_window_new_from_file(GtkApplication* app,
                                   GFile*          file)
{
  MarkerEditorWindow* window = marker_editor_window_new(app);
  marker_editor_window_open_file(window, file);
  return window;
}

