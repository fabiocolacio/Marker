#include <gtksourceview/gtksource.h>
#include <webkit/webkitwebview.h>
#include <wkhtmltox/pdf.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "marker-markdown.h"
#include "marker-editor-window.h"
#include "marker-utils.h"
#include "marker-prefs.h"

struct _MarkerEditorWindow
{
  GtkApplicationWindow  parent_instance;
    
  GtkWidget* header_bar;
  GtkWidget* paned;
  GtkWidget* source_view;
  GtkWidget* web_view_scroll;
  GtkWidget* web_view;
  GtkWidget* web_window;
  GtkWidget* popover;
    
  gboolean   split_view;
    
  gboolean   unsaved_changes;
  char*      file_name;
  char*      file_location;
  GFile*     file;
    
  char*      stylesheet_name;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_APPLICATION_WINDOW)

#define TMP_MD   ".marker_tmp.md"
#define TMP_HTML ".marker_tmp.html"

void
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
  if (response == GTK_RESPONSE_OK)
  {
    gtk_widget_destroy(GTK_WIDGET(window));
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
  
  char* html;
  if (self->stylesheet_name)
  {
    char href[strlen(STYLES_DIR) + strlen(self->stylesheet_name) + 1];
    memset(href, 0, sizeof(href));
    strcat(href, STYLES_DIR);
    strcat(href, self->stylesheet_name);
    html = marker_markdown_render_with_css(buffer_text, strlen(buffer_text), href);
  }
  else
  {
    html = marker_markdown_render(buffer_text, strlen(buffer_text));
  }
  webkit_web_view_load_string(WEBKIT_WEB_VIEW(self->web_view),
                              html,
                              NULL,
                              NULL,
                              "file://");
  g_signal_connect(self->web_view, "load-progress-changed", G_CALLBACK(web_view_load_event), NULL);
  
  free(html);
  g_free(buffer_text);
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

static void
save_as_activated(GSimpleAction* action,
                  GVariant*      parameter,
                  gpointer       data)
{   
  MarkerEditorWindow* self = data;
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
    save_as_activated(NULL, NULL, self);
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

void
marker_editor_window_export_file_as(MarkerEditorWindow*  self,
                                    GFile*               file,
                                    MarkerExportSettings settings)
{
  if (G_IS_FILE(file))
  {
    char* filepath_dirty = g_file_get_path(file);
    char* filepath = marker_utils_escape_file_path(filepath_dirty);
    int slash = marker_utils_rfind('/', filepath_dirty);
    char loc[slash + 1];
    memset(loc, 0, sizeof(loc));
    memcpy(loc, filepath_dirty, slash);
    int ret = chdir(loc);
    if (!ret)
    {
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

      char command[256] = "pandoc -s -c ";
      strcat(command, settings.style_sheet);
      
      switch (settings.file_type)
      {
        case HTML:
          strcat(command, " -o ");
          strcat(command, filepath);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "html");
          ret = system(command);
          break;
              
        case RTF:
          strcat(command, " -o ");
          strcat(command, filepath);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "rtf");
          ret = system(command);
          break;
              
        case EPUB:
          strcat(command, " -o ");
          strcat(command, filepath);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "epub");
          ret = system(command);
          break;
              
        case ODT:
          strcat(command, " -o ");
          strcat(command, filepath);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "odt");
          ret = system(command);
          break;
              
        case DOCX:
          strcat(command, " -o ");
          strcat(command, filepath);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "docx");
          ret = system(command);
          break;
              
        case LATEX:
          strcat(command, " -o ");
          strcat(command, filepath);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "latex");
          ret = system(command);
          break;
          
        case PDF:
          strcat(command, " -o ");
          strcat(command, TMP_HTML);
          strcat(command, " ");
          strcat(command, TMP_MD);
          strcat(command, " -t ");
          strcat(command, "html");
          ret = system(command);
          
          wkhtmltopdf_global_settings* gs;
          wkhtmltopdf_object_settings* os;
          wkhtmltopdf_converter* c;
          wkhtmltopdf_init(false);
          gs = wkhtmltopdf_create_global_settings();
          wkhtmltopdf_set_global_setting(gs, "out", filepath_dirty);
          os = wkhtmltopdf_create_object_settings();
          wkhtmltopdf_set_object_setting(os, "page", TMP_HTML);
          c = wkhtmltopdf_create_converter(gs);
          wkhtmltopdf_add_object(c, os, NULL);
          ret = wkhtmltopdf_convert(c);
          wkhtmltopdf_destroy_global_settings(gs);
          wkhtmltopdf_destroy_object_settings(os);
          wkhtmltopdf_destroy_converter(c);
          wkhtmltopdf_deinit();
          remove(TMP_HTML);
          break;
      }
    }
    g_free(filepath_dirty);
    free(filepath);
    remove(TMP_MD);
  }
}

