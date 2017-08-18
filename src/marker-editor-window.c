#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "marker-editor-window.h"

struct _MarkerEditorWindow
{
    GtkWindow parent_instance;
    
    GtkWidget* paned;
    GtkWidget* source_view;
    GtkWidget* web_view;
    GtkWidget* header_bar;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_WINDOW)

static void
open_btn_pressed(GtkWidget* widget,
                 gpointer   user_data)
{
    GtkWidget*      dialog;
    GtkWidget*      self;
    GtkFileChooser* chooser;
    char*           filename;
    gint            response;
    FILE*           fp;
    
    self = user_data;
    
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         user_data,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT)
    {
        chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        
        /* TODO: Load contents of file into source buffer */
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void
refresh_btn_pressed(GtkWidget* widget,
                    gpointer   user_data)
{
    MarkerEditorWindow* self;
    GtkTextBuffer* buffer;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    char uri[50];
    char cwd[50];
    gchar* buffer_text;
    FILE* fp;
    
    self = user_data;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start_iter);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end_iter);
    
    buffer_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),
                                           &start_iter,
                                           &end_iter,
                                           FALSE);
    
    fp = fopen("tmp.md", "w");
    fprintf(fp, buffer_text);
    fclose(fp);
    
    system("pandoc -s -o tmp.html tmp.md");
    
    memset(uri, 0, 50);
    memset(cwd, 0, 50);
    
    getcwd(cwd, 50);
    strcat(uri, "file://");
    strcat(uri, cwd);
    strcat(uri, "/tmp.html");
    
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(self->web_view), uri);
}

static void
marker_editor_window_init(MarkerEditorWindow* self)
{
    GtkWidget* refresh_btn;
    GtkWidget* open_btn;
    GtkSourceLanguageManager* source_language_manager;
    GtkSourceLanguage* source_language;
    GtkSourceBuffer* source_buffer;

    refresh_btn = gtk_button_new_from_icon_name("view-refresh-symbolic",
                                                GTK_ICON_SIZE_SMALL_TOOLBAR);
    open_btn = gtk_button_new_from_icon_name("document-open-symbolic",
                                             GTK_ICON_SIZE_SMALL_TOOLBAR);
    
    source_language_manager = gtk_source_language_manager_get_default();
    source_language = gtk_source_language_manager_get_language(source_language_manager, "markdown");
    source_buffer = gtk_source_buffer_new_with_language(source_language);

    self->header_bar = gtk_header_bar_new();
    self->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    self->source_view = gtk_source_view_new_with_buffer(source_buffer);
    self->web_view = webkit_web_view_new();
    
    g_signal_connect(open_btn, "clicked", G_CALLBACK(open_btn_pressed), self);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(refresh_btn_pressed), self);
    
    gtk_window_set_default_size(GTK_WINDOW(self), 800, 700);
    gtk_window_set_titlebar(GTK_WINDOW(self), self->header_bar);
    
    gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), "Untitled.md");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(self->header_bar), TRUE);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(self->header_bar), open_btn);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(self->header_bar), refresh_btn);
    
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

