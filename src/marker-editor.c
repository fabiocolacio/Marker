/*
 * marker-editor.c
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

#include <locale.h>
#include <glib/gi18n.h>

#include "marker-prefs.h"
#include "marker-string.h"

#include "marker-editor.h"

struct _MarkerEditor
{
  GtkBox                parent_instance;

  GFile                *file;
  GFileMonitor         *file_monitor;
  gchar*                title;
  gboolean              unsaved_changes;

  GtkPaned             *paned;
  GtkBox               *vbox;
  GtkSearchEntry       *search_entry;
  GtkSearchBar         *search_bar;
  MarkerPreview        *preview;
  MarkerSourceView     *source_view;
  GtkScrolledWindow    *source_scroll;
  MarkerViewMode        view_mode;

  gboolean              search_active;
  gboolean              needs_refresh;
  guint                 timer_id;

  GtkTextIter          *text_iter;
  
  gboolean              scroll_sync_enabled;
  guint                 scroll_sync_timer;
};

G_DEFINE_TYPE (MarkerEditor, marker_editor, GTK_TYPE_BOX);

static void
emit_signal_title_changed (MarkerEditor *editor)
{
  g_autofree gchar *title = marker_editor_get_title (editor);
  g_autofree gchar *raw_title = marker_editor_get_raw_title(editor);
  g_signal_emit_by_name (editor, "title-changed", title, raw_title);
}

static void
emit_signal_subtitle_changed (MarkerEditor *editor)
{
  g_autofree gchar *subtitle = marker_editor_get_subtitle (editor);
  g_signal_emit_by_name (editor, "subtitle-changed", subtitle);
}

static gboolean
refresh_timeout_cb (gpointer user_data)
{
  MarkerEditor *editor = user_data;
  if (editor->needs_refresh)
    marker_editor_refresh_preview (editor);
  return G_SOURCE_CONTINUE;
}

static gboolean
on_editor_scroll_sync_timeout (gpointer user_data)
{
  MarkerEditor *editor = MARKER_EDITOR (user_data);
  editor->scroll_sync_timer = 0;
  return G_SOURCE_REMOVE;
}

static void
on_editor_scroll_event (GtkAdjustment *adj,
                        gpointer       user_data)
{
  MarkerEditor *editor = MARKER_EDITOR (user_data);
  
  if (!editor->scroll_sync_enabled || editor->scroll_sync_timer)
    return;
    
  if (editor->view_mode != DUAL_PANE_MODE)
    return;
  
  GtkScrolledWindow *scrolled = editor->source_scroll;
  GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment (scrolled);
  
  gdouble value = gtk_adjustment_get_value (vadj);
  gdouble upper = gtk_adjustment_get_upper (vadj);
  gdouble page_size = gtk_adjustment_get_page_size (vadj);
  
  /* Calculate scroll percentage */
  gdouble percentage = 0.0;
  if (upper > page_size) {
    percentage = value / (upper - page_size);
  }
  
  /* Apply percentage to preview */
  gchar *script = g_strdup_printf (
    "var body = document.body, html = document.documentElement; "
    "var height = Math.max(body.scrollHeight, body.offsetHeight, html.clientHeight, html.scrollHeight, html.offsetHeight); "
    "window.scrollTo(0, %f * (height - window.innerHeight));",
    percentage
  );
  
  webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (editor->preview), script, NULL, NULL, NULL);
  g_free (script);
  
  /* Prevent feedback loop */
  editor->scroll_sync_timer = g_timeout_add (100, on_editor_scroll_sync_timeout, editor);
}

static gboolean
file_changed_cb (GFileMonitor      *monior,
                 GFile             *file,
                 GFile             *other_file,
                 GFileMonitorEvent  event_type,
                 gpointer           user_data)
{
  MarkerEditor * editor = MARKER_EDITOR(user_data);
  g_assert (MARKER_IS_EDITOR (editor));
  if (file) {
    g_autofree gchar *file_contents = NULL;
    gsize file_size = 0;
    GError *err = NULL;

    g_file_load_contents (file, NULL, &file_contents, &file_size, NULL, &err);

    if (err)
    {
      g_error_free (err);
    }
    else
    {
      MarkerSourceView *source_view = editor->source_view;
      GtkSourceBuffer *buffer =
        GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));
      gtk_source_buffer_begin_not_undoable_action (buffer);
      marker_source_view_set_text (source_view, file_contents, file_size);
      gtk_source_buffer_end_not_undoable_action (buffer);
    }

    editor->unsaved_changes = FALSE;

  }
  return G_SOURCE_CONTINUE;
}

