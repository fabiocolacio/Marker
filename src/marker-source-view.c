/*
 * marker-source-view.c
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Marker; see the file LICENSE.md. If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include <stdlib.h>

#include "marker-source-view.h"
#include "marker-prefs.h"
#include "marker-utils.h"
#include "marker.h"
#include "marker-window.h"
#include "marker-editor.h"

#include <glib.h>
#include <gtkspell/gtkspell.h>

struct _MarkerSourceView
{
  GtkSourceView           parent_instance;
  GSettings              *settings;
  GtkSpellChecker        *spell;
  GtkSourceSearchContext *search_context;
};

G_DEFINE_TYPE(MarkerSourceView, marker_source_view, GTK_SOURCE_TYPE_VIEW)

void
marker_source_view_surround_selection_with (MarkerSourceView *source_view,
                                            const char       *insertion)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  gint start_index, end_index, selection_len;
  gboolean selected;
  size_t len = strlen (insertion);

  selected = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

  start_index = gtk_text_iter_get_line_offset (&start);
  end_index = gtk_text_iter_get_line_offset (&end);
  selection_len = end_index - start_index;

  gtk_text_buffer_insert (buffer, &start, insertion, len);
  gtk_text_iter_forward_chars (&start, selection_len);
  gtk_text_buffer_insert (buffer, &start, insertion, len);

  if (!selected)
  {
    gtk_text_iter_backward_chars (&start, len);
    gtk_text_buffer_place_cursor (buffer, &start);
  }
}

void
marker_source_view_insert_link (MarkerSourceView   *source_view)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  gint start_index, end_index, selection_len;
  gboolean selected;

  selected = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if (selected) {
    start_index = gtk_text_iter_get_line_offset (&start);
    end_index = gtk_text_iter_get_line_offset (&end);
    selection_len = end_index - start_index;

    gchar * selected = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
    if (!marker_utils_is_url(selected)) {
      gtk_text_buffer_insert (buffer, &start, "[", 1);
      gtk_text_iter_forward_chars (&start, selection_len);
      gtk_text_buffer_insert (buffer, &start, "]()", 3);
      gtk_text_iter_backward_chars(&start, 1);
      gtk_text_buffer_place_cursor(buffer, &start);

    } else {
      gtk_text_buffer_insert (buffer, &start, "[](", 3);
      gtk_text_iter_forward_chars (&start, selection_len);
      gtk_text_buffer_insert (buffer, &start, ")", 1);
      gtk_text_iter_backward_chars(&start, selection_len + 3);
      gtk_text_buffer_place_cursor(buffer, &start);
    }
  } else {
    gchar * link = g_strdup("[]()");
    GtkTextMark * mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
    size_t len = strlen(link);

    gtk_text_buffer_insert(buffer, &start, link, len);
    gtk_text_iter_backward_chars(&start, 3);
    gtk_text_buffer_place_cursor(buffer, &start);
    g_free(link);
  }
}

void
marker_source_view_insert_image (MarkerSourceView   *source_view,
                                 const char         *image_path)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  GtkTextIter start ;

  gtk_text_buffer_get_iter_at_mark(buffer, &start, gtk_text_buffer_get_insert(buffer));

  gchar * img = g_strdup_printf("![](%s)", image_path);

  size_t len = strlen(img);

  gtk_text_buffer_insert(buffer, &start, img, len);
  g_free(img);
}

void
marker_source_view_set_spell_check(MarkerSourceView *source_view,
                                   gboolean          state)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));

  gboolean is_attached =
    source_view->spell == gtk_spell_checker_get_from_text_view (GTK_TEXT_VIEW (source_view));

  if (state && !is_attached)
  {
    gtk_spell_checker_attach((GtkSpellChecker*)source_view->spell, GTK_TEXT_VIEW(source_view));
  }
  else if (!state && is_attached)
  {
    g_object_ref (source_view->spell);
    gtk_spell_checker_detach((GtkSpellChecker*)source_view->spell);
  }
}

void
marker_source_view_set_spell_check_lang (MarkerSourceView *source_view,
                                         const gchar      *lang)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));
  gtk_spell_checker_set_language (source_view->spell, lang, NULL);
}

void
marker_source_view_set_syntax_theme(MarkerSourceView* source_view,
                                    const char*       theme)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  GtkSourceStyleSchemeManager* style_manager =
    gtk_source_style_scheme_manager_get_default();
  GtkSourceStyleScheme* scheme =
    gtk_source_style_scheme_manager_get_scheme(style_manager, theme);
  gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), scheme);
}

gboolean
marker_source_view_get_modified(MarkerSourceView* source_view)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  return gtk_text_buffer_get_modified(buffer);
}

void
marker_source_view_set_modified(MarkerSourceView* source_view,
                                gboolean          modified)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  gtk_text_buffer_set_modified(buffer, modified);
}

gchar*
marker_source_view_get_text(MarkerSourceView* source_view,
                            gboolean          include_position)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  if (include_position && !gtk_text_iter_equal(&start, &end)) {
    gchar * identifier = g_strdup("<span id=\"cursor_pos\"></span>"); 
    GtkTextIter pos;
    gtk_text_buffer_get_selection_bounds (buffer, &pos, NULL);
    gchar * beginning = gtk_text_buffer_get_text(buffer, &start, &pos, FALSE);
    gchar * ending = gtk_text_buffer_get_text(buffer, &pos, &end, FALSE);
    return g_strconcat(beginning, identifier, ending, NULL);
  }
  return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

int                      
marker_source_view_get_cursor_position (MarkerSourceView   *source_view)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  GtkTextIter pos;
  gtk_text_buffer_get_selection_bounds (buffer, &pos, NULL);
  return gtk_text_iter_get_offset(&pos);
}


void
marker_source_view_set_text(MarkerSourceView* source_view,
                            const char*       text,
                            size_t            size)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  gtk_text_buffer_set_text(buffer, text, size);
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
apply_font_css(MarkerSourceView* source_view, const gchar* fontname, guint font_size)
{
  gchar* css_font = g_strdup_printf("textview { font-family: \"%s\"; font-size: %upt; }", fontname, font_size);
  GtkCssProvider* provider = gtk_css_provider_new();
  GError* error = NULL;
  
  if (gtk_css_provider_load_from_data(provider, css_font, -1, &error)) {
    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(source_view));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  } else {
    g_warning("Failed to load CSS: %s", error ? error->message : "Unknown error");
    if (error) g_error_free(error);
  }
  
  g_object_unref(provider);
  g_free(css_font);
}

static void
default_font_changed(GSettings*   settings,
                     const gchar* key,
                     gpointer     user_data)
{
  MarkerSourceView* source_view = (MarkerSourceView*) user_data;
  gchar* custom_family = marker_prefs_get_editor_font_family();
  gchar* fontname;
  
  if (custom_family && strlen(custom_family) > 0 && g_strcmp0(custom_family, "System Default") != 0) {
    fontname = g_strdup(custom_family);
  } else {
    fontname = g_settings_get_string(settings, key);
  }
  
  guint font_size = marker_prefs_get_editor_font_size();
  
  apply_font_css(source_view, fontname, font_size);
  g_free(fontname);
  g_free(custom_family);
}

static gboolean 
update_other_editors_callback(gpointer user_data)
{
  guint *update_id_ptr = (guint *)user_data;
  
  GtkApplication *app = marker_get_app();
  if (!app) {
    *update_id_ptr = 0;
    return G_SOURCE_REMOVE;
  }
  
  GList *windows = gtk_application_get_windows(app);
  for (GList *item = windows; item != NULL; item = item->next) {
    if (MARKER_IS_WINDOW(item->data)) {
      MarkerWindow *window = MARKER_WINDOW(item->data);
      MarkerEditor *editor = marker_window_get_active_editor(window);
      if (editor) {
        marker_editor_apply_prefs(editor);
      }
    }
  }
  
  *update_id_ptr = 0;
  return G_SOURCE_REMOVE;
}

static gboolean
on_scroll_event(GtkWidget* widget, GdkEventScroll* event, gpointer user_data)
{
  gboolean ctrl_pressed = (event->state & GDK_CONTROL_MASK) != 0;
  gboolean zoom_enabled = marker_prefs_get_use_ctrl_wheel_zoom();
  
  /* Check if Ctrl key is pressed and the setting is enabled */
  if (ctrl_pressed && zoom_enabled) {
    guint current_size = marker_prefs_get_editor_font_size();
    guint new_size = current_size;
    gboolean size_changed = FALSE;
    
    /* Handle both discrete and smooth scrolling */
    if (event->direction == GDK_SCROLL_UP) {
      /* Discrete scroll up - increase font size */
      if (current_size < 72) {
        new_size = current_size + 1;
        size_changed = TRUE;
      }
    } else if (event->direction == GDK_SCROLL_DOWN) {
      /* Discrete scroll down - decrease font size */
      if (current_size > 6) {
        new_size = current_size - 1;
        size_changed = TRUE;
      }
    } else if (event->direction == GDK_SCROLL_SMOOTH) {
      /* Smooth scrolling - use delta values */
      gdouble delta_x, delta_y;
      if (gdk_event_get_scroll_deltas((GdkEvent*)event, &delta_x, &delta_y)) {
        /* Negative delta_y means scroll up (increase font), positive means scroll down (decrease font) */
        if (delta_y < -0.1 && current_size < 72) { /* Scroll up */
          new_size = current_size + 1;
          size_changed = TRUE;
        } else if (delta_y > 0.1 && current_size > 6) { /* Scroll down */
          new_size = current_size - 1;
          size_changed = TRUE;
        }
      }
    }
    
    /* Update font size if it changed */
    if (size_changed) {
      /* Immediate update without throttling for better responsiveness */
      marker_prefs_set_editor_font_size(new_size);
      
      /* Update only the current source view immediately */
      marker_source_view_update_font(MARKER_SOURCE_VIEW(widget));
      
      /* Schedule update for other editors if not already scheduled */
      static guint update_id = 0;
      if (update_id > 0) {
        g_source_remove(update_id);
        update_id = 0;
      }
      
      /* Defer updating other editors to avoid blocking */
      update_id = g_timeout_add(100, (GSourceFunc)update_other_editors_callback, &update_id);
    }
    
    /* Return TRUE to consume the event and prevent scrolling */
    return TRUE;
  }
  
  /* Return FALSE to allow normal scrolling */
  return FALSE;
}

