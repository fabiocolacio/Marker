/*
 * marker-exporter.c
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib/gi18n.h>

#include "marker.h"
#include "marker-utils.h"
#include "marker-string.h"
#include "marker-markdown.h"
#include "marker-prefs.h"
#include "marker-preview.h"
#include "marker-editor.h"

#include "marker-exporter.h"

static MarkerExportFormat
format_from_filename (const gchar *filename)
{
  g_autofree gchar *lower = g_ascii_strdown (filename, -1);

  if (g_str_has_suffix (lower, ".pdf"))  return PDF;
  if (g_str_has_suffix (lower, ".rtf"))  return RTF;
  if (g_str_has_suffix (lower, ".odt"))  return ODT;
  if (g_str_has_suffix (lower, ".docx")) return DOCX;
  if (g_str_has_suffix (lower, ".tex"))  return LATEX;
  return HTML;
}

void
marker_exporter_export_pandoc (const char *markdown,
                               const char *stylesheet_path,
                               const char *outfile)
{
  const char *ftmp = ".marker_tmp_markdown.md";
  g_autofree char *path = marker_string_filename_get_path (outfile);
  if (chdir (path) == 0)
  {
    FILE *fp = fopen (ftmp, "w");
    if (fp)
    {
      fputs (markdown, fp);
      fclose (fp);

      g_autofree gchar *command = g_strdup_printf ("pandoc -s -c \"%s\" -o \"%s\" \"%s\"",
                                                   stylesheet_path, outfile, ftmp);
      if (system (command) != 0)
        g_warning ("pandoc export failed: %s", command);

      remove (ftmp);
    }
  }
}

static void
export_document (MarkerWindow *window,
                 const gchar  *filename)
{
  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
    return;

  MarkerPreview *preview = marker_editor_get_preview (editor);
  MarkerSourceView *source_view = marker_editor_get_source_view (editor);

  g_autofree gchar *stylesheet_path = marker_prefs_get_css_theme ();
  g_autofree gchar *markdown = marker_source_view_get_text (source_view, FALSE);

  GFile *source = marker_editor_get_file (editor);
  g_autofree char *base_folder = NULL;
  if (source)
  {
    g_autoptr (GFile) parent = g_file_get_parent (source);
    base_folder = parent ? g_file_get_path (parent) : NULL;
  }

  size_t len = strlen (markdown);
  metadata *meta = marker_markdown_metadata (markdown, len);
  if (!meta)
  {
    g_warning ("marker-exporter: document metadata is NULL");
    return;
  }

  enum scidown_paper_size paper_size = meta->paper_size;
  if (meta->doc_class == CLASS_BEAMER && !(paper_size == B43 || paper_size == B169))
    paper_size = B43;

  GtkPageOrientation orientation = meta->doc_class == CLASS_BEAMER
    ? GTK_PAGE_ORIENTATION_LANDSCAPE
    : GTK_PAGE_ORIENTATION_PORTRAIT;

  MarkerMathJSMode mathjs = marker_prefs_get_use_mathjs () ? MATHJS_NET : MATHJS_OFF;
  MarkerHighlightMode highlight = marker_prefs_get_use_highlight () ? HIGHLIGHT_NET : HIGHLIGHT_OFF;
  MarkerMermaidMode mermaid = marker_prefs_get_use_mermaid () ? MERMAID_NET : MERMAID_OFF;

  switch (format_from_filename (filename))
  {
    case HTML:
      marker_markdown_to_html_file_with_css_inline (markdown, len, base_folder,
                                                    mathjs, highlight, mermaid,
                                                    stylesheet_path, filename);
      break;

    case PDF:
      marker_preview_print_pdf (preview, filename, paper_size, orientation);
      break;

    case LATEX:
      marker_markdown_to_latex_file (markdown, len, base_folder,
                                     mathjs, highlight, mermaid, filename);
      break;

    default:
    {
      char *html = marker_markdown_to_html_with_css_inline (markdown, len, base_folder,
                                                            mathjs, highlight, mermaid,
                                                            stylesheet_path, -1);
      marker_exporter_export_pandoc (html, stylesheet_path, filename);
      free (html);
    }
  }
}

static void
add_filter (GListStore  *filters,
            const gchar *name,
            const gchar *suffix)
{
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, name);
  gtk_file_filter_add_suffix (filter, suffix);
  g_list_store_append (filters, filter);
  g_object_unref (filter);
}

static void
export_save_cb (GObject      *source,
                GAsyncResult *result,
                gpointer      user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  g_autoptr (GFile) file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (source), result, NULL);

  if (file)
  {
    g_autofree gchar *filename = g_file_get_path (file);
    if (filename)
      export_document (window, filename);
  }

  g_object_unref (window);
}

void
marker_exporter_show_export_dialog (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
    return;

  g_autoptr (GtkFileDialog) dialog = gtk_file_dialog_new ();
  gtk_file_dialog_set_title (dialog, _("Export"));
  gtk_file_dialog_set_accept_label (dialog, _("_Export"));

  g_autoptr (GListStore) filters = g_list_store_new (GTK_TYPE_FILE_FILTER);
  add_filter (filters, "HTML", "html");
  add_filter (filters, "PDF", "pdf");

  g_autofree gchar *pandoc_path = g_find_program_in_path ("pandoc");
  if (pandoc_path != NULL)
  {
    add_filter (filters, "RTF", "rtf");
    add_filter (filters, "DOCX", "docx");
    add_filter (filters, "ODT", "odt");
  }

  add_filter (filters, "LaTeX", "tex");

  gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));
  g_autoptr (GtkFileFilter) default_filter = g_list_model_get_item (G_LIST_MODEL (filters), 0);
  gtk_file_dialog_set_default_filter (dialog, default_filter);

  /* Suggest <document>.html next to the source file */
  g_autofree gchar *raw_title = marker_editor_get_raw_title (editor);
  g_autofree gchar *stem = marker_string_filename_get_name_noext (raw_title);
  g_autofree gchar *initial_name = g_strdup_printf ("%s.html", (stem && *stem) ? stem : "document");
  gtk_file_dialog_set_initial_name (dialog, initial_name);

  GFile *file = marker_editor_get_file (editor);
  if (file)
  {
    g_autoptr (GFile) parent = g_file_get_parent (file);
    if (parent)
      gtk_file_dialog_set_initial_folder (dialog, parent);
  }

  gtk_file_dialog_save (dialog, GTK_WINDOW (window), NULL, export_save_cb, g_object_ref (window));
}