static void
buffer_changed_cb (GtkTextBuffer *buffer,
                   gpointer user_data)
{
  MarkerEditor *editor = user_data;
  if (editor->view_mode != PREVIEW_ONLY_MODE) {
    editor->unsaved_changes = TRUE;
  }
  editor->needs_refresh = TRUE;
  if (editor->text_iter) {
    editor->text_iter = NULL;
  }
  emit_signal_title_changed (editor);
}

static gboolean
preview_window_closed_cb (GtkWindow *preview_window,
                          GdkEvent  *event,
                          gpointer   user_data)
{
  GtkWidget *preview = user_data;
  g_object_ref (preview);
  gtk_container_remove (GTK_CONTAINER (preview_window), preview);
  gtk_widget_destroy (GTK_WIDGET (preview_window));
  return TRUE;
}

static void
search_text_changed (GtkEntry         *entry,
                     MarkerEditor     *editor)
{
  GtkSourceSearchContext* context = marker_source_get_search_context(editor->source_view);
  GtkSourceSearchSettings * settings = gtk_source_search_context_get_settings(context);
  gtk_source_search_settings_set_search_text(settings, gtk_entry_get_text(entry));
}

static void
search_next     (GtkEntry         *entry,
                 MarkerEditor     *editor)
{
  GtkSourceSearchContext* context = marker_source_get_search_context(editor->source_view);
  GtkTextBuffer * buffer = GTK_TEXT_BUFFER(gtk_source_search_context_get_buffer(context));

  GtkTextIter *iter;
  GtkTextIter close;

  gtk_text_buffer_get_end_iter(buffer, &close);

  if (editor->text_iter)
  {
    iter = editor->text_iter;
    if (gtk_text_iter_compare(iter, &close) == 0)
    {
      gtk_text_buffer_get_start_iter(buffer, iter);
    }
  } else {
    GtkTextIter tmp;
    iter = &tmp;
    gtk_text_buffer_get_start_iter(buffer, iter);
  }


  /** TODO use async forward instead **/
  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_start_iter(buffer, &end);

  gtk_source_search_context_forward2(context, iter, &start, &end, NULL);

  if (gtk_text_iter_compare(&start, &end) != 0){
    gtk_text_buffer_select_range(buffer, &start, &end);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(editor->source_view), &start, 0, TRUE, 0, 0);
    editor->text_iter = gtk_text_iter_copy(&end);
  } else {
    gtk_text_buffer_get_start_iter(buffer, &start);
    editor->text_iter = gtk_text_iter_copy(&start);
  }
}


static void
search_previous     (GtkEntry         *entry,
                     MarkerEditor     *editor)
{

  GtkSourceSearchContext* context = marker_source_get_search_context(editor->source_view);
  GtkTextBuffer * buffer = GTK_TEXT_BUFFER(gtk_source_search_context_get_buffer(context));

  GtkTextIter *iter;
  GtkTextIter close;

  gtk_text_buffer_get_start_iter(buffer, &close);

  if (editor->text_iter)
  {
    iter = editor->text_iter;
    if (gtk_text_iter_compare(iter, &close) == 0)
    {
      gtk_text_buffer_get_end_iter(buffer, iter);
    }
  } else {
    GtkTextIter tmp;
    iter = &tmp;
    gtk_text_buffer_get_start_iter(buffer,iter);
  }

  /** TODO use async forward instead **/
  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_start_iter(buffer, &end);

  gtk_source_search_context_backward2(context, iter, &start, &end, NULL);

  if (gtk_text_iter_compare(&start, &end) != 0){
    gtk_text_buffer_select_range(buffer, &start, &end);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(editor->source_view), &start, 0, TRUE, 0, 0);
    editor->text_iter = gtk_text_iter_copy(&start);
  } else {
    gtk_text_buffer_get_end_iter(buffer, &start);
    editor->text_iter = gtk_text_iter_copy(&start);
  }
}

