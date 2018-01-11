#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>
#include <gtkspell/gtkspell.h>

#include <stdlib.h>
#include <string.h>

#include "marker.h"
#include "marker-prefs.h"
#include "marker-string.h"
#include "marker-source-view.h"
#include "marker-preview.h"
#include "marker-markdown.h"
#include "marker-exporter.h"

#include "marker-editor-window.h"

struct _MarkerEditorWindow
{
  GtkApplicationWindow parent_instance;
  
  GtkBox                     *header_box;
  GtkHeaderBar               *header_bar;
  GtkButton                  *unfullscreen_btn;
  GtkPaned                   *paned;
  MarkerSourceView           *source_view;
  GtkWidget                  *source_scroll;
  MarkerPreview              *web_view;
  GtkBox                     *vbox;
  
  GFile                      *file;
  
  MarkerEditorWindowViewMode  view_mode;
  gboolean                    is_fullscreen;
};

G_DEFINE_TYPE(MarkerEditorWindow, marker_editor_window, GTK_TYPE_APPLICATION_WINDOW)

static void
print_cb(GSimpleAction* action,
         GVariant*      parameter,
         gpointer       user_data)
{
  MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(user_data);
  marker_preview_run_print_dialog(window->web_view, GTK_WINDOW(window));
}

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
  MarkerEditorWindow* window = user_data;
  marker_exporter_show_export_dialog(window);
}

static void
new_cb(GSimpleAction* action,
       GVariant*      parameter,
       gpointer       user_data)
{
  marker_create_new_window();
}

static void
editoronlymode_cb(GSimpleAction* action,
                  GVariant*      parameter,
                  gpointer       user_data)
{
  MarkerEditorWindow* window = user_data;
  marker_editor_window_set_view_mode(window, EDITOR_ONLY_MODE);
}

static void
previewonlymode_cb(GSimpleAction* action,
                   GVariant*      parameter,
                   gpointer       user_data)
{
  MarkerEditorWindow* window = user_data;
  marker_editor_window_set_view_mode(window, PREVIEW_ONLY_MODE);
}

static void
dualpanemode_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data)
{
  MarkerEditorWindow* window = user_data;
  marker_editor_window_set_view_mode(window, DUAL_PANE_MODE);
}

static void
dualwindowmode_cb(GSimpleAction* action,
                  GVariant*      parameter,
                  gpointer       user_data)
{
  MarkerEditorWindow* window = user_data;
  marker_editor_window_set_view_mode(window, DUAL_WINDOW_MODE);
}

static GActionEntry win_entries[] =
{
  { "saveas", save_as_cb, NULL, NULL, NULL },
  { "export", export_cb, NULL, NULL, NULL },
  { "print", print_cb, NULL, NULL, NULL },
  { "new", new_cb, NULL, NULL, NULL },
  { "editoronlymode", editoronlymode_cb, NULL, NULL, NULL },
  { "previewonlymode", previewonlymode_cb, NULL, NULL, NULL },
  { "dualpanemode", dualpanemode_cb, NULL, NULL, NULL },
  { "dualwindowmode", dualwindowmode_cb, NULL, NULL, NULL }
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
                  gpointer            user_data)
{
  marker_editor_window_try_close(window);
  return TRUE;
}

static gboolean
preview_window_closed(GtkWindow* preview_window,
                      GdkEvent*  event,
                      gpointer   user_data)
{
  GtkWidget* web_view = (GtkWidget*) user_data;
  g_object_ref(web_view);
  gtk_container_remove(GTK_CONTAINER(preview_window), web_view);
  gtk_widget_destroy(GTK_WIDGET(preview_window));
  return TRUE;
}

