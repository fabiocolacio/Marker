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

#include "marker-source-view.h"
#include "marker-prefs.h"

#include <gtkspell/gtkspell.h>

struct _MarkerSourceView
{
  GtkSourceView parent_instance;
  GSettings* settings;
  GtkSpellChecker* spell;
};

G_DEFINE_TYPE(MarkerSourceView, marker_source_view, GTK_SOURCE_TYPE_VIEW)

void
marker_source_view_surround_selection_with(MarkerSourceView* source_view,
                                           const char*       insertion)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  GtkTextIter start, end;
  gint start_index, end_index, selection_len;
  gboolean selected;
  size_t len = strlen(insertion);
  
  selected = gtk_text_buffer_get_selection_bounds(buffer, &start, &end);  
  if (selected)
  {
    start_index = gtk_text_iter_get_line_offset(&start);
    end_index = gtk_text_iter_get_line_offset(&end);
    selection_len = end_index - start_index;
    
    gtk_text_buffer_insert(buffer, &start, insertion, len);
    gtk_text_iter_forward_chars(&start, selection_len);
    gtk_text_buffer_insert(buffer, &start, insertion, len);
  }
}

void
marker_source_view_set_spell_check(MarkerSourceView*    source_view,
                                   gboolean             state)
{
  if (state)
  {
    gtk_spell_checker_attach((GtkSpellChecker*)source_view->spell, GTK_TEXT_VIEW(source_view));
    g_object_unref (source_view->spell);
  }
  else
  {
    g_object_ref (source_view->spell);
    gtk_spell_checker_detach((GtkSpellChecker*)source_view->spell);
  }
}

void
marker_source_view_set_spell_check_lang(MarkerSourceView*   source_view,
                                        const gchar*        lang)
{
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
marker_source_view_get_text(MarkerSourceView* source_view)
{
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
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
default_font_changed(GSettings*   settings,
                     const gchar* key,
                     gpointer     user_data)
{
  MarkerSourceView* source_view = (MarkerSourceView*) user_data;
  gchar* fontname = g_settings_get_string(settings, key);
  PangoFontDescription* font = pango_font_description_from_string(fontname);
  gtk_widget_modify_font(GTK_WIDGET(source_view), font);
  pango_font_description_free(font);
  g_free(fontname);
}

static void
marker_source_view_init (MarkerSourceView *source_view)
{
  marker_source_view_set_language (source_view, "markdown");
  source_view->settings = g_settings_new ("org.gnome.desktop.interface");
  g_signal_connect (source_view->settings, "changed::monospace-font-name", G_CALLBACK (default_font_changed), source_view);
  gchar *fontname = g_settings_get_string (source_view->settings, "monospace-font-name");
  PangoFontDescription* font = pango_font_description_from_string (fontname);
  gtk_widget_modify_font (GTK_WIDGET (source_view), font);
  pango_font_description_free (font);
  g_free (fontname);

  gtk_source_view_set_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (source_view), marker_prefs_get_replace_tabs ());
  gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (source_view), marker_prefs_get_tab_width ());
  gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (source_view), marker_prefs_get_auto_indent ());

  source_view->spell = gtk_spell_checker_new ();
  gchar* lang = marker_prefs_get_spell_check_langauge();
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