static void
marker_editor_init (MarkerEditor *editor)
{
  editor->file = NULL;
  editor->title = g_strdup(_("Untitled.md"));
  editor->unsaved_changes = FALSE;
  editor->search_active = FALSE;
  editor->text_iter = NULL;
  editor->scroll_sync_enabled = marker_prefs_get_enable_scroll_sync();
  editor->scroll_sync_timer = 0;

  editor->paned = GTK_PANED (gtk_paned_new (GTK_ORIENTATION_HORIZONTAL));
  editor->vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));

  /** SEARCH TOOL BAR **/
  editor->search_entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());

  GtkSearchBar * sbar = GTK_SEARCH_BAR(gtk_search_bar_new());
  editor->search_bar = sbar;

  gtk_container_add(GTK_CONTAINER(sbar), GTK_WIDGET(editor->search_entry));

  gtk_box_pack_start(editor->vbox, GTK_WIDGET(sbar), FALSE, TRUE, 0);

  gtk_widget_show_all(GTK_WIDGET(sbar));

  gtk_search_bar_set_search_mode(sbar, FALSE);
  gtk_search_bar_set_show_close_button(sbar, TRUE);

  g_signal_connect(editor->search_entry,
                   "search-changed",
                   G_CALLBACK(search_text_changed),
                   editor);

  g_signal_connect(editor->search_entry,
                   "activate",
                   G_CALLBACK(search_next),
                   editor);

  g_signal_connect(editor->search_entry,
                   "next-match",
                   G_CALLBACK(search_next),
                   editor);


  g_signal_connect(editor->search_entry,
                   "previous-match",
                   G_CALLBACK(search_previous),
                   editor);
  /** DONE **/

  gtk_paned_set_position (editor->paned, 450);
  gtk_widget_show (GTK_WIDGET (editor->paned));
  gtk_box_pack_start (GTK_BOX (editor), GTK_WIDGET (editor->paned), TRUE, TRUE, 0);

  editor->preview = marker_preview_new ();
  gtk_widget_show (GTK_WIDGET (editor->preview));

  editor->source_view = marker_source_view_new ();
  gtk_widget_show (GTK_WIDGET (editor->source_view));
  editor->source_scroll = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
  gtk_widget_show (GTK_WIDGET (editor->source_scroll));
  gtk_container_add (GTK_CONTAINER (editor->source_scroll), GTK_WIDGET (editor->source_view));
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->source_view));
  g_signal_connect (buffer, "changed", G_CALLBACK (buffer_changed_cb), editor);
  
  /* Connect scroll sync handler */
  GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment (editor->source_scroll);
  g_signal_connect (vadj, "value-changed", G_CALLBACK (on_editor_scroll_event), editor);

  gtk_box_pack_end(editor->vbox, GTK_WIDGET(editor->source_scroll), TRUE, TRUE, 0);
  gtk_widget_show(GTK_WIDGET (editor->vbox));
  gtk_widget_show(GTK_WIDGET (editor->search_entry));

  editor->view_mode = marker_prefs_get_default_view_mode ();
  editor->needs_refresh = FALSE;

  marker_editor_set_view_mode (editor, editor->view_mode);
  gtk_widget_show (GTK_WIDGET (editor));

  editor->timer_id = g_timeout_add (20, refresh_timeout_cb, editor);

  marker_editor_apply_prefs (editor);
}

