#ifndef __MARKER_MARKDOWN_H__
#define __MARKER_MARKDOWN_H__

#include <stddef.h>

/**
 * An enum that controls mathjax settings for generated HTML documents
 */
typedef enum {
  MATHJAX_OFF,  /**< Disable MathJax */
  MATHJAX_NET,  /**< Include MathJax hosted on the CloudFlare CDN */
  MATHJAX_LOCAL /**< Include the local MathJax installation */
} MarkerMathJaxMode;

/**
 * Generates HTML output from markdown input.
 *
 * Optionally pass the filepath to a css stylesheet to link from
 * the HTML document or NULL.
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param mathjax_mode The mathjax settings to use for this document
 * @param stylesheet_location The stylesheet to link to in the generated HTML or NULL
 *
 * @return An allocated buffer containing null-terminated UTF-8 encoded HTML
 */
char*
marker_markdown_to_html(const char*       markdown,
                        size_t            size,
                        MarkerMathJaxMode mathjax_mode,
                        const char*       stylesheet_location);

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
 * @param mathjax_mode The mathjax settings to use for this document
 * @param stylesheet_location The stylesheet to include in the generated HTML or NULL
 *
 * @return An allocated buffer containing null-terminating UTF-8 encoding HTML
 */                
char*
marker_markdown_to_html_with_css_inline(const char*       markdown,
                                        size_t            size,
                                        MarkerMathJaxMode mathjax_mode,
                                        const char*       stylesheet_location);

/**
 * Creates an HTML file from markdown input.
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param mathjax_mode The mathjax settings to use for this document
 * @param stylesheet_location The stylesheet to include in the generated HTML or NULL
 * @param filepath The HTML file to create
 */
void
marker_markdown_to_html_file(const char*       markdown,
                             size_t            size,
                             MarkerMathJaxMode mathjax_mode,
                             const char*       stylesheet_location,
                             const char*       filepath);

/**
 * Creates an HTML file from markdown input.
 *
 * @see marker_markdown_to_html()
 *
 * @param markdown A buffer which contains UTF-8 encoded Markdown
 * @param size The size of the markdown buffer
 * @param mathjax_mode The mathjax settings to use for this document
 * @param stylesheet_location The stylesheet to include in the generated HTML or NULL
 * @param filepath The HTML file to create
 */
void
marker_markdown_to_html_file_with_css_inline(const char*       markdown,
                                             size_t            size,
                                             MarkerMathJaxMode mathjax_mode,
                                             const char*       stylesheet_location,
                                             const char*       filepath);

#endif
