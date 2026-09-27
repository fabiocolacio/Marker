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

#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <libspelling.h>

#define EDITOR_CSS_CLASS "marker-editor"

struct _MarkerSourceView
{
  GtkSourceView              parent_instance;
  GSettings                 *settings;
  SpellingChecker           *checker;
  SpellingTextBufferAdapter *spell_adapter;
  GtkSourceSearchContext    *search_context;
};

G_DEFINE_FINAL_TYPE (MarkerSourceView, marker_source_view, GTK_SOURCE_TYPE_VIEW)

/* One display-wide provider shared by every editor instance */
static GtkCssProvider *font_provider = NULL;

void
marker_source_view_surround_selection_with (MarkerSourceView *source_view,
                                            const char       *insertion)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  gint start_index, end_index, selection_len;
  gboolean selected;
  size_t len = strlen (insertion);

  selected = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

  start_index = gtk_text_iter_get_offset (&start);
  end_index = gtk_text_iter_get_offset (&end);
  selection_len = end_index - start_index;

  gtk_text_buffer_begin_user_action (buffer);
  gtk_text_buffer_insert (buffer, &start, insertion, len);
  gtk_text_iter_forward_chars (&start, selection_len);
  gtk_text_buffer_insert (buffer, &start, insertion, len);
  gtk_text_buffer_end_user_action (buffer);

  if (!selected)
  {
    gtk_text_iter_backward_chars (&start, len);
    gtk_text_buffer_place_cursor (buffer, &start);
  }
}

void
marker_source_view_insert_link (MarkerSourceView *source_view)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  gint start_index, end_index, selection_len;
  gboolean selected;

  selected = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  gtk_text_buffer_begin_user_action (buffer);
  if (selected)
  {
    start_index = gtk_text_iter_get_offset (&start);
    end_index = gtk_text_iter_get_offset (&end);
    selection_len = end_index - start_index;

    g_autofree gchar *selected_text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    if (!marker_utils_is_url (selected_text))
    {
      gtk_text_buffer_insert (buffer, &start, "[", 1);
      gtk_text_iter_forward_chars (&start, selection_len);
      gtk_text_buffer_insert (buffer, &start, "]()", 3);
      gtk_text_iter_backward_chars (&start, 1);
      gtk_text_buffer_place_cursor (buffer, &start);
    }
    else
    {
      gtk_text_buffer_insert (buffer, &start, "[](", 3);
      gtk_text_iter_forward_chars (&start, selection_len);
      gtk_text_buffer_insert (buffer, &start, ")", 1);
      gtk_text_iter_backward_chars (&start, selection_len + 3);
      gtk_text_buffer_place_cursor (buffer, &start);
    }
  }
  else
  {
    const gchar *link = "[]()";
    GtkTextMark *mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &start, mark);
    gtk_text_buffer_insert (buffer, &start, link, strlen (link));
    gtk_text_iter_backward_chars (&start, 3);
    gtk_text_buffer_place_cursor (buffer, &start);
  }
  gtk_text_buffer_end_user_action (buffer);
}

void
marker_source_view_insert_image (MarkerSourceView *source_view,
                                 const char       *image_path)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start;

  gtk_text_buffer_get_iter_at_mark (buffer, &start, gtk_text_buffer_get_insert (buffer));

  g_autofree gchar *img = g_strdup_printf ("![](%s)", image_path);
  gtk_text_buffer_insert (buffer, &start, img, strlen (img));
}

void
marker_source_view_set_spell_check (MarkerSourceView *source_view,
                                    gboolean          state)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));
  spelling_text_buffer_adapter_set_enabled (source_view->spell_adapter, state);
}

void
marker_source_view_set_spell_check_lang (MarkerSourceView *source_view,
                                         const gchar      *lang)
{
  g_assert (MARKER_IS_SOURCE_VIEW (source_view));
  if (lang && *lang)
    spelling_checker_set_language (source_view->checker, lang);
}

void
marker_source_view_set_syntax_theme (MarkerSourceView *source_view,
                                     const char       *theme)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkSourceStyleSchemeManager *style_manager = gtk_source_style_scheme_manager_get_default ();
  GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme (style_manager, theme);
  if (scheme)
    gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (buffer), scheme);
}

gboolean
marker_source_view_get_modified (MarkerSourceView *source_view)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  return gtk_text_buffer_get_modified (buffer);
}

void
marker_source_view_set_modified (MarkerSourceView *source_view,
                                 gboolean          modified)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  gtk_text_buffer_set_modified (buffer, modified);
}

gchar *
marker_source_view_get_text (MarkerSourceView *source_view,
                             gboolean          include_position)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  if (include_position && !gtk_text_iter_equal (&start, &end))
  {
    const gchar *identifier = "<span id=\"cursor_pos\"></span>";
    GtkTextIter pos;
    gtk_text_buffer_get_selection_bounds (buffer, &pos, NULL);
    g_autofree gchar *beginning = gtk_text_buffer_get_text (buffer, &start, &pos, FALSE);
    g_autofree gchar *ending = gtk_text_buffer_get_text (buffer, &pos, &end, FALSE);
    return g_strconcat (beginning, identifier, ending, NULL);
  }
  return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

