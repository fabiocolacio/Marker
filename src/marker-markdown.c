/*
 * marker-markdown.c
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
#include <stdio.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "scidown/src/html.h"
#include "scidown/src/latex.h"
#include "scidown/src/document.h"
#include "scidown/src/buffer.h"

#include "marker-markdown.h"
#include "marker-prefs.h"

char* html_header(MarkerKaTeXMode     katex_mode,
                  MarkerHighlightMode highlight_mode,
                  MarkerMermaidMode   mermaid_mode)
{
  char* katex_script;
  char* katex_auto;
  char* katex_css;

  char* highlight_css;
  char* highlight_script;

  char* mermaid_script;

  char * local_highlight_css = marker_prefs_get_highlight_theme();

  switch (katex_mode) {
    case KATEX_OFF:
      katex_script = g_strdup(" ");
      katex_css = g_strdup(" ");
      katex_auto = g_strdup(" ");
      break;
    case KATEX_NET:
      katex_css = g_strdup("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/katex.min.css\" crossorigin=\"anonymous\">");
      katex_script = g_strdup("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/katex.min.js\" crossorigin=\"anonymous\"></script>");
      katex_auto = g_strdup("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/contrib/auto-render.min.js\" crossorigin=\"anonymous\"></script>");
      break;
    case KATEX_LOCAL:
   	  katex_css = g_strdup_printf("<link rel=\"stylesheet\" href=\"file://%skatex/katex.min.css\">", SCRIPTS_DIR);
      katex_script = g_strdup_printf("<script src=\"file://%skatex/katex.min.js\"></script>", SCRIPTS_DIR);
      katex_auto = g_strdup_printf("<script src=\"file://%skatex/contrib/auto-render.min.js\"></script>", SCRIPTS_DIR);
      break;
  }

 switch (highlight_mode){
    case HIGHLIGHT_OFF:
      highlight_css = g_strdup(" ");
      highlight_script = g_strdup(" ");
      break;
    case HIGHLIGHT_NET:
      highlight_css = g_strdup_printf("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/%s.min.css\">",
                                      local_highlight_css);
      highlight_script = g_strdup("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js\"></script>");
      break;
    case HIGHLIGHT_LOCAL:
      highlight_css = g_strdup_printf("<link rel=\"stylesheet\" href=\"file://%shighlight/styles/%s.css\">", SCRIPTS_DIR, local_highlight_css);
      highlight_script = g_strdup_printf("<script src=\"file://%shighlight/highlight.pack.js\"></script>", SCRIPTS_DIR);
      break;
  }

  switch (mermaid_mode)
  {
    case MERMAID_OFF:
      mermaid_script = g_strdup(" ");
      break;
    case MERMAID_NET:
      mermaid_script = g_strdup("<script src=\"https://unpkg.com/mermaid@7.1.2/dist/mermaid.min.js\"></script>");
      break;
    case MERMAID_LOCAL:
      mermaid_script = g_strdup_printf("<script src=\"file://%smermaid/mermaid.min.js\"></script>", SCRIPTS_DIR);
      break;
  }

  char * buffer = g_strdup_printf("%s\n%s\n%s\n%s\n%s\n%s\n",katex_css, highlight_css, katex_script, katex_auto, highlight_script, mermaid_script);


  g_free(katex_script);
  g_free(katex_auto);
  g_free(katex_css);

  g_free(highlight_css);
  g_free(highlight_script);
  g_free(local_highlight_css);

  g_free(mermaid_script);

  return buffer;
}


char*
html_footer(MarkerKaTeXMode     katex_mode,
            MarkerHighlightMode highlight_mode,
            MarkerMermaidMode   mermaid_mode)
{
  char * katex_render;
  char * highlight_render;
  char * mermaid_render;

  switch(katex_mode){
    case KATEX_OFF:
      katex_render = g_strdup(" ");
      break;
    default:
      katex_render = g_strdup("<script>renderMathInElement(document.body);</script>");
      break;
  }

  switch(highlight_mode)
  {
    case HIGHLIGHT_OFF:
      highlight_render = g_strdup(" ");
      break;
    default:
      highlight_render = g_strdup("<script>hljs.initHighlightingOnLoad();</script>");
      break;
  }

  switch(mermaid_mode)
  {
    case MERMAID_OFF:
      mermaid_render = g_strdup(" ");
      break;
    default:
      mermaid_render = g_strdup("<script>mermaid.initialize({startOnLoad:true});</script>");
      break;
  }

  char* buffer = g_strdup_printf("%s\n%s\n%s\n", katex_render, highlight_render, mermaid_render);
  g_free(highlight_render);
  g_free(katex_render);
  g_free(mermaid_render);
  return buffer;
}

scidown_render_flags
get_render_mode(MarkerMermaidMode mermaid_mode)
{
  scidown_render_flags mode = 0;

  if (mermaid_mode != MERMAID_OFF)
  {
    mode |= SCIDOWN_RENDER_MERMAID;
  }

  if (marker_prefs_get_use_charter())
  {
    mode |= SCIDOWN_RENDER_CHARTER;
  }

  return mode;
}

localization get_local()
{
  localization local;
  local.figure = "Figure";
  local.listing = "Listing";
  local.table = "Table";
  return local;
}

char*
marker_markdown_to_html(const char*         markdown,
                        size_t              size,
                        char *              base_folder,
                        MarkerKaTeXMode     katex_mode,
                        MarkerHighlightMode highlight_mode,
                        MarkerMermaidMode   mermaid_mode,
                        const char*         stylesheet_location)
{
  char* html = NULL;

  hoedown_renderer* renderer;
  hoedown_document* document;
  hoedown_buffer* buffer;
  scidown_render_flags html_mode = get_render_mode(mermaid_mode);

  renderer = hoedown_html_renderer_new(html_mode, 0, get_local());

  char * header = html_header(katex_mode, highlight_mode, mermaid_mode);

  char * ref;
  if (stylesheet_location)
  {
    ref = header;
    header = g_strdup_printf("%s<link rel=\"stylesheet\" type=\"text/css\" href=\"file://%s\">\n", header, stylesheet_location);
    free(ref);
  }

  /*COMMON CSS STYLING*/
  ref = header;
  header = g_strdup_printf("%s<link rel=\"stylesheet\" type=\"text/css\" href=\"file://%s/%s\">\n", header, COMMON_DIR, "scidown.css");
  free(ref);

  char * footer = html_footer(katex_mode, highlight_mode, mermaid_mode);

  ext_definition def = {header, footer};


  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK         |
                                  HOEDOWN_EXT_SPAN          |
                                  HOEDOWN_EXT_FLAGS,
                                  &def,
                                  base_folder,
                                  16);

  buffer = hoedown_buffer_new(500);
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);

  g_free(header);
  g_free(footer);

  const char* buf_cstr = hoedown_buffer_cstr(buffer);

  html = strdup(buf_cstr);
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);

  return html;
}

