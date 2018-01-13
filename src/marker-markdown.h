/*
 * marker-markdown.h
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MARKER_MARKDOWN_H__
#define __MARKER_MARKDOWN_H__

#include <stddef.h>

/**
 * An enum that controls mathjax settings for generated HTML documents
 */
typedef enum {
  KATEX_OFF,  /**< Disable KaTeX */
  KATEX_NET,  /**< Include net KaTeX version */
  KATEX_LOCAL /**< Include the local KaTeX installation */
} MarkerKaTeXMode;

typedef enum{
  HIGHLIGHT_OFF,  /**< Disable highlight.js */
  HIGHLIGHT_NET,  /**< Include net highlight.js version */
  HIGHLIGHT_LOCAL /**< Include the local highlight.js installation */
} MarkerHighlightMode;

typedef enum{
  MERMAID_OFF,
  MERMAID_NET,
  MERMAID_LOCAL
} MarkerMermaidMode;

/**
 * Generates HTML output from markdown input.
 *
 * Optionally pass the filepath to a css stylesheet to link from
 * the HTML document or NULL.
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param katex_mode The KaTeX settings to use for this document
 * @param highligh_mode The highlight.js settings to use for this document
 * @param stylesheet_location The stylesheet to link to in the generated HTML or NULL
 *
 * @return An allocated buffer containing null-terminated UTF-8 encoded HTML
 */
char*
marker_markdown_to_html(const char*         markdown,
                        size_t              size,
                        MarkerKaTeXMode     katex_mode,
                        MarkerHighlightMode highlight_mode, 
                        MarkerMermaidMode   mermaid_mode,
                        const char*         stylesheet_location);

/**
 * Generates HTML output from markdown input.
 *
 * Unlike marker_markdown_to_html(), the stylesheet is copied from
 * stylesheet_location, into the html file as inline css.
 *
 * @see marker_markdown_to_html()
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param katex_mode The KaTeX settings to use for this document
 * @param highligh_mode The highlight.js settings to use for this document
 * @param stylesheet_location The stylesheet to include in the generated HTML or NULL
 *
 * @return An allocated buffer containing null-terminating UTF-8 encoding HTML
 */                
char*
marker_markdown_to_html_with_css_inline(const char*         markdown,
                                        size_t              size,
                                        MarkerKaTeXMode     katex_mode,
                                        MarkerHighlightMode highlight_mode, 
                                        MarkerMermaidMode   mermaid_mode,
                                        const char*         stylesheet_location);

/**
 * Creates an HTML file from markdown input.
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param katex_mode The KaTeX settings to use for this document
 * @param highligh_mode The highlight.js settings to use for this document
 * @param stylesheet_location The stylesheet to include in the generated HTML or NULL
 * @param filepath The HTML file to create
 */
void
marker_markdown_to_html_file(const char*         markdown,
                             size_t              size,
                             MarkerKaTeXMode     katex_mode,
                             MarkerHighlightMode highlight_mode,
                             MarkerMermaidMode   mermaid_mode,
                             const char*         stylesheet_location,
                             const char*         filepath);

/**
 * Creates an HTML file from markdown input.
 *
 * @see marker_markdown_to_html()
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param katex_mode The KaTeX settings to use for this document
 * @param highligh_mode The highlight.js settings to use for this document
 * @param stylesheet_location The stylesheet to include in the generated HTML or NULL
 * @param filepath The HTML file to create
 */
void
marker_markdown_to_html_file_with_css_inline(const char*         markdown,
                                             size_t              size,
                                             MarkerKaTeXMode     katex_mode,
                                             MarkerHighlightMode highlight_mode, 
                                             MarkerMermaidMode   mermaid_mode,
                                             const char*         stylesheet_location,
                                             const char*         filepath);

#endif