static void
marker_editor_class_init (MarkerEditorClass *class)
{
  g_signal_new ("title-changed",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                0, NULL, NULL, NULL,
                G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  g_signal_new ("subtitle-changed",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                0, NULL, NULL, NULL,
                G_TYPE_NONE, 1, G_TYPE_STRING);
}

MarkerEditor *
marker_editor_new (void)
{
  return g_object_new (MARKER_TYPE_EDITOR,
                       "orientation", GTK_ORIENTATION_VERTICAL,
                       "spacing",     0,
                       NULL);
}

MarkerEditor *
marker_editor_new_from_file (GFile *file)
{
  MarkerEditor *editor = g_object_new (MARKER_TYPE_EDITOR,
                                       "orientation", GTK_ORIENTATION_VERTICAL,
                                       "spacing", 0, NULL);
  marker_editor_open_file (editor, file);
  return editor;
}

void
marker_editor_refresh_preview (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  editor->needs_refresh = FALSE;

  g_autofree gchar *markdown = marker_source_view_get_text (editor->source_view, false);
  int cursor = marker_source_view_get_cursor_position(editor->source_view);

  const char* css_theme = (marker_prefs_get_use_css_theme()) ? marker_prefs_get_css_theme() : NULL;
  g_autofree gchar *uri = (G_IS_FILE(editor->file)) ? g_file_get_path(editor->file) : NULL;

  marker_preview_render_markdown(editor->preview, markdown, css_theme, uri, cursor);
}

MarkerViewMode
marker_editor_get_view_mode (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->view_mode;
}

void
marker_editor_set_view_mode (MarkerEditor   *editor,
                             MarkerViewMode  view_mode)
{
  g_assert (MARKER_IS_EDITOR (editor));

  if (editor->view_mode  != view_mode){
    if (view_mode == PREVIEW_ONLY_MODE && editor->file)
    {
      editor->file_monitor = g_file_monitor_file(editor->file, G_FILE_MONITOR_NONE, NULL, NULL);
      g_signal_connect(editor->file_monitor, "changed", G_CALLBACK(file_changed_cb), editor);

    } else if (editor->view_mode == PREVIEW_ONLY_MODE && editor->file)
    {
      g_file_monitor_cancel(editor->file_monitor);
    }
  }
  editor->view_mode = view_mode;

  GtkWidget * const paned = GTK_WIDGET (editor->paned);
  GtkWidget * const preview = GTK_WIDGET (editor->preview);
  GtkWidget * const source_scroll = GTK_WIDGET (editor->vbox);
  GtkContainer *parent;

  parent = GTK_CONTAINER (gtk_widget_get_parent (preview));
  if (parent)
  {
    g_object_ref (preview);
    gtk_container_remove (parent, preview);

    if (GTK_IS_WINDOW (parent))
    {
      gtk_widget_destroy (GTK_WIDGET (parent));
    }
  }

  parent = GTK_CONTAINER (gtk_widget_get_parent (source_scroll));
  if (parent)
  {
    g_object_ref (source_scroll);
    gtk_container_remove (parent, source_scroll);
  }

  switch (view_mode)
  {
    case EDITOR_ONLY_MODE:
      gtk_paned_add1 (GTK_PANED (paned), source_scroll);
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
      break;

    case PREVIEW_ONLY_MODE:
      gtk_paned_add2 (GTK_PANED (paned), preview);
      gtk_widget_grab_focus (GTK_WIDGET (preview));
      break;

    case DUAL_PANE_MODE:
      gtk_paned_add1 (GTK_PANED (paned), source_scroll);
      gtk_paned_add2 (GTK_PANED (paned), preview);
      // load saved pane width from the preferences
      guint pane_width = marker_prefs_get_editor_pane_width ();
      /* Ensure minimum width of 300 to prevent invisible editor */
      if (pane_width == 0 || pane_width < 300) {
        pane_width = 450; /* Use default width */
      }
      gtk_paned_set_position (GTK_PANED (paned), pane_width);
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
      break;

    case DUAL_WINDOW_MODE:
      gtk_paned_add1(GTK_PANED(paned), source_scroll);

      GtkWindow *preview_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
      g_signal_connect(preview_window, "delete-event", G_CALLBACK (preview_window_closed_cb), preview);
      gtk_container_add (GTK_CONTAINER (preview_window), preview);
      gtk_window_set_title (preview_window, "Preview");
      gtk_window_set_default_size (preview_window, 500, 600);
      gtk_widget_show_all (GTK_WIDGET (preview_window));
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
      break;
  }

}


guint
marker_editor_get_pane_width (MarkerEditor *editor)
{
  // editor pane width is valid only for dual pane mode
  if(marker_editor_get_view_mode (editor) == DUAL_PANE_MODE)
  {
    return gtk_paned_get_position (GTK_PANED (editor->paned));
  }
  else
  {
    // return last saved value
    return marker_prefs_get_editor_pane_width ();
  }
}

void
marker_editor_open_file (MarkerEditor *editor,
                        GFile        *file)
{
  g_assert (MARKER_IS_EDITOR (editor));

  if (G_IS_FILE (editor->file))
    g_object_unref (editor->file);

  editor->file = file;
  g_object_ref (file);
  editor->title = g_file_get_basename(file);
  editor->needs_refresh = TRUE;

  g_autofree gchar *file_contents = NULL;
  gsize file_size = 0;
  GError *err = NULL;

  g_file_load_contents (file, NULL, &file_contents, &file_size, NULL, &err);

  if (err)
  {
    g_error_free (err);
  }
  else
  {
    MarkerSourceView *source_view = editor->source_view;
    GtkSourceBuffer *buffer =
      GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));
    gtk_source_buffer_begin_not_undoable_action (buffer);
    marker_source_view_set_text (source_view, file_contents, file_size);
    gtk_source_buffer_end_not_undoable_action (buffer);
  }

  editor->unsaved_changes = FALSE;

  emit_signal_title_changed (editor);
  emit_signal_subtitle_changed (editor);
}