char*
marker_markdown_to_html_with_css_inline(const char*         markdown,
                                        size_t              size,
                                        char *              base_folder,
                                        MarkerKaTeXMode     katex_mode,
                                        MarkerHighlightMode highlight_mode,
                                        MarkerMermaidMode   mermaid_mode,
                                        const char*         stylesheet_location)
{
  char* html = NULL;


  FILE* fp = NULL;
  fp = fopen(stylesheet_location, "r");
  char* inline_css = NULL;
  if (fp)
  {
    inline_css = NULL;

    fseek(fp , 0 , SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    inline_css = (char*) malloc(sizeof(char) * size);
    fread(inline_css, 1, size, fp);

    fclose(fp);
  }

  hoedown_renderer* renderer;
  hoedown_document* document;
  hoedown_buffer* buffer;
  scidown_render_flags html_mode = get_render_mode(mermaid_mode);

  renderer = hoedown_html_renderer_new(html_mode, 0, get_local());

  char * header = html_header(katex_mode, highlight_mode, mermaid_mode);

  /*To be fixed.*/
  char * common_path;
  common_path = g_strdup_printf("%s/%s", COMMON_DIR, "scidown.css");

  fp = fopen(common_path, "r");
  char* common_css = NULL;
  if (fp)
  {
    fseek(fp , 0 , SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    common_css = (char*) malloc(sizeof(char) * size);
    fread(common_css, 1, size, fp);

    fclose(fp);
  }
  free(common_path);


  if(inline_css && common_css) {
    char * old = header;
    header = g_strdup_printf("%s<style>\n%s\n%s\n</style>\n", header, inline_css, common_css);
    free(old);
    free(common_css);
    free(inline_css);
    inline_css = NULL;
    common_css = NULL;
  } else if (inline_css) {
    char * old = header;
    header = g_strdup_printf("%s<style>\n%s\n</style>\n", header, inline_css);
    free(old);
    free(inline_css);
    inline_css = NULL;
    common_css = NULL;
  } else if (common_css) {
    char * old = header;
    header = g_strdup_printf("%s<style>\n%s\n</style>\n", header, common_css);
    free(old);
    free(common_css);
    inline_css = NULL;
    common_css = NULL;
  }


  char * footer = html_footer(katex_mode, highlight_mode, mermaid_mode);

  ext_definition def = {header, footer};
  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK         |
                                  HOEDOWN_EXT_SPAN          |
                                  HOEDOWN_EXT_FLAGS,
                                  &def,
                                  base_folder,
                                  16);

  buffer = hoedown_buffer_new(500);
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);

  g_free(footer);
  g_free(header);

  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  html = strdup(buf_cstr);

  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);


  return html;
}

