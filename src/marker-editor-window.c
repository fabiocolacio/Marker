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
    GtkWindow parent_instance;
    
    GtkWidget* paned;
    GtkWidget* source_view;
    GtkWidget* web_view;
    GtkWidget* header_bar;
    
    gboolean   unsaved_changes;
    char*      file_name;
    char*      file_location;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_WINDOW)

void
marker_editor_window_set_file_name(MarkerEditorWindow* self,
                                   char*               filepath)
{
    free(self->file_name);
    free(self->file_location);
    
    int last_slash = marker_utils_rfind('/', filepath);
    
    self->file_location = malloc(last_slash + 2);
    memset(self->file_location, 0, last_slash + 2);
    memcpy(self->file_location, filepath, last_slash + 1);
    
    char* name = &filepath[last_slash + 1];
    int name_len = strlen(name);
    self->file_name = malloc(name_len + 1);
    memset(self->file_name, 0, name_len + 1);
    memcpy(self->file_name, name, name_len);
}

void
marker_editor_window_refresh_web_view(MarkerEditorWindow* self)
{
    GtkTextBuffer* buffer = NULL;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    char uri[50];
    char cwd[50];
    gchar* buffer_text;
    FILE* fp;
    
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

void
marker_editor_window_open_file(MarkerEditorWindow* self,
                               char*               filepath)
{
    FILE*          fp;
    long int       file_size;
    char*          file_contents;
    int            last_slash;
    char*          filename;
    GtkTextBuffer* buffer;

    self->unsaved_changes = FALSE;

    marker_editor_window_set_file_name(self, filepath);

    fp = fopen(filepath, "r");
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    file_contents = malloc(file_size);
    memset(file_contents, 0, file_size);
    fread(file_contents, file_size, 1, fp);
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
    gtk_text_buffer_set_text(buffer, file_contents, file_size);
    gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), self->file_name);
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(self->header_bar), self->file_location);
    marker_editor_window_refresh_web_view(self);
    
    free(file_contents);
}

void
marker_editor_window_save_file_as(MarkerEditorWindow* self,
                                  char*               filepath)
{
    GtkTextBuffer* buffer;
    GtkTextIter    start_iter;
    GtkTextIter    end_iter;
    FILE*          fp;
    gchar*         buffer_text;
    size_t         buffer_size;
    int            last_slash;
    char*          filename;
    
    self->unsaved_changes = FALSE;
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start_iter);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end_iter);
    
    buffer_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),
                                           &start_iter,
                                           &end_iter,
                                           FALSE);
    
    marker_editor_window_set_file_name(self, filepath);
    gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), self->file_name);
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(self->header_bar), self->file_location);
    
    fp = fopen(filepath, "w");
    buffer_size = strlen(buffer_text);
    fwrite(buffer_text, sizeof(gchar), buffer_size, fp);
    fclose(fp);
}

static void
save_btn_pressed(GtkWidget* widget,
                 gpointer   user_data)
{
    GtkWidget*          dialog;
    MarkerEditorWindow* self;
    GtkFileChooser*     chooser;
    char*               filename;
    gint                response;
    const gchar*        title = NULL;
    const gchar*        subtitle = NULL;
    
    self = user_data;
    
    if (self->file_name && self->file_location)
    {
        size_t name_len = strlen(self->file_name);
        size_t location_len = strlen(self->file_location);
        size_t filepath_len = name_len + location_len + 1;
        char filepath[filepath_len];
        memset(filepath, 0, filepath_len);
        strcat(filepath, self->file_location);
        strcat(filepath, self->file_name);
        marker_editor_window_save_file_as(self, filepath);
    }
    else
    {
        dialog = gtk_file_chooser_dialog_new("Open File",
                                             user_data,
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             "Cancel",
                                             GTK_RESPONSE_CANCEL,
                                             "Save",
                                             GTK_RESPONSE_ACCEPT,
                                             NULL);
        
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
        
        response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_ACCEPT)
        {
            chooser = GTK_FILE_CHOOSER (dialog);
            filename = gtk_file_chooser_get_filename (chooser);
            
            marker_editor_window_save_file_as(self, filename);
            
            g_free(filename);
        }
        
        gtk_widget_destroy(dialog);
    }
}

static void
open_btn_pressed(GtkWidget* widget,
                 gpointer   user_data)
{
    GtkWidget*          dialog;
    MarkerEditorWindow* self;
    GtkFileChooser*     chooser;
    char*               filename;
    gint                response;
    
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
        
        marker_editor_window_open_file(self, filename);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void
refresh_btn_pressed(GtkWidget* widget,
                    gpointer   user_data)
{
    MarkerEditorWindow* self;
    self = user_data;
    marker_editor_window_refresh_web_view(self);
}

static void
source_buffer_changed(GtkTextBuffer* buffer,
                      gpointer       user_data)
{
    MarkerEditorWindow* self = user_data;
    
    if (!self->unsaved_changes)
    {
        self->unsaved_changes = TRUE;
        
        if (self->file_name)
        {
            int len = strlen(self->file_name);
            char title[len + 2];
            memset(title, 0, len + 2);
            strcat(title, "*");
            strcat(title, self->file_name);
            gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), title);
        }
        else
        {
            gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), "*Untitled.md");
            gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(self->header_bar), FALSE);
        }
    }
}

