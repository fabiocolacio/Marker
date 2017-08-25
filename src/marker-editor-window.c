#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "marker-editor-window.h"
#include "marker-utils.h"

struct _MarkerEditorWindow
{
    GtkApplicationWindow  parent_instance;
    
    GtkWidget* header_bar;
    GtkWidget* source_view;
    GtkWidget* web_view;
    GtkWidget* popover;
    
    gboolean   refresh_scheduled;
    
    gboolean   unsaved_changes;
    char*      file_name;
    char*      file_location;
    GFile*     file;
    
    char*      stylesheet_name;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_APPLICATION_WINDOW)

const char* TMP_MD   = ".marker_tmp.md";
const char* TMP_HTML = ".marker_tmp.html";

static void
show_unsaved_documents_warning(GtkWindow* window)
{
    GtkWidget* dialog =
        gtk_message_dialog_new_with_markup(window,
                                           GTK_DIALOG_MODAL,
                                           GTK_MESSAGE_QUESTION,
                                           GTK_BUTTONS_OK_CANCEL,
                                           "<span weight='bold' size='larger'>"
                                           "Discard changes to this document?"
                                           "</span>\n\n"
                                           "This document has unsaved changes "
                                           "that will be lost if you quit.");
                                           
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    switch (response)
    {
        case GTK_RESPONSE_OK:
            gtk_widget_destroy(GTK_WIDGET(window));
            break;
    }
    
    gtk_widget_destroy(dialog);
}

static void
web_view_load_event(WebKitWebView* web_view,
                    gint           progress)
{
    if (progress >= 100)
    {
        remove(TMP_HTML);
    }
}

void
marker_editor_window_refresh_web_view(MarkerEditorWindow* self)
{
    int ret;
    if (G_IS_FILE(self->file))
    {
        char* path = g_file_get_path(self->file);
        int last_slash = marker_utils_rfind('/', path);
        path[last_slash] = '\0';
        ret = chdir(path);
        if (ret != 0)
        {
            puts("There was a problem changing directories");
        }
        g_free(path);
    }
    
    GtkTextBuffer* buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start_iter);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end_iter);
    char* buffer_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),
                                                 &start_iter,
                                                 &end_iter,
                                                 FALSE);
    
    FILE* fp = fopen(TMP_MD, "w");
    fputs(buffer_text, fp);
    fclose(fp);
    g_free(buffer_text);
    
    char command[256] = "pandoc -s ";
    if (self->stylesheet_name)
    {
        strcat(command, "-c ");
        strcat(command, STYLES_DIR);
        strcat(command, self->stylesheet_name);
    }
    strcat(command, " -o ");
    strcat(command, TMP_HTML);
    strcat(command, " ");
    strcat(command, TMP_MD);
    strcat(command, "\0");
    ret = system(command);
    if (ret != 0)
    {
        puts(command);
        puts("There was a problem generating the html document");
    }
    
    char* cwd = getcwd(NULL, 0);
    if (cwd == NULL)
    {
        puts("There was a problem getting the current working directory");
    }
    
    int cwd_len = strlen(cwd);
    int html_len = strlen(TMP_HTML);
    int html_full_path_len = cwd_len + html_len + 2;
    char html_full_path[html_full_path_len];
    memset(html_full_path, 0, html_full_path_len);
    strcat(html_full_path, cwd);
    strcat(html_full_path, "/");
    strcat(html_full_path, TMP_HTML);
    
    char uri[html_full_path_len + strlen("file://") + 1];
    memset(uri, 0, sizeof(uri));
    strcat(uri, "file://");
    strcat(uri, html_full_path);
    
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(self->web_view), uri);
    g_signal_connect(self->web_view, "load-progress-changed", G_CALLBACK(web_view_load_event), NULL);
    
    remove(TMP_MD);
    free(cwd);
}

void
marker_editor_window_open_file(MarkerEditorWindow* self,
                               GFile*              file)
{
    if (G_IS_FILE(file))
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
            self->file = file;
            
            GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
            gtk_text_buffer_set_text(buffer, file_contents, file_size);
            char* basename = g_file_get_basename(file);
            char* path = g_file_get_path(file);
            int last_slash = marker_utils_rfind('/', path);
            path[last_slash] = '\0';
            gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), basename);
            gtk_header_bar_set_subtitle(GTK_HEADER_BAR(self->header_bar), path);
            marker_editor_window_refresh_web_view(self);
            
            g_free(basename);
            g_free(path);
            g_free(file_contents);
            
            self->unsaved_changes = FALSE;
        }
    }
}

