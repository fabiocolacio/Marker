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

void
marker_exporter_export_pandoc(const char*        tex,
                              const char*        stylesheet_path,
                              const char*        outfile)
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
  g_assert (MARKER_IS_WINDOW (window));

  g_autoptr (GtkFileChooserNative) dialog =
    gtk_file_chooser_native_new ("Export",
                                 GTK_WINDOW (window),
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 "_Export", "_Cancel");

  GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
  gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
  gtk_file_chooser_set_create_folders (chooser, TRUE);
  gtk_file_chooser_set_select_multiple (chooser, FALSE);
  
  gint response = gtk_native_dialog_run (GTK_NATIVE_DIALOG (dialog));
  if (response == GTK_RESPONSE_ACCEPT)
  {
    g_autofree gchar *filename = gtk_file_chooser_get_filename(chooser);
    g_autofree gchar *stylesheet_path = marker_prefs_get_css_theme();
    g_autofree gchar *markdown = NULL;

    MarkerEditor *editor = marker_window_get_active_editor (window);
    MarkerPreview *preview = marker_editor_get_preview(editor);

    MarkerSourceView *source_view = marker_editor_get_source_view (editor);
    markdown = marker_source_view_get_text (source_view);

    GFile * source = marker_editor_get_file(editor);
    char * base_folder = NULL;

    if (source)
      base_folder = g_file_get_path(g_file_get_parent(source));
    size_t len = strlen(markdown);
    metadata * meta = marker_markdown_metadata(markdown, len);
    scidow_paper_size paper_size = meta->paper_size;
    if (meta->doc_class == CLASS_BEAMER && !(paper_size == B43 || paper_size == B169))
      paper_size = B43;

    if (!meta) {
      fprintf(stderr, "marker-exporter.c#show_export_dialog: Document Metadata NULL!\n");
      return;
    }
    GtkPageOrientation orientation = meta->doc_class == CLASS_BEAMER ? GTK_PAGE_ORIENTATION_LANDSCAPE : GTK_PAGE_ORIENTATION_PORTRAIT;

    if (marker_string_ends_with (filename, ".html") || marker_string_ends_with (filename, ".htm"))
    {
      marker_markdown_to_html_file_with_css_inline(markdown,
                                                   len,
                                                   base_folder,
                                                   (marker_prefs_get_use_mathjs())
                                                     ? MATHJS_NET
                                                     : MATHJS_OFF,
                                                   (marker_prefs_get_use_highlight())
                                                     ? HIGHLIGHT_NET
                                                     : HIGHLIGHT_OFF,
                                                   (marker_prefs_get_use_mermaid()
                                                     ? MERMAID_NET
                                                     : MERMAID_OFF),
                                                   stylesheet_path,
                                                   filename);
    }
    else if (marker_string_ends_with (filename, ".pdf"))
    {
      marker_preview_print_pdf(preview, filename, paper_size, orientation);
    }
    else if (marker_string_ends_with (filename, ".tex"))
    {
      marker_markdown_to_latex_file(markdown,
                                    len,
                                    base_folder,
                                    (marker_prefs_get_use_mathjs())
                                      ? MATHJS_NET
                                      : MATHJS_OFF,
                                    (marker_prefs_get_use_highlight())
                                      ? HIGHLIGHT_NET
                                      : HIGHLIGHT_OFF,
                                    (marker_prefs_get_use_mermaid()
                                      ? MERMAID_NET
                                      : MERMAID_OFF),
                                    filename);
      }
      else
      {
        marker_exporter_export_pandoc(marker_markdown_to_html_with_css_inline(markdown,
                                                                              len,
                                                                              base_folder,
                                                                              (marker_prefs_get_use_mathjs())
                                                                                ? MATHJS_NET
                                                                                : MATHJS_OFF,
                                                                              (marker_prefs_get_use_highlight())
                                                                                ? HIGHLIGHT_NET
                                                                                : HIGHLIGHT_OFF,
                                                                              (marker_prefs_get_use_mermaid()
                                                                                ? MERMAID_NET
                                                                                : MERMAID_OFF),
                                                                              stylesheet_path),
                                      stylesheet_path,
                                      filename);
    }
  }
}