int
marker_source_view_get_cursor_position (MarkerSourceView *source_view)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  GtkTextIter pos;
  gtk_text_buffer_get_selection_bounds (buffer, &pos, NULL);
  return gtk_text_iter_get_offset (&pos);
}

void
marker_source_view_set_text (MarkerSourceView *source_view,
                             const char       *text,
                             size_t            size)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  gtk_text_buffer_set_text (buffer, text, size);
}

void
marker_source_view_set_language (MarkerSourceView *source_view,
                                 const gchar      *language_name)
{
  if (GTK_SOURCE_IS_VIEW (source_view))
  {
    GtkSourceLanguageManager *manager = gtk_source_language_manager_get_default ();
    GtkSourceLanguage *language = gtk_source_language_manager_get_language (manager, language_name);
    GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));
    gtk_source_buffer_set_language (buffer, language);
  }
}

static void
apply_font (const gchar *fontname)
{
  if (!font_provider)
  {
    font_provider = gtk_css_provider_new ();
    gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                GTK_STYLE_PROVIDER (font_provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  }

  PangoFontDescription *font = pango_font_description_from_string (fontname);
  const gchar *family = pango_font_description_get_family (font);
  gint size = pango_font_description_get_size (font);
  gboolean absolute = pango_font_description_get_size_is_absolute (font);

  GString *css = g_string_new ("." EDITOR_CSS_CLASS " { ");
  if (family && *family)
    g_string_append_printf (css, "font-family: \"%s\"; ", family);
  if (size > 0)
    g_string_append_printf (css, "font-size: %d%s; ", size / PANGO_SCALE, absolute ? "px" : "pt");
  g_string_append (css, "}");

  gtk_css_provider_load_from_string (font_provider, css->str);

  g_string_free (css, TRUE);
  pango_font_description_free (font);
}

static void
default_font_changed (GSettings   *settings,
                      const gchar *key,
                      gpointer     user_data)
{
  g_autofree gchar *fontname = g_settings_get_string (settings, key);
  apply_font (fontname);
}

static GMenuModel *
build_extra_menu (MarkerSourceView *source_view)
{
  GMenu *menu = g_menu_new ();

  GMenuModel *spelling_menu = spelling_text_buffer_adapter_get_menu_model (source_view->spell_adapter);
  if (spelling_menu)
    g_menu_append_section (menu, NULL, spelling_menu);

  GMenu *section = g_menu_new ();
  g_menu_append (section, _("_Reload from Disk"), "win.reload");
  g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
  g_object_unref (section);

  return G_MENU_MODEL (menu);
}

static void
marker_source_view_init (MarkerSourceView *source_view)
{
  GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));

  source_view->search_context = gtk_source_search_context_new (buffer, NULL);
  marker_source_view_set_language (source_view, "markdown");

  gtk_widget_add_css_class (GTK_WIDGET (source_view), EDITOR_CSS_CLASS);
  gtk_text_view_set_monospace (GTK_TEXT_VIEW (source_view), TRUE);

  source_view->settings = g_settings_new ("org.gnome.desktop.interface");
  g_signal_connect (source_view->settings, "changed::monospace-font-name",
                    G_CALLBACK (default_font_changed), source_view);
  g_autofree gchar *fontname = g_settings_get_string (source_view->settings, "monospace-font-name");
  apply_font (fontname);

  gtk_source_view_set_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (source_view), marker_prefs_get_replace_tabs ());
  gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (source_view), marker_prefs_get_tab_width ());
  gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (source_view), marker_prefs_get_auto_indent ());

  /* Spell checking (libspelling) */
  source_view->checker = spelling_checker_get_default ();
  g_autofree gchar *lang = marker_prefs_get_spell_check_language ();
  if (lang && *lang)
    spelling_checker_set_language (source_view->checker, lang);

  source_view->spell_adapter = spelling_text_buffer_adapter_new (buffer, source_view->checker);
  gtk_text_view_set_extra_menu (GTK_TEXT_VIEW (source_view), NULL);
  GMenuModel *extra_menu = build_extra_menu (source_view);
  gtk_text_view_set_extra_menu (GTK_TEXT_VIEW (source_view), extra_menu);
  g_object_unref (extra_menu);
  gtk_widget_insert_action_group (GTK_WIDGET (source_view), "spelling", G_ACTION_GROUP (source_view->spell_adapter));
  spelling_text_buffer_adapter_set_enabled (source_view->spell_adapter, marker_prefs_get_spell_check ());
}

static void
marker_source_view_dispose (GObject *object)
{
  MarkerSourceView *source_view = MARKER_SOURCE_VIEW (object);

  if (source_view->settings)
    g_signal_handlers_disconnect_by_func (source_view->settings, default_font_changed, source_view);

  g_clear_object (&source_view->settings);
  g_clear_object (&source_view->spell_adapter);
  g_clear_object (&source_view->search_context);

  G_OBJECT_CLASS (marker_source_view_parent_class)->dispose (object);
}

static void
marker_source_view_class_init (MarkerSourceViewClass *class)
{
  G_OBJECT_CLASS (class)->dispose = marker_source_view_dispose;
}

MarkerSourceView *
marker_source_view_new (void)
{
  return g_object_new (MARKER_TYPE_SOURCE_VIEW, NULL);
}

GtkSourceSearchContext *
marker_source_get_search_context (MarkerSourceView *source_view)
{
  return source_view->search_context;
}