void
marker_editor_window_save_file_as(MarkerEditorWindow* self,
                                  GFile*              file)
{
    if (G_IS_FILE(file))
    {
        GError* err = NULL;
        
        GFileOutputStream* stream = g_file_replace(file,
                                                   NULL,
                                                   false,
                                                   G_FILE_CREATE_NONE,
                                                   NULL,
                                                   &err);
        if (err)
        {
            printf("There was a problem opening the file stream!\n\n%s\n", err->message);
            g_error_free(err);
        }
        else
        {
            GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
            GtkTextIter    start_iter;
            GtkTextIter    end_iter;
            gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start_iter);
            gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end_iter);
            gchar* buffer_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),
                                                          &start_iter,
                                                          &end_iter,
                                                          FALSE);
            size_t buffer_size = strlen(buffer_text);
            gsize bytes_written = 0;
            g_output_stream_write_all(G_OUTPUT_STREAM(stream),
                                      buffer_text,
                                      buffer_size,
                                      &bytes_written,
                                      NULL,
                                      &err);
            
            if (err)
            {
                printf("There was a problem writing to the file!\n\n%s\n", err->message);
                g_error_free(err);
            }
                                
            g_output_stream_close(G_OUTPUT_STREAM(stream), FALSE, &err);
            if (err)
            {
                printf("There was a problem closing the stream!\n\n%s\n", err->message);
                g_error_free(err);
            }
                   
            self->unsaved_changes = FALSE;
            self->file = file;
            
            char* basename = g_file_get_basename(file);
            char* path = g_file_get_path(file);
            int last_slash = marker_utils_rfind('/', path);
            path[last_slash] = '\0';
            gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), basename);
            gtk_header_bar_set_subtitle(GTK_HEADER_BAR(self->header_bar), path);
            marker_editor_window_refresh_web_view(self);
            g_free(basename);
            g_free(path);
            g_free(buffer_text);
        }
    }
}

static void save_as_btn_pressed(GtkWidget*          widget,
                                MarkerEditorWindow* self)
{    
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
                                                    GTK_WINDOW(self),
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
        marker_editor_window_save_file_as(self, file);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void
save_btn_pressed(GtkWidget*          widget,
                 MarkerEditorWindow* self)
{
    if (G_IS_FILE(self->file))
    {
        marker_editor_window_save_file_as(self, self->file);
    }
    else
    {
        save_as_btn_pressed(widget, self);
    }
}

static void
open_btn_pressed(GtkWidget*          widget,
                 MarkerEditorWindow* self)
{    
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
                                                    GTK_WINDOW(self),
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
        
        GtkApplication* app;
        g_object_get(self, "application", &app, NULL);
        MarkerEditorWindow* win = marker_editor_window_new_from_file(app, file);
        gtk_widget_show_all(GTK_WIDGET(win));
        g_object_unref(app);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void
new_btn_pressed(GtkWidget*          widget,
                MarkerEditorWindow* self)
{
    GtkApplication* app;
    g_object_get(self, "application", &app, NULL);
    MarkerEditorWindow* win = marker_editor_window_new(app);
    gtk_widget_show_all(GTK_WIDGET(win));
    g_object_unref(app);
}

static void
refresh_btn_pressed(GtkWidget*          widget,
                    MarkerEditorWindow* self)
{
    marker_editor_window_refresh_web_view(self);
}

static gboolean
auto_refresh(gpointer user_data)
{
    MarkerEditorWindow* self = user_data;
    marker_editor_window_refresh_web_view(self);
    self->refresh_scheduled = FALSE;
    return FALSE;
}

static void
source_buffer_changed(GtkTextBuffer*      buffer,
                      MarkerEditorWindow* self)
{
    if (!self->unsaved_changes)
    {
        self->unsaved_changes = TRUE;
        
        if (G_IS_FILE(self->file))
        {
            char* basename = g_file_get_basename(self->file);        
            int len = strlen(basename);
            char title[len + 2];
            memset(title, 0, len + 2);
            strcat(title, "*");
            strcat(title, basename);
            gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), title);
            g_free(basename);
        }
        else
        {
            gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), "*Untitled.md");
            gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(self->header_bar), FALSE);
        }
    }
    
    if (!self->refresh_scheduled)
    {
        g_timeout_add_seconds(1, auto_refresh, self);
        self->refresh_scheduled = TRUE;
    }
}