static void
export_location_btn_pressed(GtkButton* btn,
                            GtkWindow* win)
{
  GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
                                                  win,
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
    gtk_button_set_label(btn, filename);
    
    g_free(filename);
  }
    
  gtk_widget_destroy(dialog);
}

static void
export_activated(GSimpleAction* action,
                 GVariant*      parameter,
                 gpointer       data)
{
  MarkerEditorWindow* self = data;
  GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-export-dialog.ui");
  GtkDialog* export_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "export_dialog"));
  gtk_window_set_transient_for(GTK_WINDOW(export_dialog), GTK_WINDOW(self));
  GtkButton* loc_btn = GTK_BUTTON(gtk_builder_get_object(builder, "export_location_btn"));
  gtk_builder_add_callback_symbol(builder, "export_location_btn_pressed", G_CALLBACK(export_location_btn_pressed));
  gtk_builder_connect_signals(builder, export_dialog);
  
  GtkComboBox* format_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "format_chooser"));
  GtkListStore* export_format_model = GTK_LIST_STORE(gtk_builder_get_object(builder, "export_format_model"));
  marker_utils_combo_box_set_model(format_chooser, GTK_TREE_MODEL(export_format_model));
  
  gtk_widget_show_all(GTK_WIDGET(format_chooser));
  g_object_unref(builder);
  
  gint ret = gtk_dialog_run(export_dialog);
  if (ret == GTK_RESPONSE_OK)
  {
    MarkerExportSettings settings;
    settings.file_type = gtk_combo_box_get_active(format_chooser);
    char style_sheet[strlen(STYLES_DIR) + strlen(self->stylesheet_name) + 1];
    memset(style_sheet, 0, sizeof(style_sheet));
    strcat(style_sheet, STYLES_DIR);
    strcat(style_sheet, self->stylesheet_name);
    settings.style_sheet = style_sheet;
    const gchar* filename = gtk_button_get_label(loc_btn);
    GFile* file = g_file_new_for_path(filename);
    marker_editor_window_export_file_as(self, file, settings);
  }
    
  gtk_widget_destroy(GTK_WIDGET(export_dialog));
}

static void
new_activated(GSimpleAction* action,
              GVariant*      parameter,
              gpointer       data)
{
  MarkerEditorWindow* self = data;
  GtkApplication* app;
  g_object_get(self, "application", &app, NULL);
  MarkerEditorWindow* win = marker_editor_window_new(app);
  gtk_widget_show_all(GTK_WIDGET(win));
  g_object_unref(app);
}

static gboolean
preview_window_closed(GtkWidget* window,
                      GdkEvent*  event,
                      gpointer   user_data)
{
  MarkerEditorWindow* marker_window = user_data;
  g_object_ref(marker_window->web_view_scroll);
  gtk_container_remove(GTK_CONTAINER(window), marker_window->web_view_scroll);
  gtk_paned_add2(GTK_PANED(marker_window->paned), marker_window->web_view_scroll); 
  gtk_widget_destroy(window);
  return TRUE;
}

