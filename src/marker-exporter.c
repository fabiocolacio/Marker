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

#include "marker-string.h"
#include "marker-markdown.h"
#include "marker-prefs.h"
#include "marker-preview.h"
#include "marker-editor.h"

#include "marker-exporter.h"

MarkerExportFormat
marker_exporter_str_to_fmt(const char* str)
{
  if (strcmp(str, "PDF") == 0)
  {
    return PDF;
  }

  if (strcmp(str, "RTF") == 0)
  {
    return RTF;
  }

  if (strcmp(str, "ODT") == 0)
  {
    return ODT;
  }

  if (strcmp(str, "DOCX") == 0)
  {
    return DOCX;
  }

  if (strcmp(str, "LATEX") == 0)
  {
    return LATEX;
  }

  return HTML;
}

void
marker_exporter_export_pandoc(const char*        tex,
                              const char*        stylesheet_path,
                              const char*        outfile,
                              MarkerExportFormat format)
{
  const char* ftmp = ".marker_tmp_markdown.html";
  char* path = marker_string_filename_get_path(outfile);
  if (chdir(path) == 0)
  {
    FILE* fp = NULL;
    fp = fopen(ftmp, "w");
    if (fp)
    {
      fputs(tex, fp);
      fclose(fp);

      // const char* format_s = "rtf";
      // switch (format)
      // {
      //   case RTF:
      //     format_s = "rtf";
      //     break;

      //   case DOCX:
      //     format_s = "docx";
      //     break;

      //   case ODT:
      //     format_s = "odt";
      //     break;

      //   case PDF:
      //     format_s = "pdf";
      //     break;

      //   default:
      //     break;
      // }

      char* command = NULL;

      asprintf(&command,
               "pandoc -s -c %s -o %s %s",
               stylesheet_path,
               outfile,
               ftmp);

      if (command)
      {
        system(command);
      }

      free(command);
      remove(ftmp);
    }
  }
  free(path);
}

void
marker_exporter_show_export_dialog(MarkerWindow* window)
{
  GtkDialog *dialog = GTK_DIALOG(gtk_file_chooser_dialog_new ("Export",
                                                              GTK_WINDOW(window),
                                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                                              "Cancel", GTK_RESPONSE_CANCEL,
                                                              "Export", GTK_RESPONSE_ACCEPT,
                                                              NULL));

  GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
  gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
  gtk_file_chooser_set_create_folders (chooser, TRUE);
  gtk_file_chooser_set_select_multiple (chooser, FALSE);

  GtkFileFilter *filter = NULL;

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "HTML");
  gtk_file_filter_add_pattern (filter, "*.html");
  gtk_file_chooser_add_filter (chooser, filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "PDF");
  gtk_file_filter_add_pattern (filter, "*.pdf");
  gtk_file_chooser_add_filter (chooser, filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "RTF");
  gtk_file_filter_add_pattern (filter, "*.rtf");
  gtk_file_chooser_add_filter (chooser, filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "DOCX");
  gtk_file_filter_add_pattern (filter, "*.docx");
  gtk_file_chooser_add_filter (chooser, filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "ODT");
  gtk_file_filter_add_pattern (filter, "*.odt");
  gtk_file_chooser_add_filter (chooser, filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "LATEX");
  gtk_file_filter_add_pattern (filter, "*.tex");
  gtk_file_chooser_add_filter (chooser, filter);

  filter = NULL;

  gint ret = gtk_dialog_run (dialog);
  if (ret == GTK_RESPONSE_ACCEPT)
  {
    g_autofree gchar *filename = gtk_file_chooser_get_filename(chooser);
    g_autofree gchar *stylesheet_path = marker_prefs_get_css_theme();
    g_autofree gchar *markdown = NULL;

    filter = gtk_file_chooser_get_filter(chooser);
    const gchar* file_type = gtk_file_filter_get_name(filter);
    MarkerExportFormat fmt = marker_exporter_str_to_fmt(file_type);


    MarkerEditor *editor = marker_window_get_active_editor (window);
    MarkerPreview *preview = marker_editor_get_preview(editor);

    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    markdown = marker_source_view_get_text (source_view);

    GFile * source = marker_editor_get_file(editor);
    char * base_folder = NULL;

    if (source)
      base_folder = g_file_get_path(g_file_get_parent(source));

    switch (fmt)
    {
      case HTML:
        marker_markdown_to_html_file_with_css_inline(markdown,
                                                     strlen(markdown),
                                                     base_folder,
                                                     (marker_prefs_get_use_katex())
                                                       ? KATEX_NET
                                                       : KATEX_OFF,
                                                     (marker_prefs_get_use_highlight())
                                                       ? HIGHLIGHT_NET
                                                       : HIGHLIGHT_OFF,
                                                     (marker_prefs_get_use_mermaid()
                                                       ? MERMAID_NET
                                                       : MERMAID_OFF),
                                                     stylesheet_path,
                                                     filename);
        break;

      case PDF:
        marker_preview_print_pdf(preview, filename);
        break;

      case LATEX:
        marker_markdown_to_latex_file(markdown,
                                      strlen(markdown),
                                      base_folder,
                                      (marker_prefs_get_use_katex())
                                        ? KATEX_NET
                                        : KATEX_OFF,
                                      (marker_prefs_get_use_highlight())
                                        ? HIGHLIGHT_NET
                                        : HIGHLIGHT_OFF,
                                      (marker_prefs_get_use_mermaid()
                                        ? MERMAID_NET
                                        : MERMAID_OFF),
                                      filename);
        break;

      default:
        marker_exporter_export_pandoc(marker_markdown_to_html_with_css_inline(markdown,
                                                                              strlen(markdown),
                                                                              base_folder,
                                                                              (marker_prefs_get_use_katex())
                                                                                ? KATEX_NET
                                                                                : KATEX_OFF,
                                                                              (marker_prefs_get_use_highlight())
                                                                                ? HIGHLIGHT_NET
                                                                                : HIGHLIGHT_OFF,
                                                                              (marker_prefs_get_use_mermaid()
                                                                                ? MERMAID_NET
                                                                                : MERMAID_OFF),
                                                                              stylesheet_path),
                                      stylesheet_path,
                                      filename,
                                      fmt);
        break;
    }
  }

  gtk_widget_destroy(GTK_WIDGET(dialog));
}

