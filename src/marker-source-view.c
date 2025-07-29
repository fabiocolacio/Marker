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
  
  if (custom_family && strlen(custom_family) > 0 && g_strcmp0(custom_family, "System Monospace") != 0) {
    fontname = g_strdup(custom_family);
  } else {
    /* Use "Monospace" alias which resolves to system default monospace font */
    fontname = g_strdup("Monospace");
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
  
  if (custom_family && strlen(custom_family) > 0 && g_strcmp0(custom_family, "System Monospace") != 0) {
    fontname = g_strdup(custom_family);
  } else {
    /* Use "Monospace" alias which resolves to system default monospace font */
    fontname = g_strdup("Monospace");
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
  
  if (custom_family && strlen(custom_family) > 0 && g_strcmp0(custom_family, "System Monospace") != 0) {
    fontname = g_strdup(custom_family);
  } else {
    /* Use "Monospace" alias which resolves to system default monospace font */
    fontname = g_strdup("Monospace");
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

void
marker_source_view_convert_to_bullet_list (MarkerSourceView *source_view)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));
  
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  
  /* Check if there's a selection */
  if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
  {
    /* No selection, just add bullet to current line */
    GtkTextMark *insert_mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
    gtk_text_iter_set_line_offset (&start, 0);
    
    /* Get the current line */
    GtkTextIter line_end = start;
    gtk_text_iter_forward_to_line_end (&line_end);
    gchar *line_text = gtk_text_buffer_get_text (buffer, &start, &line_end, FALSE);
    
    /* Remove numbered list prefix if present */
    GRegex *num_regex = g_regex_new ("^\\d+\\.\\s+", 0, 0, NULL);
    gchar *cleaned_text = g_regex_replace (num_regex, line_text, -1, 0, "", 0, NULL);
    g_regex_unref (num_regex);
    
    /* Check if line already starts with "- " */
    if (!g_str_has_prefix (cleaned_text, "- "))
    {
      /* Delete the original line and insert with bullet */
      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_delete (buffer, &start, &line_end);
      gchar *new_line = g_strdup_printf ("- %s", cleaned_text);
      gtk_text_buffer_insert (buffer, &start, new_line, -1);
      
      /* Select the entire line */
      GtkTextIter new_end = start;
      gtk_text_iter_forward_chars (&new_end, g_utf8_strlen (new_line, -1));
      gtk_text_buffer_select_range (buffer, &start, &new_end);
      
      g_free (new_line);
      gtk_text_buffer_end_user_action (buffer);
    }
    
    g_free (cleaned_text);
    g_free (line_text);
    return;
  }
  
  /* Process selected lines */
  gtk_text_iter_set_line_offset (&start, 0);
  if (!gtk_text_iter_starts_line (&end))
    gtk_text_iter_forward_line (&end);
  
  /* Get the selected text */
  gchar *selected_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  gchar **lines = g_strsplit (selected_text, "\n", -1);
  
  /* Build new text with bullets */
  GString *new_text = g_string_new ("");
  for (int i = 0; lines[i] != NULL; i++)
  {
    /* Skip empty lines at the end */
    if (lines[i + 1] == NULL && strlen (lines[i]) == 0)
      break;
      
    /* Check if line starts with numbered list pattern and remove it */
    GRegex *num_regex = g_regex_new ("^\\d+\\.\\s+", 0, 0, NULL);
    gchar *line_without_number = g_regex_replace (num_regex, lines[i], -1, 0, "", 0, NULL);
    g_regex_unref (num_regex);
    
    /* Check if line already starts with "- " */
    if (!g_str_has_prefix (line_without_number, "- "))
    {
      g_string_append (new_text, "- ");
    }
    g_string_append (new_text, line_without_number);
    if (lines[i + 1] != NULL)
      g_string_append_c (new_text, '\n');
      
    g_free (line_without_number);
  }
  
  /* Replace the selection */
  gtk_text_buffer_begin_user_action (buffer);
  
  /* Remember the start position */
  GtkTextMark *start_mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
  
  gtk_text_buffer_delete (buffer, &start, &end);
  gtk_text_buffer_insert (buffer, &start, new_text->str, -1);
  
  /* Get the start position from mark and calculate end */
  GtkTextIter new_start, new_end;
  gtk_text_buffer_get_iter_at_mark (buffer, &new_start, start_mark);
  new_end = new_start;
  gtk_text_iter_forward_chars (&new_end, g_utf8_strlen (new_text->str, -1));
  
  /* Select the new text */
  gtk_text_buffer_select_range (buffer, &new_start, &new_end);
  
  /* Ensure the view has focus */
  gtk_widget_grab_focus (GTK_WIDGET (source_view));
  
  gtk_text_buffer_delete_mark (buffer, start_mark);
  gtk_text_buffer_end_user_action (buffer);
  
  /* Clean up */
  g_string_free (new_text, TRUE);
  g_strfreev (lines);
  g_free (selected_text);
}

