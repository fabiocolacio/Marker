#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>

#include <stdlib.h>
#include <string.h>

#include "marker.h"
#include "marker-prefs.h"
#include "marker-string.h"
#include "marker-source-view.h"
#include "marker-markdown.h"

#include "marker-editor-window.h"

struct _MarkerEditorWindow
{
  GtkApplicationWindow parent_instance;
  
  MarkerSourceView* source_view;
  WebKitWebView* web_view;
  
  GtkHeaderBar* header_bar;
  
  GFile* file;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_APPLICATION_WINDOW)

static void
save_as_cb(GSimpleAction* action,
           GVariant*      parameter,
           gpointer       user_data)
{
  MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(user_data);
  GtkWidget* dialog = gtk_file_chooser_dialog_new("Save File",
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
export_cb(GSimpleAction* action,
          GVariant*      parameter,
          gpointer       user_data)
{

}

static void
new_cb(GSimpleAction* action,
       GVariant*      parameter,
       gpointer       user_data)
{
  marker_create_new_window();
}

static GActionEntry win_entries[] =
{
  { "saveas", save_as_cb, NULL, NULL, NULL },
  { "export", export_cb, NULL, NULL, NULL },
  { "new", new_cb, NULL, NULL, NULL }
};

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
  if (response == GTK_RESPONSE_OK)
  {
    gtk_widget_destroy(GTK_WIDGET(window));
  }
  gtk_widget_destroy(dialog);
}

void
marker_editor_window_try_close(MarkerEditorWindow* window)
{
  if (marker_source_view_get_modified(window->source_view))
  {
    show_unsaved_documents_warning(GTK_WINDOW(window));
  }
  else
  {
    gtk_widget_destroy(GTK_WIDGET(window));
  }
}

static gboolean
close_btn_pressed(MarkerEditorWindow* window,
                  gpointer*           user_data)
{
  marker_editor_window_try_close(window);
  return TRUE;
}

void
marker_editor_window_refresh_preview(MarkerEditorWindow* window)
{
  WebKitWebView* web_view = window->web_view;
  gchar* markdown = marker_editor_window_get_markdown(window);
  const char* css_theme = marker_prefs_get_css_theme();
  char* html = (css_theme)
    ? marker_markdown_to_html_with_css(markdown, strlen(markdown), css_theme)
    : marker_markdown_to_html(markdown, strlen(markdown));
  
  gchar* uri = NULL;
  if (G_IS_FILE(window->file)) { uri = g_file_get_uri(window->file); }
  
  webkit_web_view_load_html(web_view,
                            html,
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
    
    marker_source_view_set_text(window->source_view, file_contents, file_size);    
    g_free(file_contents);

    marker_source_view_set_modified(window->source_view, FALSE);
    char* filepath = g_file_get_path(file);
    marker_editor_window_set_title_filename(window);
    g_free(filepath);
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
    marker_editor_window_set_title_filename(window);
    marker_source_view_set_modified(window->source_view, FALSE);
  }
  g_free(filepath);
}

void
marker_editor_window_set_title_filename(MarkerEditorWindow* window)
{
  if (G_IS_FILE(window->file))
  {
    char* filepath = g_file_get_path(window->file);
    if (marker_prefs_get_client_side_decorations())
    {
      char* filename = marker_string_rfind(filepath, "/");
      *filename = '\0'; ++filename;
      gtk_header_bar_set_title(window->header_bar, filename);
      gtk_header_bar_set_subtitle(window->header_bar, filepath);
    }
    else
    {
      gtk_window_set_title(GTK_WINDOW(window), filepath);
    }
    g_free(filepath);
  }
  else
  {
    if (marker_prefs_get_client_side_decorations())
    {
      gtk_header_bar_set_title(window->header_bar, "Untitled.md");
      gtk_header_bar_set_has_subtitle(window->header_bar, FALSE);
    }
    else
    {
      gtk_window_set_title(GTK_WINDOW(window), "Untitled.md");
    }
  }
}

void
marker_editor_window_set_title_filename_unsaved(MarkerEditorWindow* window)
{
  if (G_IS_FILE(window->file))
  {
    char* filepath = g_file_get_path(window->file);
    if (marker_prefs_get_client_side_decorations())
    {
      char* filename = marker_string_rfind(filepath, "/");
      *filename = '\0'; ++filename;
      char buf[strlen(filename) + 1];
      marker_string_prepend(filename, "*", buf, sizeof(buf));
      gtk_header_bar_set_title(window->header_bar, buf);
      gtk_header_bar_set_subtitle(window->header_bar, filepath);
    }
    else
    {
      char buf[strlen(filepath) + 1];
      marker_string_prepend(filepath, "*", buf, sizeof(buf));
      gtk_window_set_title(GTK_WINDOW(window), buf);
    }
    g_free(filepath);
  }
  else
  {
    if (marker_prefs_get_client_side_decorations())
    {
      gtk_header_bar_set_title(window->header_bar, "*Untitled.md");
      gtk_header_bar_set_has_subtitle(window->header_bar, FALSE);
    }
    else
    {
      gtk_window_set_title(GTK_WINDOW(window), "*Untitled.md");
    }
  }
}

void
marker_editor_window_set_syntax_theme(MarkerEditorWindow* window,
                                      const char*         theme)
{
  marker_source_view_set_syntax_theme(window->source_view, theme);
}

void
marker_editor_window_set_show_line_numbers(MarkerEditorWindow* window,
                                           gboolean            state)
{
  gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(window->source_view),
                                        state);
}
                                           