static gboolean
auto_refresh(gpointer user_data)
{
    MarkerEditorWindow* self = user_data;
    marker_editor_window_refresh_web_view(self);
    return TRUE;
}

static void
menu_btn_toggled(GtkToggleButton* button,
                 GtkWidget*       popover)
{
    gtk_widget_set_visible(popover, gtk_toggle_button_get_active(button));
}

static void
menu_popover_closed(GtkPopover*      popover,
                    GtkToggleButton* toggle_btn)
{
    gtk_toggle_button_set_active(toggle_btn, FALSE);
}

static void
marker_editor_window_init(MarkerEditorWindow* self)
{

    /*
    self->file_name = NULL;
    self->file_location = NULL;
    self->unsaved_changes = FALSE;

    GtkWidget* refresh_btn;
    GtkWidget* open_btn;
    GtkWidget* save_btn;
    GtkWidget* menu_btn;
    GtkWidget* scrolled_window;
    GtkWidget* icon;
    GtkWidget* menu_popover;
    GtkWidget* popover_box;
    GtkWidget* widget;
    GtkBuilder* builder;
    GError*     err;
    GtkSourceLanguageManager* source_language_manager;
    GtkSourceLanguage* source_language;
    GtkSourceBuffer* source_buffer;

    

    err = NULL;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../src/ui/marker-editor-window.ui", &err);
    if (err)
    {
        puts(err->message);
    }

    refresh_btn = gtk_button_new_from_icon_name("view-refresh-symbolic",
                                                GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(refresh_btn, "refresh markdown preview");
    open_btn = gtk_button_new_from_icon_name("document-open-symbolic",
                                             GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(open_btn, "open a document");
    save_btn = gtk_button_new_from_icon_name("document-save-symbolic",
                                             GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(save_btn, "save the current document");
    menu_btn = gtk_toggle_button_new();
    icon = gtk_image_new_from_icon_name("open-menu-symbolic",
                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_button_set_image(GTK_BUTTON(menu_btn), icon);
    
    menu_popover = gtk_popover_new(menu_btn);
    gtk_popover_set_modal(GTK_POPOVER(menu_popover), TRUE);
    gtk_popover_set_position(GTK_POPOVER(menu_popover), GTK_POS_BOTTOM);
    widget = gtk_menu_item_new_with_label("Open...");
    popover_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(menu_popover), popover_box);
    gtk_box_pack_start(GTK_BOX(popover_box), widget, TRUE, TRUE, 10);
    gtk_widget_show_all(GTK_WIDGET(popover_box));
    g_signal_connect(menu_btn, "toggled", G_CALLBACK(menu_btn_toggled), menu_popover);
    g_signal_connect(menu_popover, "closed", G_CALLBACK(menu_popover_closed), menu_btn);
    
    source_language_manager = gtk_source_language_manager_get_default();
    source_language = gtk_source_language_manager_get_language(source_language_manager, "markdown");
    source_buffer = gtk_source_buffer_new_with_language(source_language);
    g_signal_connect(source_buffer, "changed", G_CALLBACK(source_buffer_changed), self);

    self->header_bar = gtk_header_bar_new();
    self->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    self->source_view = gtk_source_view_new_with_buffer(source_buffer);
    self->web_view = webkit_web_view_new();
    
    g_signal_connect(open_btn, "clicked", G_CALLBACK(open_btn_pressed), self);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(refresh_btn_pressed), self);
    g_signal_connect(save_btn, "clicked", G_CALLBACK(save_btn_pressed), self);
    
    gtk_window_set_default_size(GTK_WINDOW(self), 800, 700);
    gtk_window_set_titlebar(GTK_WINDOW(self), self->header_bar);
    
    gtk_header_bar_set_title(GTK_HEADER_BAR(self->header_bar), "Untitled.md");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(self->header_bar), TRUE);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(self->header_bar), open_btn);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(self->header_bar), save_btn);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(self->header_bar), menu_btn);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(self->header_bar), refresh_btn);
    
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), self->source_view);
    gtk_paned_add1(GTK_PANED(self->paned), scrolled_window);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), self->web_view);
    gtk_paned_add2(GTK_PANED(self->paned), scrolled_window);
    
    gtk_container_add(GTK_CONTAINER(self), self->paned);
    
    g_timeout_add_seconds(1, auto_refresh, self);
    */
    
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void
marker_editor_window_class_init(MarkerEditorWindowClass* class)
{
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),"/com/github/fabiocolacio/marker/marker-editor-window.ui");
}

MarkerEditorWindow*
marker_editor_window_new(void)
{
    return g_object_new(MARKER_TYPE_EDITOR_WINDOW, NULL);
}