void
marker_source_view_convert_to_numbered_list (MarkerSourceView *source_view)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));
  
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  
  /* Check if there's a selection */
  if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
  {
    /* No selection, just add number to current line */
    GtkTextMark *insert_mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
    gtk_text_iter_set_line_offset (&start, 0);
    
    /* Get the current line */
    GtkTextIter line_end = start;
    gtk_text_iter_forward_to_line_end (&line_end);
    gchar *line_text = gtk_text_buffer_get_text (buffer, &start, &line_end, FALSE);
    
    gchar *cleaned_text = line_text;
    gboolean needs_free = FALSE;
    
    /* Remove bullet prefix if present */
    if (g_str_has_prefix (line_text, "- "))
    {
      cleaned_text = line_text + 2;
    }
    /* Remove existing number if present */
    else if (g_regex_match_simple ("^\\d+\\.\\s+", line_text, 0, 0))
    {
      GRegex *num_regex = g_regex_new ("^\\d+\\.\\s+", 0, 0, NULL);
      cleaned_text = g_regex_replace (num_regex, line_text, -1, 0, "", 0, NULL);
      g_regex_unref (num_regex);
      needs_free = TRUE;
    }
    
    /* If not already numbered, add number */
    if (cleaned_text != line_text || needs_free)
    {
      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_delete (buffer, &start, &line_end);
      gchar *new_line = g_strdup_printf ("1. %s", cleaned_text);
      gtk_text_buffer_insert (buffer, &start, new_line, -1);
      
      /* Select the entire line */
      GtkTextIter new_end = start;
      gtk_text_iter_forward_chars (&new_end, g_utf8_strlen (new_line, -1));
      gtk_text_buffer_select_range (buffer, &start, &new_end);
      
      g_free (new_line);
      gtk_text_buffer_end_user_action (buffer);
    }
    else
    {
      /* Line has no list marker, just add number */
      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_insert (buffer, &start, "1. ", -1);
      
      /* Select the entire line */
      gtk_text_iter_forward_to_line_end (&line_end);
      gtk_text_buffer_select_range (buffer, &start, &line_end);
      gtk_text_buffer_end_user_action (buffer);
    }
    
    if (needs_free)
      g_free (cleaned_text);
    g_free (line_text);
    return;
  }
  
  /* Process selected lines */
  gtk_text_iter_set_line_offset (&start, 0);
  if (!gtk_text_iter_starts_line (&end))
    gtk_text_iter_forward_line (&end);
  
  /* Get the selected text */
  gchar *selected_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  gchar **lines = g_strsplit (selected_text, "\n", -1);
  
  /* Build new text with numbers */
  GString *new_text = g_string_new ("");
  int line_num = 1;
  for (int i = 0; lines[i] != NULL; i++)
  {
    /* Skip empty lines at the end */
    if (lines[i + 1] == NULL && strlen (lines[i]) == 0)
      break;
      
    gchar *cleaned_line = lines[i];
    gboolean needs_free = FALSE;
    
    /* Remove bullet list marker if present */
    if (g_str_has_prefix (lines[i], "- "))
    {
      cleaned_line = lines[i] + 2;
    }
    /* Remove existing number if present */
    else if (g_regex_match_simple ("^\\d+\\.\\s+", lines[i], 0, 0))
    {
      GRegex *num_regex = g_regex_new ("^\\d+\\.\\s+", 0, 0, NULL);
      cleaned_line = g_regex_replace (num_regex, lines[i], -1, 0, "", 0, NULL);
      g_regex_unref (num_regex);
      needs_free = TRUE;
    }
    
    /* Add number */
    g_string_append_printf (new_text, "%d. ", line_num);
    g_string_append (new_text, cleaned_line);
    if (lines[i + 1] != NULL)
      g_string_append_c (new_text, '\n');
      
    if (needs_free)
      g_free (cleaned_line);
      
    line_num++;
  }
  
  /* Replace the selection */
  gtk_text_buffer_begin_user_action (buffer);
  
  /* Remember the start position */
  GtkTextMark *start_mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
  
  gtk_text_buffer_delete (buffer, &start, &end);
  gtk_text_buffer_insert (buffer, &start, new_text->str, -1);
  
  /* Get the start position from mark and calculate end */
  GtkTextIter new_start, new_end;
  gtk_text_buffer_get_iter_at_mark (buffer, &new_start, start_mark);
  new_end = new_start;
  gtk_text_iter_forward_chars (&new_end, g_utf8_strlen (new_text->str, -1));
  
  /* Select the new text */
  gtk_text_buffer_select_range (buffer, &new_start, &new_end);
  
  /* Ensure the view has focus */
  gtk_widget_grab_focus (GTK_WIDGET (source_view));
  
  gtk_text_buffer_delete_mark (buffer, start_mark);
  gtk_text_buffer_end_user_action (buffer);
  
  /* Clean up */
  g_string_free (new_text, TRUE);
  g_strfreev (lines);
  g_free (selected_text);
}
void
marker_source_view_align_table (MarkerSourceView *source_view)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));
  
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  
  /* Check if there's a selection */
  if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
  {
    /* No selection, try to detect table at cursor position */
    GtkTextIter cursor;
    gtk_text_buffer_get_iter_at_mark (buffer, &cursor, gtk_text_buffer_get_insert (buffer));
    
    /* Find start of table (go up until we find a non-table line) */
    start = cursor;
    gtk_text_iter_set_line_offset (&start, 0);
    while (gtk_text_iter_backward_line (&start))
    {
      GtkTextIter line_end = start;
      gtk_text_iter_forward_to_line_end (&line_end);
      gchar *line = gtk_text_buffer_get_text (buffer, &start, &line_end, FALSE);
      gchar *trimmed = g_strstrip (g_strdup (line));
      
      if (!g_str_has_prefix (trimmed, "|"))
      {
        g_free (line);
        g_free (trimmed);
        gtk_text_iter_forward_line (&start);
        break;
      }
      g_free (line);
      g_free (trimmed);
    }
    
    /* Find end of table (go down until we find a non-table line) */
    end = cursor;
    gtk_text_iter_forward_to_line_end (&end);
    while (gtk_text_iter_forward_line (&end))
    {
      GtkTextIter line_start = end;
      gtk_text_iter_set_line_offset (&line_start, 0);
      GtkTextIter line_end = end;
      gtk_text_iter_forward_to_line_end (&line_end);
      gchar *line = gtk_text_buffer_get_text (buffer, &line_start, &line_end, FALSE);
      gchar *trimmed = g_strstrip (g_strdup (line));
      
      if (!g_str_has_prefix (trimmed, "|"))
      {
        g_free (line);
        g_free (trimmed);
        break;
      }
      g_free (line);
      g_free (trimmed);
      end = line_end;
    }
  }
  
  /* Make sure selection starts at beginning of line */
  if (!gtk_text_iter_starts_line (&start))
    gtk_text_iter_set_line_offset (&start, 0);
  
  /* Make sure selection ends at end of line */
  if (!gtk_text_iter_ends_line (&end))
    gtk_text_iter_forward_to_line_end (&end);
  
  /* Get the selected text */
  gchar *text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  gchar **lines = g_strsplit (text, "\n", -1);
  g_free (text);
  
  /* Parse table rows and cells */
  GPtrArray *table_rows = g_ptr_array_new ();
  gint max_columns = 0;
  
  /* First pass: extract all table rows and find maximum columns */
  for (gint i = 0; lines[i] != NULL; i++)
  {
    gchar *line = lines[i];
    gchar *trimmed = g_strstrip (g_strdup (line));
    
    /* Skip empty lines or non-table lines */
    if (strlen (trimmed) == 0 || !g_str_has_prefix (trimmed, "|"))
    {
      g_free (trimmed);
      continue;
    }
    
    /* Split by pipe and extract cells */
    gchar **raw_cells = g_strsplit (trimmed, "|", -1);
    GPtrArray *row_cells = g_ptr_array_new_with_free_func (g_free);
    
    /* Process each cell (skip first and last which are usually empty) */
    for (gint j = 1; raw_cells[j] != NULL && raw_cells[j + 1] != NULL; j++)
    {
      gchar *cell = g_strstrip (g_strdup (raw_cells[j]));
      g_ptr_array_add (row_cells, cell);
    }
    
    if (row_cells->len > max_columns)
      max_columns = row_cells->len;
    
    g_ptr_array_add (table_rows, row_cells);
    g_strfreev (raw_cells);
    g_free (trimmed);
  }
  
  /* If no table found, return */
  if (table_rows->len == 0 || max_columns == 0)
  {
    g_ptr_array_free (table_rows, TRUE);
    g_strfreev (lines);
    return;
  }
  
  /* Calculate maximum width for each column */
  gint *column_widths = g_new0 (gint, max_columns);
  
  for (gint i = 0; i < table_rows->len; i++)
  {
    GPtrArray *row = g_ptr_array_index (table_rows, i);
    for (gint j = 0; j < row->len && j < max_columns; j++)
    {
      gchar *cell = g_ptr_array_index (row, j);
      gint cell_width = g_utf8_strlen (cell, -1);
      
      /* For separator rows, ensure minimum width of 3 for alignment markers */
      if (g_strstr_len (cell, -1, "-") != NULL)
      {
        if (cell_width < 3)
          cell_width = 3;
      }
      
      if (cell_width > column_widths[j])
        column_widths[j] = cell_width;
    }
  }
  
  /* Build the aligned table */
  GString *aligned_table = g_string_new ("");
  gint table_line_index = 0;
  
  for (gint i = 0; lines[i] != NULL; i++)
  {
    gchar *line = lines[i];
    gchar *trimmed = g_strstrip (g_strdup (line));
    
    /* Handle non-table lines */
    if (strlen (trimmed) == 0)
    {
      /* Empty line */
      if (lines[i + 1] != NULL)
        g_string_append_c (aligned_table, '\n');
      g_free (trimmed);
      continue;
    }
    
    if (!g_str_has_prefix (trimmed, "|"))
    {
      /* Non-table line */
      g_string_append (aligned_table, line);
      if (lines[i + 1] != NULL)
        g_string_append_c (aligned_table, '\n');
      g_free (trimmed);
      continue;
    }
    g_free (trimmed);
    
    /* Process table row */
    GPtrArray *row = g_ptr_array_index (table_rows, table_line_index++);
    
    /* Check if this is a separator row */
    gboolean is_separator = FALSE;
    if (row->len > 0)
    {
      gchar *first_cell = g_ptr_array_index (row, 0);
      is_separator = (g_strstr_len (first_cell, -1, "-") != NULL);
    }
    
    /* Build the aligned row */
    g_string_append (aligned_table, "|");
    
    for (gint j = 0; j < max_columns; j++)
    {
      g_string_append (aligned_table, " ");
      
      if (j < row->len)
      {
        gchar *cell = g_ptr_array_index (row, j);
        
        if (is_separator)
        {
          /* Handle separator row with alignment markers */
          if (g_str_has_prefix (cell, ":") && g_str_has_suffix (cell, ":"))
          {
            /* Center alignment: :---: */
            g_string_append (aligned_table, ":");
            for (gint k = 1; k < column_widths[j] - 1; k++)
              g_string_append_c (aligned_table, '-');
            g_string_append (aligned_table, ":");
          }
          else if (g_str_has_suffix (cell, ":"))
          {
            /* Right alignment: ---: */
            for (gint k = 0; k < column_widths[j] - 1; k++)
              g_string_append_c (aligned_table, '-');
            g_string_append (aligned_table, ":");
          }
          else if (g_str_has_prefix (cell, ":"))
          {
            /* Left alignment: :--- */
            g_string_append (aligned_table, ":");
            for (gint k = 1; k < column_widths[j]; k++)
              g_string_append_c (aligned_table, '-');
          }
          else
          {
            /* Default alignment: --- */
            for (gint k = 0; k < column_widths[j]; k++)
              g_string_append_c (aligned_table, '-');
          }
        }
        else
        {
          /* Regular data cell - left align and pad with spaces */
          gint cell_len = g_utf8_strlen (cell, -1);
          g_string_append (aligned_table, cell);
          for (gint k = cell_len; k < column_widths[j]; k++)
            g_string_append_c (aligned_table, ' ');
        }
      }
      else
      {
        /* Empty cell - fill with spaces */
        for (gint k = 0; k < column_widths[j]; k++)
          g_string_append_c (aligned_table, ' ');
      }
      
      g_string_append (aligned_table, " |");
    }
    
    if (lines[i + 1] != NULL)
      g_string_append_c (aligned_table, '\n');
  }
  
  /* Replace the text in the buffer */
  gtk_text_buffer_begin_user_action (buffer);
  gtk_text_buffer_delete (buffer, &start, &end);
  gtk_text_buffer_insert (buffer, &start, aligned_table->str, -1);
  gtk_text_buffer_end_user_action (buffer);
  
  /* Clean up */
  g_free (column_widths);
  for (gint i = 0; i < table_rows->len; i++)
  {
    GPtrArray *row = g_ptr_array_index (table_rows, i);
    g_ptr_array_free (row, TRUE);
  }
  g_ptr_array_free (table_rows, TRUE);
  g_strfreev (lines);
  g_string_free (aligned_table, TRUE);
}