void
marker_editor_window_set_highlight_current_line(MarkerEditorWindow* window,
                                                gboolean            state)
{
  gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(window->source_view),
                                             state);
}

void
marker_editor_window_set_wrap_text(MarkerEditorWindow* window,
                                   gboolean            state)
{
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(window->source_view),
                              GTK_WRAP_WORD);
}

void
marker_editor_window_set_show_right_margin(MarkerEditorWindow* window,
                                           gboolean            state)
{
  gtk_source_view_set_show_right_margin(GTK_SOURCE_VIEW(window->source_view),
                                        state);
}

void
marker_editor_window_apply_prefs(MarkerEditorWindow* window)
{
  marker_editor_window_set_syntax_theme(window, marker_prefs_get_syntax_theme());
  marker_editor_window_set_show_right_margin(window, marker_prefs_get_show_right_margin());
  marker_editor_window_set_wrap_text(window, marker_prefs_get_wrap_text());
  marker_editor_window_set_highlight_current_line(window, marker_prefs_get_highlight_current_line());
  marker_editor_window_set_show_line_numbers(window, marker_prefs_get_show_line_numbers());
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
save_cb(GtkWidget*          widget,
        MarkerEditorWindow* window)
{
  if (G_IS_FILE(window->file))
  {
    marker_editor_window_save_file(window, window->file);
  }
  else
  {
    save_as_cb(NULL, NULL, window);
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
  marker_editor_window_set_title_filename_unsaved(window);
  marker_editor_window_refresh_preview(window);
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
  MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(widget);
  if (ctrl_pressed)
  {
    switch (event->keyval)
    {
      case GDK_KEY_s:
        save_cb(widget, window);
        break;
        
      case GDK_KEY_o:
        open_cb(widget, window);
        break;
        
      case GDK_KEY_S:
        save_as_cb(NULL, NULL, window);
        break;
        
      case GDK_KEY_n:
        new_cb(NULL, NULL, window);
        break;
        
      case GDK_KEY_e:
        export_cb(NULL, NULL, window);
        break;
        
      case GDK_KEY_r:
        marker_editor_window_refresh_preview(window);
        break;
      
      case GDK_KEY_b:
        marker_source_view_surround_selection_with(window->source_view, "**");
        break;
       
      case GDK_KEY_i:
        marker_source_view_surround_selection_with(window->source_view, "*");
        break;
        
      case GDK_KEY_m:
        marker_source_view_surround_selection_with(window->source_view, "``");
        break;
    }
  }
  
  marker_editor_window_refresh_preview(window);
  return FALSE;
}

static void
init_ui(MarkerEditorWindow* window)
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/ui/editor-window.ui");

  GtkBox* vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));

  // Tool Bar //
  GtkHeaderBar* header_bar =
    GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));
  window->header_bar = header_bar;
  if (marker_prefs_get_client_side_decorations())
  {
    gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(header_bar));
    gtk_header_bar_set_show_close_button(header_bar, TRUE);
  }
  else
  {
    gtk_box_pack_start(vbox, GTK_WIDGET(header_bar), FALSE, TRUE, 0);
  }
  
  GtkMenuButton* menu_btn =
    GTK_MENU_BUTTON(gtk_builder_get_object(builder, "menu_btn"));  
  
  if (marker_has_app_menu())
  {
    GMenuModel* gear_menu =
      G_MENU_MODEL(gtk_builder_get_object(builder, "gear_menu"));  
    gtk_menu_button_set_use_popover(menu_btn, TRUE);
    gtk_menu_button_set_menu_model(menu_btn, gear_menu);
    g_action_map_add_action_entries(G_ACTION_MAP(window),
                                    win_entries,
                                    G_N_ELEMENTS(win_entries),
                                    window);
  }
  else
  {
    GMenuModel* gear_menu =
      G_MENU_MODEL(gtk_builder_get_object(builder, "gear_menu_full"));  
    gtk_menu_button_set_use_popover(menu_btn, TRUE);
    gtk_menu_button_set_menu_model(menu_btn, gear_menu);
    g_action_map_add_action_entries(G_ACTION_MAP(window),
                                    win_entries,
                                    G_N_ELEMENTS(win_entries),
                                    window);
    GtkApplication* app = marker_get_app();
    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                    APP_MENU_ACTION_ENTRIES,
                                    3,
                                    window);
  }
  
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
  
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_signal_connect(window, "delete-event", G_CALLBACK(close_btn_pressed), window);
  g_signal_connect(window, "key-press-event", G_CALLBACK(key_pressed), NULL);
  marker_editor_window_set_title_filename(window);
  
  marker_editor_window_refresh_preview(window);
  
  gtk_builder_add_callback_symbol(builder, "open_cb", G_CALLBACK(open_cb));
  gtk_builder_add_callback_symbol(builder, "save_cb", G_CALLBACK(save_cb));
  gtk_builder_connect_signals(builder, window);
  
  g_object_unref(builder);
}

static void
marker_editor_window_init(MarkerEditorWindow* window)
{
  init_ui(window);
  marker_editor_window_apply_prefs(window);
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