static void
on_size_allocate (GtkWidget     *widget,
                  GtkAllocation *allocation,
                  gpointer       user_data)
{
  /* Set bottom margin to half the height of the editor window */
  /* This allows scrolling past the end by half the window size */
  gint bottom_margin = allocation->height / 2;
  gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(widget), bottom_margin);
}

static gboolean
on_key_press_event (GtkWidget   *widget,
                    GdkEventKey *event,
                    gpointer     user_data)
{
  /* Check for Ctrl+C */
  if ((event->state & GDK_CONTROL_MASK) && 
      (event->keyval == GDK_KEY_c || event->keyval == GDK_KEY_C))
  {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    GtkTextIter start, end;
    
    /* Check if there's a selection */
    if (!gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
    {
      /* No selection - copy the current line */
      GtkTextIter line_start, line_end;
      GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buffer);
      gtk_text_buffer_get_iter_at_mark(buffer, &line_start, insert_mark);
      
      /* Move to start of line */
      gtk_text_iter_set_line_offset(&line_start, 0);
      
      /* Copy line_start to line_end */
      line_end = line_start;
      
      /* Move to end of line */
      if (!gtk_text_iter_ends_line(&line_end))
        gtk_text_iter_forward_to_line_end(&line_end);
      
      /* Include the newline character */
      if (!gtk_text_iter_is_end(&line_end))
        gtk_text_iter_forward_char(&line_end);
      
      /* Get the text with newline at beginning and end */
      gchar *line_text = gtk_text_buffer_get_text(buffer, &line_start, &line_end, FALSE);
      gchar *text_to_copy;
      
      /* Add newline at the beginning if we're not at the first line */
      if (gtk_text_iter_get_line(&line_start) > 0) {
        text_to_copy = g_strdup_printf("\n%s", line_text);
      } else {
        text_to_copy = g_strdup(line_text);
      }
      
      /* Copy to clipboard */
      GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      gtk_clipboard_set_text(clipboard, text_to_copy, -1);
      
      g_free(line_text);
      g_free(text_to_copy);
      
      /* Return TRUE to indicate we handled the event */
      return TRUE;
    }
  }
  
  /* Return FALSE to let the default handler process the event */
  return FALSE;
}