void
marker_exporter_export (const gchar *infile,
                        const gchar *outfile)
{
  g_return_if_fail (infile != NULL && outfile != NULL);

  long len = 0;
  g_autofree gchar *markdown = marker_utils_read_file (infile, &len);
  g_autofree gchar *stylesheet = marker_prefs_get_css_theme ();
  g_autofree gchar *base_folder = marker_string_filename_get_path (infile);

  if (!markdown)
  {
    g_printerr ("Unable to read %s\n", infile);
    return;
  }

  MarkerMathJSMode mathjs = marker_prefs_get_use_mathjs () ? MATHJS_NET : MATHJS_OFF;
  MarkerHighlightMode highlight = marker_prefs_get_use_highlight () ? HIGHLIGHT_NET : HIGHLIGHT_OFF;
  MarkerMermaidMode mermaid = marker_prefs_get_use_mermaid () ? MERMAID_NET : MERMAID_OFF;

  if (marker_string_ends_with (outfile, ".html")) {
    marker_markdown_to_html_file_with_css_inline (markdown, len, base_folder,
                                                  mathjs, highlight, mermaid,
                                                  stylesheet, outfile);
  }
  else if (marker_string_ends_with (outfile, ".pdf")) {
    g_printerr ("PDF export from the command line is not supported; use Export… in the app.\n");
  }
  else if (marker_string_ends_with (outfile, ".tex")) {
    marker_markdown_to_latex_file (markdown, len, base_folder,
                                   mathjs, highlight, mermaid, outfile);
  }
  else {
    marker_exporter_export_pandoc (markdown, stylesheet, outfile);
  }
}