static void
popout_btn_pressed(GtkButton*          button,
                   MarkerEditorWindow* window)
{
  GtkWidget* image;
  if (window->split_view)
  {
    image = gtk_image_new_from_icon_name("view-paged-symbolic",
                                         GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_button_set_image(button, image);
    
    if (window->web_window)
    {
      preview_window_closed(window->web_window, NULL, window);
      window->web_window = NULL;
    }
  }
  else
  {
    image = gtk_image_new_from_icon_name("view-dual-symbolic",
                                         GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_button_set_image(button, image);
    
    g_object_ref(window->web_view_scroll);
    gtk_container_remove(GTK_CONTAINER(window->paned), window->web_view_scroll);
    GtkWidget* prev_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    const gchar* title = gtk_header_bar_get_title(GTK_HEADER_BAR(window->header_bar)); 
    gtk_window_set_title(GTK_WINDOW(prev_win), title);
    gtk_window_set_default_size(GTK_WINDOW(prev_win), 400, 600);
    window->web_window = prev_win;
    gtk_container_add(GTK_CONTAINER(prev_win), window->web_view_scroll);
    gtk_widget_show_all(GTK_WIDGET(prev_win));
    g_signal_connect(prev_win, "delete-event", G_CALLBACK(preview_window_closed), window);
  }
  window->split_view = !window->split_view;
}

static void
refresh_btn_pressed(GtkWidget*          widget,
                    MarkerEditorWindow* self)
{
  marker_editor_window_refresh_web_view(self);
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
}

void
marker_editor_window_try_close(MarkerEditorWindow* self)
{
  if (self->unsaved_changes)
  {
    show_unsaved_documents_warning(GTK_WINDOW(self));
  }
  else
  {
    gtk_widget_destroy(GTK_WIDGET(self));
  }
}

static gboolean
close_btn_pressed(MarkerEditorWindow* self,
                  gpointer*           user_data)
{
  marker_editor_window_try_close(self);
  return TRUE;
}

static gboolean
modifier_pressed(GdkEventKey     event,
                 GdkModifierType modifier)
{
  return (event.state & modifier) == modifier;
}
                 

static gboolean
key_pressed(GtkWidget*   widget,
            GdkEventKey* event,
            gpointer     user_data)
{
  gboolean ctrl_pressed = modifier_pressed(*event, GDK_CONTROL_MASK);
  
  if (ctrl_pressed)
  {
    switch (event->keyval)
    {
      case GDK_KEY_s:
        save_btn_pressed(widget, MARKER_EDITOR_WINDOW(widget));
        break;
        
      case GDK_KEY_o:
        open_btn_pressed(widget, MARKER_EDITOR_WINDOW(widget));
        break;
        
      case GDK_KEY_S:
        save_as_activated(NULL, NULL, MARKER_EDITOR_WINDOW(widget));
        break;
        
      case GDK_KEY_n:
        new_activated(NULL, NULL, MARKER_EDITOR_WINDOW(widget));
        break;
        
      case GDK_KEY_e:
        export_activated(NULL, NULL, MARKER_EDITOR_WINDOW(widget));
        break;
        
      case GDK_KEY_r:
        refresh_btn_pressed(widget, MARKER_EDITOR_WINDOW(widget));
        break;
    }
  }
  
  return FALSE;
}


void
marker_editor_window_set_css_theme(MarkerEditorWindow* self,
                                   char*               theme)
{
  free(self->stylesheet_name);
  if (strcmp(theme, "none") == 0)
  {
    self->stylesheet_name = NULL;
  }
  else
  {
    self->stylesheet_name = marker_utils_allocate_string(theme);
  }
  marker_editor_window_refresh_web_view(self);
}

void
marker_editor_window_set_syntax_theme(MarkerEditorWindow* self,
                                      char*               theme)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view));
  GtkSourceStyleSchemeManager* style_manager = gtk_source_style_scheme_manager_get_default();
  GtkSourceStyleScheme* scheme = gtk_source_style_scheme_manager_get_scheme(style_manager, theme);
  gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), scheme);
}

void
marker_editor_window_set_show_line_numbers(MarkerEditorWindow* window,
                                           gboolean            line_nums)
{
  GtkSourceView* source_view = GTK_SOURCE_VIEW(window->source_view);
  if (source_view)
  {
    gtk_source_view_set_show_line_numbers(source_view, line_nums);
  }
}

void                                           
marker_editor_window_set_highlight_current_line(MarkerEditorWindow* window,
                                                gboolean            highlight)
{
  GtkSourceView* source_view = GTK_SOURCE_VIEW(window->source_view);
  if (source_view)
  {
    gtk_source_view_set_highlight_current_line(source_view, highlight);
  }
}

void                                           
marker_editor_window_set_show_right_margin(MarkerEditorWindow* window,
                                           gboolean            margin)
{
  GtkSourceView* source_view = GTK_SOURCE_VIEW(window->source_view);
  if (source_view)
  {
    gtk_source_view_set_show_right_margin(source_view, margin);
  }
}