void
marker_editor_window_set_view_mode(MarkerEditorWindow*        window,
                                   MarkerEditorWindowViewMode mode)
{
  window->view_mode = mode;
  
  GtkWidget* const paned = GTK_WIDGET(window->paned);
  GtkWidget* const source_scroll = window->source_scroll;
  GtkWidget* const web_view = GTK_WIDGET(window->web_view);
  
  GtkContainer* source_parent = GTK_CONTAINER(gtk_widget_get_parent(source_scroll));
  if (source_parent)
  {
    g_object_ref(source_scroll);
    gtk_container_remove(source_parent, source_scroll);
  }
  
  GtkContainer* web_parent = GTK_CONTAINER(gtk_widget_get_parent(web_view));
  if (web_parent)
  {
    g_object_ref(web_view);
    gtk_container_remove(web_parent, web_view);
  }
  
  if (GTK_IS_WINDOW(web_parent))
  {
    gtk_widget_destroy(GTK_WIDGET(web_parent));
  }
  
  switch (mode)
  {
    case EDITOR_ONLY_MODE:
      gtk_paned_add1(GTK_PANED(paned), source_scroll);
      break;
      
    case PREVIEW_ONLY_MODE:
      gtk_paned_add2(GTK_PANED(paned), web_view);
      break;
    
    case DUAL_PANE_MODE:
      gtk_paned_add1(GTK_PANED(paned), source_scroll);
      gtk_paned_add2(GTK_PANED(paned), web_view);
      break;
      
    case DUAL_WINDOW_MODE:
      gtk_paned_add1(GTK_PANED(paned), source_scroll);
      
      GtkWindow* preview_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
      g_signal_connect(preview_window, "delete-event", G_CALLBACK(preview_window_closed), web_view);
      gtk_container_add(GTK_CONTAINER(preview_window), web_view);
      gtk_window_set_title(preview_window, "Preview");
      gtk_window_set_default_size(preview_window, 500, 600);
      gtk_widget_show_all(GTK_WIDGET(preview_window));
      break;
  }
}

void
marker_editor_window_set_use_syntax_theme(MarkerEditorWindow* window,
                                          gboolean            state)
{
  GtkSourceBuffer* buffer =
    GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->source_view)));
  gtk_source_buffer_set_highlight_syntax(buffer, state);
}

void
marker_editor_window_refresh_preview(MarkerEditorWindow* window)
{
  gchar* markdown = marker_editor_window_get_markdown(window);
  const char* css_theme = (marker_prefs_get_use_css_theme())
                          ? marker_prefs_get_css_theme()
                          : NULL;


  gchar* uri = (G_IS_FILE(window->file))
               ? g_file_get_uri(window->file)
               : NULL;

  marker_preview_render_markdown(window->web_view, markdown, css_theme, uri);
  
  if (uri)
  {
    g_free(uri);
  }
  
  g_free(markdown);
}

void
marker_editor_window_open_file (MarkerEditorWindow *window,
                                GFile*              file)
{
  char *file_contents = NULL;
  gsize file_size = 0;
  GError* err = NULL;
  
  g_file_load_contents (file, NULL, &file_contents, &file_size, NULL, &err);
  
  if (err)
  {
    g_error("There was a problem opening the file!\n\n%s\n", err->message);
    g_error_free(err);
  }
  else
  {
    window->file = file;
    
    GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (window->source_view)));
    gtk_source_buffer_begin_not_undoable_action (buffer);
    marker_source_view_set_text (window->source_view, file_contents, file_size);
    gtk_source_buffer_end_not_undoable_action (buffer);
    g_free (file_contents);

    marker_source_view_set_modified (window->source_view, FALSE);
    marker_editor_window_set_title_filename (window);
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
    char* name = marker_string_filename_get_name(filepath);
    char* path = marker_string_filename_get_path(filepath);
    
    gtk_header_bar_set_title(window->header_bar, name);
    gtk_header_bar_set_subtitle(window->header_bar, path);
    
    free(name);
    free(path);
    g_free(filepath);
  }
  else
  {
    gtk_header_bar_set_title(window->header_bar, "Untitled.md");
    gtk_header_bar_set_has_subtitle(window->header_bar, FALSE);
  }
}

void
marker_editor_window_set_title_filename_unsaved(MarkerEditorWindow* window)
{
  if (G_IS_FILE(window->file))
  {
    char* filepath = g_file_get_path(window->file);
    char* filename = marker_string_filename_get_name(filepath);
    char buf[strlen(filename) + 1];
    marker_string_prepend(filename, "*", buf, sizeof(buf));
    gtk_header_bar_set_title(window->header_bar, buf);
    gtk_header_bar_set_subtitle(window->header_bar, filepath);
    g_free(filepath);
  }
  else
  {
    gtk_header_bar_set_title(window->header_bar, "*Untitled.md");
    gtk_header_bar_set_has_subtitle(window->header_bar, FALSE);
  }
}