void
marker_editor_save_file (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  g_return_if_fail (G_IS_FILE (editor->file));

  g_autofree gchar *filepath = g_file_get_path (editor->file);

  FILE *fp = fopen (filepath, "w");

  if (fp)
  {
    g_autofree gchar *buffer = marker_source_view_get_text (editor->source_view, false);
    
    /* Check if we need to add a trailing newline */
    if (marker_prefs_get_add_trailing_newline ())
    {
      gsize len = strlen (buffer);
      if (len > 0 && buffer[len - 1] != '\n')
      {
        /* Add trailing newline */
        gchar *new_buffer = g_strdup_printf ("%s\n", buffer);
        fputs (new_buffer, fp);
        g_free (new_buffer);
      }
      else
      {
        fputs (buffer, fp);
      }
    }
    else
    {
      fputs (buffer, fp);
    }
    
    fclose (fp);
  }

  editor->unsaved_changes = FALSE;
  emit_signal_title_changed (editor);
}

void
marker_editor_save_file_as (MarkerEditor *editor,
                            GFile        *file)
{
  g_assert (MARKER_IS_EDITOR (editor));

  if (G_IS_FILE (editor->file))
  {
    g_object_unref (editor->file);
  }
  editor->title = g_file_get_basename(file);
  editor->file = file;
  marker_editor_save_file (editor);
  emit_signal_subtitle_changed (editor);
}

GFile *
marker_editor_get_file (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  if (G_IS_FILE (editor->file))
  {
    return editor->file;
  }
  return NULL;
}

gboolean
marker_editor_has_unsaved_changes (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->unsaved_changes;
}

gchar *
marker_editor_get_title (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  gchar *title = NULL;

  if (marker_editor_has_unsaved_changes (editor))
  {
    title = g_strdup_printf ("*%s", editor->title);
  }
  else
  {
    title = g_strdup(editor->title);
  }

  return title;
}

gchar *
marker_editor_get_raw_title (MarkerEditor *editor)
{
  g_assert(MARKER_IS_EDITOR(editor));
  return g_strdup(editor->title);
}

gchar *
marker_editor_get_subtitle (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  gchar *subtitle = NULL;
  GFile *file = marker_editor_get_file (editor);

  if (G_IS_FILE (file))
  {
    g_autofree gchar *path = g_file_get_path (file);
    subtitle = marker_string_filename_get_path (path);
  }

  return subtitle;
}

MarkerPreview *
marker_editor_get_preview (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->preview;
}

MarkerSourceView *
marker_editor_get_source_view (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  return editor->source_view;
}

void
marker_editor_apply_prefs (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));

  GtkSourceView * const source_view = GTK_SOURCE_VIEW (marker_editor_get_source_view (editor));

  gboolean state;

  state = marker_prefs_get_show_line_numbers ();
  gtk_source_view_set_show_line_numbers (source_view, state);

  state = marker_prefs_get_wrap_text ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (source_view), (state) ? GTK_WRAP_WORD : GTK_WRAP_NONE );

  state = marker_prefs_get_show_right_margin ();
  gtk_source_view_set_show_right_margin (source_view, state);

  guint position = marker_prefs_get_right_margin_position ();
  gtk_source_view_set_right_margin_position (source_view, position);

  state = marker_prefs_get_spell_check ();
  marker_source_view_set_spell_check (MARKER_SOURCE_VIEW (source_view), state);

  g_autofree gchar *lang = marker_prefs_get_spell_check_language ();
  marker_source_view_set_spell_check_lang (MARKER_SOURCE_VIEW (source_view), lang);

  state = marker_prefs_get_highlight_current_line ();
  gtk_source_view_set_highlight_current_line (source_view, state);

  GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view)));
  state = marker_prefs_get_use_syntax_theme ();
  gtk_source_buffer_set_highlight_syntax(buffer, state);

  g_autofree gchar *theme = marker_prefs_get_syntax_theme ();
  marker_source_view_set_syntax_theme (MARKER_SOURCE_VIEW (source_view), theme);

  state = marker_prefs_get_auto_indent ();
  gtk_source_view_set_auto_indent (source_view, state);

  state = marker_prefs_get_replace_tabs ();
  gtk_source_view_set_insert_spaces_instead_of_tabs (source_view, state);

  state = marker_prefs_get_show_spaces();
  GtkSourceSpaceDrawer * space_drawer = gtk_source_view_get_space_drawer(source_view);
  if (state) {
    gtk_source_space_drawer_set_types_for_locations(space_drawer, GTK_SOURCE_SPACE_LOCATION_ALL,
                                                    GTK_SOURCE_SPACE_TYPE_NBSP |
                                                    GTK_SOURCE_SPACE_TYPE_SPACE |
                                                    GTK_SOURCE_SPACE_TYPE_TAB);
  }
  gtk_source_space_drawer_set_enable_matrix(space_drawer, state);

  guint tab_width = marker_prefs_get_tab_width ();
  gtk_source_view_set_indent_width (source_view, tab_width);
  
  /* Update font size */
  marker_source_view_update_font (MARKER_SOURCE_VIEW (source_view));
}