char*
marker_markdown_to_latex(const char*         markdown,
                         size_t              size,
                         char *              base_folder,
                         MarkerKaTeXMode     katex_mode,
                         MarkerHighlightMode highlight_mode,
                         MarkerMermaidMode   mermaid_mode,
                         const char*         stylesheet_location)
{
  char* latex = NULL;

  hoedown_renderer* renderer;
  hoedown_document* document;
  hoedown_buffer* buffer;
  scidown_render_flags mode = get_render_mode(mermaid_mode);

  renderer = scidown_latex_renderer_new(mode, 0, get_local());

  ext_definition def = {NULL, NULL};


  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK         |
                                  HOEDOWN_EXT_SPAN          |
                                  HOEDOWN_EXT_FLAGS,
                                  &def,
                                  base_folder,
                                  16);

  buffer = hoedown_buffer_new(500);
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);

  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  latex = strdup(buf_cstr);
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);

  return latex;
}


void
marker_markdown_to_html_file(const char*         markdown,
                             size_t              size,
                             char               *base_folder,
                             MarkerKaTeXMode     katex_mode,
                             MarkerHighlightMode highlight_mode,
                             MarkerMermaidMode   mermaid_mode,
                             const char*         stylesheet_location,
                             const char*         filepath)
{
  char* html = marker_markdown_to_html(markdown,
                                       size,
                                       base_folder,
                                       katex_mode,
                                       highlight_mode,
                                       mermaid_mode,
                                       stylesheet_location);
  FILE* fp = fopen(filepath, "w");
  if (fp && html)
  {
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}

void
marker_markdown_to_html_file_with_css_inline(const char*         markdown,
                                             size_t              size,
                                             char               *base_folder,
                                             MarkerKaTeXMode     katex_mode,
                                             MarkerHighlightMode highlight_mode,
                                             MarkerMermaidMode   mermaid_mode,
                                             const char*         stylesheet_location,
                                             const char*         filepath)
{
  char* html = marker_markdown_to_html_with_css_inline(markdown,
                                                       size,
                                                       base_folder,
                                                       katex_mode,
                                                       highlight_mode,
                                                       mermaid_mode,
                                                       stylesheet_location);
  FILE* fp = fopen(filepath, "w");
  if (fp && html)
  {
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}

void
marker_markdown_to_latex_file(const char*         markdown,
                              size_t              size,
                              char               *base_folder,
                              MarkerKaTeXMode     katex_mode,
                              MarkerHighlightMode highlight_mode,
                              MarkerMermaidMode   mermaid_mode,
                              const char*         filepath)
{


  char* latex = marker_markdown_to_latex(markdown,
                                         size,
                                         base_folder,
                                         katex_mode,
                                         highlight_mode,
                                         mermaid_mode,
                                         NULL);
  FILE* fp = fopen(filepath, "w");
  if (fp && latex)
  {
    fputs(latex, fp);
    fclose(fp);
  }

  free(latex);
}