void marker_editor_window_set_replace_tabs(MarkerEditorWindow*  window,
                                           gboolean             state)
{
  gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(window->source_view), state);
}

void marker_editor_window_set_auto_indent(MarkerEditorWindow*  window,
                                           gboolean             state)
{
  gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(window->source_view), state);
}

void marker_editor_window_set_tab_width(MarkerEditorWindow*   window,
                                        guint                 value)
{
  gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(window->source_view), value);
}

void marker_editor_window_set_spell_check(MarkerEditorWindow* window,
                                          gboolean            state)
{
  marker_source_view_set_spell_check(window->source_view, state);
}

void marker_editor_window_set_spell_lang(MarkerEditorWindow* window,
                                    gchar*            lang)
{
  marker_source_view_set_spell_check_lang(window->source_view, lang);
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
  if (state)
  {
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(window->source_view), GTK_WRAP_WORD);
  }
  else
  {
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(window->source_view), GTK_WRAP_NONE);
  }
}

void
marker_editor_window_set_show_right_margin(MarkerEditorWindow* window,
                                           gboolean            state)
{
  gtk_source_view_set_show_right_margin(GTK_SOURCE_VIEW(window->source_view),
                                        state);
}

void
marker_editor_window_set_right_margin_position(MarkerEditorWindow* window,
                                               guint               value)
{
  gtk_source_view_set_right_margin_position(GTK_SOURCE_VIEW(window->source_view), value);
}

void
marker_editor_window_fullscreen (MarkerEditorWindow *window)
{
  window->is_fullscreen = TRUE;

  GtkBox *vbox = window->vbox;
  GtkBox *header_box = window->header_box;
  GtkWidget *header_bar = GTK_WIDGET (window->header_bar);
  GtkWidget *paned = GTK_WIDGET (window->paned);
  
  gtk_window_fullscreen (GTK_WINDOW (window));

  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (header_box), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), FALSE);
  gtk_widget_show (GTK_WIDGET (window->unfullscreen_btn));
  
  g_object_ref (paned);
  gtk_container_remove (GTK_CONTAINER (vbox), paned);
  
  gtk_box_pack_start (GTK_BOX (window->vbox), header_bar, FALSE, TRUE, 0); 
  gtk_box_pack_start (GTK_BOX (window->vbox), paned, TRUE, TRUE, 0);
}

void
marker_editor_window_unfullscreen (MarkerEditorWindow *window)
{
  window->is_fullscreen = FALSE;

  GtkBox *vbox = window->vbox;
  GtkBox *header_box = window->header_box;
  GtkWidget *header_bar = GTK_WIDGET (window->header_bar);

  gtk_window_unfullscreen (GTK_WINDOW (window));
    
  g_object_ref (header_bar);
  gtk_container_remove (GTK_CONTAINER (vbox), header_bar);
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), TRUE);
  gtk_widget_hide (GTK_WIDGET (window->unfullscreen_btn));
  
  gtk_box_pack_start (header_box, header_bar, FALSE, TRUE, 0);
}

void
marker_editor_window_set_fullscreen (MarkerEditorWindow *window,
                                     gboolean            state)
{
  if (state)
    marker_editor_window_fullscreen (window);
  else
    marker_editor_window_unfullscreen (window);
}

gboolean
marker_editor_window_get_is_fullscreen (MarkerEditorWindow *window)
{
  return window->is_fullscreen;
}

void
marker_editor_window_toggle_fullscreen (MarkerEditorWindow *window)
{
  marker_editor_window_set_fullscreen (window, !(window->is_fullscreen));
}