void
marker_editor_closing(MarkerEditor       *editor)
{
  g_source_remove (editor->timer_id);
  editor->needs_refresh = FALSE;
}

gboolean
marker_editor_rename_file (MarkerEditor *editor,
                           gchar*        name)
{
  if (G_IS_FILE (editor->file))
  {
    /** Get new path **/
    gchar * parent = marker_editor_get_subtitle(editor);
    gchar * path =  g_strdup_printf("%s/%s",
                    parent,
                    name);

    /** Delete old file **/
    g_file_delete(editor->file,
                  NULL,
                  NULL);

    /** Save file in new destination **/
    GFile * file = g_file_new_for_path(path);
    marker_editor_save_file_as(editor, file);
    g_free(parent);
    g_free(path);
  }else {
    /** Just change the title... **/
    editor->title = name;
  }
  return TRUE;
}


void
marker_editor_toggle_search_bar (MarkerEditor       *editor)
{
  if (!editor->search_active){
    editor->search_active = TRUE;
  } else {
    gtk_entry_set_text(GTK_ENTRY(editor->search_entry), "");
    if (editor->text_iter)
    {
      g_free(editor->text_iter);
      editor->text_iter = NULL;
    }
  }

  gtk_search_bar_set_search_mode (editor->search_bar, editor->search_active);
}

GtkSearchBar*
marker_editor_get_search_bar (MarkerEditor       *editor)
{
  return editor->search_bar;
}

void
marker_editor_set_scroll_sync (MarkerEditor *editor,
                               gboolean      enabled)
{
  g_assert (MARKER_IS_EDITOR (editor));
  editor->scroll_sync_enabled = enabled;
  marker_prefs_set_enable_scroll_sync (enabled);
}

void
marker_editor_go_to_line (MarkerEditor *editor)
{
  g_assert (MARKER_IS_EDITOR (editor));
  
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *entry;
  GtkWidget *label;
  GtkWidget *hbox;
  
  /* Create the dialog */
  dialog = gtk_dialog_new_with_buttons (_("Go to Line"),
                                        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (editor))),
                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL,
                                        _("_Go"), GTK_RESPONSE_OK,
                                        NULL);
  
  /* Set default response */
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  /* Create content */
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);
  
  label = gtk_label_new (_("Line number:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  
  entry = gtk_entry_new ();
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 10);
  gtk_entry_set_input_purpose (GTK_ENTRY (entry), GTK_INPUT_PURPOSE_NUMBER);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  
  /* Get current line number for reference */
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->source_view));
  GtkTextMark *insert_mark = gtk_text_buffer_get_insert (buffer);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_mark (buffer, &iter, insert_mark);
  gint current_line = gtk_text_iter_get_line (&iter) + 1;
  gint total_lines = gtk_text_buffer_get_line_count (buffer);
  
  /* Set placeholder text showing current position */
  gchar *placeholder = g_strdup_printf (_("Current: %d of %d"), current_line, total_lines);
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry), placeholder);
  g_free (placeholder);
  
  /* Activate default response on Enter in entry */
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  
  gtk_widget_show_all (dialog);
  
  /* Focus the entry */
  gtk_widget_grab_focus (entry);
  
  /* Run the dialog */
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
  {
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
    gint line_number = atoi (text);
    
    if (line_number > 0)
    {
      /* Move to the specified line */
      GtkTextIter target;
      
      if (line_number > total_lines)
      {
        /* Go to end of document if line number exceeds total lines */
        gtk_text_buffer_get_end_iter (buffer, &target);
      }
      else
      {
        /* Go to the specified line */
        gtk_text_buffer_get_iter_at_line (buffer, &target, line_number - 1);
      }
      
      gtk_text_buffer_place_cursor (buffer, &target);
      
      /* Scroll to make the line visible */
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor->source_view), &target, 0.25, FALSE, 0.0, 0.5);
      
      /* Give focus back to the editor */
      gtk_widget_grab_focus (GTK_WIDGET (editor->source_view));
    }
  }
  
  gtk_widget_destroy (dialog);
}