static void
marker_source_view_init (MarkerSourceView *source_view)
{
  GtkSourceSearchContext * search_context = gtk_source_search_context_new(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view))),
                                                                          NULL);
  source_view->search_context = search_context;
  marker_source_view_set_language (source_view, "markdown");
  
  /* Enable scroll past end by setting an initial bottom margin */
  /* This will be dynamically adjusted based on window size */
  gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(source_view), 400);
  
  /* Connect size-allocate handler to dynamically adjust bottom margin */
  g_signal_connect(source_view, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
  
  /* Connect key-press-event handler for copy current line functionality */
  g_signal_connect(source_view, "key-press-event", G_CALLBACK(on_key_press_event), NULL);
  
  /* Connect scroll event handler for Ctrl+wheel zoom */
  g_signal_connect(source_view, "scroll-event", G_CALLBACK(on_scroll_event), NULL);
  source_view->settings = g_settings_new ("org.gnome.desktop.interface");
  g_signal_connect (source_view->settings, "changed::monospace-font-name", G_CALLBACK (default_font_changed), source_view);
  
  gchar* custom_family = marker_prefs_get_editor_font_family();
  gchar* fontname;
  
  if (custom_family && strlen(custom_family) > 0 && g_strcmp0(custom_family, "System Default") != 0) {
    fontname = g_strdup(custom_family);
  } else {
    fontname = g_settings_get_string(source_view->settings, "monospace-font-name");
  }
  
  guint font_size = marker_prefs_get_editor_font_size();
  
  apply_font_css(source_view, fontname, font_size);
  g_free(fontname);
  g_free(custom_family);

  gtk_source_view_set_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (source_view), marker_prefs_get_replace_tabs ());
  gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (source_view), marker_prefs_get_tab_width ());
  gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (source_view), marker_prefs_get_auto_indent ());

  source_view->spell = gtk_spell_checker_new ();
  gchar* lang = marker_prefs_get_spell_check_language();
  gtk_spell_checker_set_language (source_view->spell, lang, NULL);
  if (marker_prefs_get_spell_check ()){
    gtk_spell_checker_attach (source_view->spell, GTK_TEXT_VIEW (source_view));
  }
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

void
marker_source_view_update_font(MarkerSourceView* source_view)
{
  gchar* custom_family = marker_prefs_get_editor_font_family();
  gchar* fontname;
  
  if (custom_family && strlen(custom_family) > 0 && g_strcmp0(custom_family, "System Default") != 0) {
    fontname = g_strdup(custom_family);
  } else {
    fontname = g_settings_get_string(source_view->settings, "monospace-font-name");
  }
  
  guint font_size = marker_prefs_get_editor_font_size();
  
  apply_font_css(source_view, fontname, font_size);
  g_free(fontname);
  g_free(custom_family);
}

GtkSourceSearchContext*
marker_source_get_search_context (MarkerSourceView   *source_view)
{
  return source_view->search_context;
}