void
marker_editor_window_apply_prefs(MarkerEditorWindow* window)
{
  if (marker_prefs_get_use_syntax_theme())
  {
    marker_editor_window_set_syntax_theme(window, marker_prefs_get_syntax_theme());
  }
  else
  {
    marker_editor_window_set_use_syntax_theme(window, false);
  }
  
  marker_editor_window_set_show_right_margin(window, marker_prefs_get_show_right_margin());
  marker_editor_window_set_right_margin_position(window, marker_prefs_get_right_margin_position());
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

static void
unfullscreen_btn_clicked (GtkButton *button,
                          gpointer   user_data)
{
  marker_editor_window_unfullscreen (MARKER_EDITOR_WINDOW (user_data));
}
                 
static gboolean
key_pressed(GtkWidget   *widget,
            GdkEventKey *event,
            gpointer     user_data)
{
  gboolean ctrl_pressed = modifier_pressed(*event, GDK_CONTROL_MASK);
  MarkerEditorWindow *window = MARKER_EDITOR_WINDOW(widget);
  
  switch (event->keyval)
  {
    case GDK_KEY_F11:
      marker_editor_window_toggle_fullscreen (window);
      break;
  }
  
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
      
      case GDK_KEY_p:
        marker_preview_run_print_dialog(window->web_view, GTK_WINDOW(window));
        break;

      case GDK_KEY_1:
        marker_editor_window_set_view_mode (window, EDITOR_ONLY_MODE);
        break;
      
      case GDK_KEY_2:
        marker_editor_window_set_view_mode (window, PREVIEW_ONLY_MODE);
        break;
      
      case GDK_KEY_3:
        marker_editor_window_set_view_mode (window, DUAL_PANE_MODE);
        break;
      
      case GDK_KEY_4:
        marker_editor_window_set_view_mode (window, DUAL_WINDOW_MODE);
        break;
    }
  }
  
  return FALSE;
}

MarkerPreview*
marker_editor_window_get_preview(MarkerEditorWindow* window)
{
  return window->web_view;
}

static void
init_ui (MarkerEditorWindow *window)
{
  GtkBuilder* builder =
    gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/ui/editor-window.ui");

  GtkBox* vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  window->vbox = vbox;
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));

  // Header Bar //
  GtkBox *header_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  window->header_box = header_box;
  GtkHeaderBar* header_bar = GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));
  window->header_bar = header_bar;
  gtk_window_set_titlebar(GTK_WINDOW (window), GTK_WIDGET (header_box));
  gtk_box_pack_start (header_box, GTK_WIDGET (header_bar), FALSE, TRUE, 0);
  gtk_header_bar_set_show_close_button (header_bar, TRUE);
  gtk_widget_show (GTK_WIDGET (header_box));
  
  GtkButton *unfullscreen_btn =
    GTK_BUTTON (gtk_builder_get_object (builder, "unfullscreen_btn"));
  window->unfullscreen_btn = unfullscreen_btn;
  g_signal_connect (unfullscreen_btn,
                    "clicked",
                    G_CALLBACK (unfullscreen_btn_clicked),
                    window);
  
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
                                    APP_MENU_ACTION_ENTRIES_LEN,
                                    window);
  }
  
  // Source View //
  GtkWidget* source_view = GTK_WIDGET(marker_source_view_new());
  window->source_view = MARKER_SOURCE_VIEW(source_view);
  GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  g_signal_connect(buf, "changed", G_CALLBACK(buffer_changed), window);
  GtkWidget* source_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(source_scroll), source_view);
  window->source_scroll = source_scroll;
  gtk_widget_show_all(GTK_WIDGET(source_scroll));
  
  // Web View //
  GtkWidget* web_view = GTK_WIDGET(marker_preview_new());
  window->web_view = MARKER_PREVIEW(web_view);
  gtk_widget_show_all(web_view);
  
  // View Area //
  GtkPaned* paned = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_HORIZONTAL));
  window->paned = paned;
  gtk_paned_set_position(paned, 450);
  gtk_box_pack_start(vbox, GTK_WIDGET(paned), TRUE, TRUE, 0);
  gtk_widget_show_all(GTK_WIDGET(vbox));
  
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_signal_connect(window, "delete-event", G_CALLBACK(close_btn_pressed), window);
  g_signal_connect(window, "key-press-event", G_CALLBACK(key_pressed), NULL);
  marker_editor_window_set_title_filename(window);
  marker_editor_window_set_view_mode (window, marker_prefs_get_default_view_mode());
  marker_editor_window_refresh_preview(window);
  
  gtk_widget_grab_focus(source_view);
  
  gtk_builder_add_callback_symbol(builder, "open_cb", G_CALLBACK(open_cb));
  gtk_builder_add_callback_symbol(builder, "save_cb", G_CALLBACK(save_cb));
  gtk_builder_connect_signals(builder, window);
  
  g_object_unref(builder);
}

static void
marker_editor_window_init (MarkerEditorWindow *window)
{
  window->is_fullscreen = FALSE;
  
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