static void
menu_btn_toggled(GtkToggleButton*    button,
                 MarkerEditorWindow* self)
{
    gtk_widget_set_visible(self->popover, gtk_toggle_button_get_active(button));
}

static void
menu_popover_closed(GtkPopover*      popover,
                    GtkToggleButton* toggle_btn)
{
    gtk_toggle_button_set_active(toggle_btn, FALSE);
}

static gboolean
close_btn_pressed(MarkerEditorWindow* self,
                  gpointer*           user_data)
{
    if (self->unsaved_changes)
    {
        show_unsaved_documents_warning(GTK_WINDOW(self));
        return TRUE;
    }
    return FALSE;
}

static void
marker_editor_window_init(MarkerEditorWindow* self)
{   
    self->file = NULL;
    self->unsaved_changes = FALSE;
    self->refresh_scheduled = FALSE;
    self->stylesheet_name = "marker.css";
    
    GError* err = NULL;
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/com/github/fabiocolacio/marker/marker-editor-window.ui", &err);
    if (err)
    {
        puts(err->message);
        g_error_free(err);
    }
    
    GtkSourceLanguageManager* source_language_manager = gtk_source_language_manager_get_default();
    GtkSourceLanguage* source_language = gtk_source_language_manager_get_language(source_language_manager, "markdown");
    GtkSourceBuffer* source_buffer = gtk_source_buffer_new_with_language(source_language);
    g_object_unref(source_language_manager);

    GtkWidget* widget;
    
    self->header_bar = GTK_WIDGET(gtk_builder_get_object(builder, "header_bar"));
    self->source_view = GTK_WIDGET(gtk_builder_get_object(builder, "source_view"));
    self->web_view = GTK_WIDGET(gtk_builder_get_object(builder, "web_view"));
    
    gtk_widget_grab_focus(GTK_WIDGET(self->source_view));
    
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(self->source_view), GTK_TEXT_BUFFER(source_buffer));
    g_signal_connect(source_buffer, "changed", G_CALLBACK(source_buffer_changed), self);
    
    self->popover = GTK_WIDGET(gtk_builder_get_object(builder, "menu_popover"));
    widget = GTK_WIDGET(gtk_builder_get_object(builder, "menu_toggler"));
    gtk_popover_set_relative_to(GTK_POPOVER(self->popover), widget);
    g_signal_connect(self->popover, "closed", G_CALLBACK(menu_popover_closed), widget);
    
    widget = GTK_WIDGET(gtk_builder_get_object(builder, "editor"));
    gtk_container_add(GTK_CONTAINER(self), widget);
    gtk_window_set_titlebar(GTK_WINDOW(self), self->header_bar);
    
    gtk_window_set_default_size(GTK_WINDOW(self), 800, 500);
    
    g_signal_connect(self, "delete-event", G_CALLBACK(close_btn_pressed), NULL); 
    
    gtk_builder_add_callback_symbol(builder, "open_btn_pressed", G_CALLBACK(open_btn_pressed));
    gtk_builder_add_callback_symbol(builder, "save_btn_pressed", G_CALLBACK(save_btn_pressed));
    gtk_builder_add_callback_symbol(builder, "save_as_btn_pressed", G_CALLBACK(save_as_btn_pressed));
    gtk_builder_add_callback_symbol(builder, "new_btn_pressed", G_CALLBACK(new_btn_pressed));
    gtk_builder_add_callback_symbol(builder, "refresh_btn_pressed", G_CALLBACK(refresh_btn_pressed));
    gtk_builder_add_callback_symbol(builder, "menu_btn_toggled", G_CALLBACK(menu_btn_toggled));
    gtk_builder_connect_signals(builder, self);
    
    g_object_unref(builder);
}

static void
marker_editor_window_class_init(MarkerEditorWindowClass* class)
{

}

MarkerEditorWindow*
marker_editor_window_new(GtkApplication* app)
{
    return g_object_new(MARKER_TYPE_EDITOR_WINDOW,
                        "application", app,
                        NULL);
}

MarkerEditorWindow*
marker_editor_window_new_from_file(GtkApplication* app,
                                   GFile*          file)
{
    MarkerEditorWindow* win = marker_editor_window_new(app);
    if (G_IS_FILE(file))
    {
        marker_editor_window_open_file(win, file);
    }
    return win;
}