void                                           
marker_editor_window_set_wrap_text(MarkerEditorWindow* window,
                                   gboolean            wrap)
{
  GtkTextView* source_view = GTK_TEXT_VIEW(window->source_view);
  if (source_view)
  {
    GtkWrapMode mode = (wrap) ? GTK_WRAP_WORD : GTK_WRAP_NONE;
    gtk_text_view_set_wrap_mode(source_view, mode);
  }
}

void
marker_editor_window_apply_prefs(MarkerEditorWindow* win)
{
  char* tmp;
  
  tmp = marker_prefs_get_syntax_theme();
  marker_editor_window_set_syntax_theme(win, tmp);
  free(tmp);
  
  tmp = marker_prefs_get_css_theme();
  marker_editor_window_set_css_theme(win, tmp);
  free(tmp);
  
  marker_editor_window_set_show_line_numbers(win, marker_prefs_get_show_line_numbers());
  marker_editor_window_set_highlight_current_line(win, marker_prefs_get_highlight_current_line());
  marker_editor_window_set_show_right_margin(win, marker_prefs_get_show_right_margin());
  marker_editor_window_set_wrap_text(win, marker_prefs_get_wrap_text());
}

static GActionEntry win_entries[] =
{
  { "saveas", save_as_activated, NULL, NULL, NULL },
  { "export", export_activated, NULL, NULL, NULL },
  { "new", new_activated, NULL, NULL, NULL }
};

static void
marker_editor_window_init(MarkerEditorWindow* self)
{   
  self->file = NULL;
  self->unsaved_changes = FALSE;
  self->stylesheet_name = marker_utils_allocate_string("marker.css");
  
  GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-editor-window.ui");
  
  GtkSourceLanguageManager* source_language_manager = gtk_source_language_manager_get_default();
  GtkSourceLanguage* source_language = gtk_source_language_manager_get_language(source_language_manager, "markdown");
  GtkSourceBuffer* source_buffer = gtk_source_buffer_new_with_language(source_language);
  g_object_unref(source_language_manager);
  
  GtkWidget* widget;
  
  self->paned = GTK_WIDGET(gtk_builder_get_object(builder, "editor"));
  self->header_bar = GTK_WIDGET(gtk_builder_get_object(builder, "header_bar"));
  self->source_view = GTK_WIDGET(gtk_builder_get_object(builder, "source_view"));
  self->web_view_scroll = GTK_WIDGET(gtk_builder_get_object(builder, "web_view_scroll"));
  self->web_view = GTK_WIDGET(gtk_builder_get_object(builder, "web_view"));
  
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(self->source_view), TRUE);
  
  gtk_widget_grab_focus(GTK_WIDGET(self->source_view));
  
  gtk_text_view_set_buffer(GTK_TEXT_VIEW(self->source_view), GTK_TEXT_BUFFER(source_buffer));
  g_signal_connect(source_buffer, "changed", G_CALLBACK(source_buffer_changed), self);
  
  GtkMenuButton* menu_btn = GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn"));
  GMenuModel* gear_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "gear_menu"));
  gtk_menu_button_set_use_popover(menu_btn, TRUE);
  gtk_menu_button_set_menu_model(menu_btn, gear_menu);
  g_action_map_add_action_entries(G_ACTION_MAP(self),
                                  win_entries,
                                  G_N_ELEMENTS(win_entries),
                                  self);
  
  widget = GTK_WIDGET(gtk_builder_get_object(builder, "editor"));
  gtk_container_add(GTK_CONTAINER(self), widget);
  gtk_window_set_titlebar(GTK_WINDOW(self), self->header_bar);
  
  gtk_window_set_default_size(GTK_WINDOW(self), 800, 500);
  
  g_signal_connect(self, "delete-event", G_CALLBACK(close_btn_pressed), NULL);
  g_signal_connect(self, "key-press-event", G_CALLBACK(key_pressed), NULL);
  
  gtk_builder_add_callback_symbol(builder, "open_btn_pressed", G_CALLBACK(open_btn_pressed));
  gtk_builder_add_callback_symbol(builder, "save_btn_pressed", G_CALLBACK(save_btn_pressed));
  gtk_builder_add_callback_symbol(builder, "refresh_btn_pressed", G_CALLBACK(refresh_btn_pressed));
  gtk_builder_add_callback_symbol(builder, "popout_btn_pressed", G_CALLBACK(popout_btn_pressed));
  gtk_builder_connect_signals(builder, self);
  
  g_object_unref(builder);
  
  marker_editor_window_apply_prefs(self);
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

