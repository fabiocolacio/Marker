#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "hoedown/html.h"
#include "hoedown/document.h"
#include "hoedown/buffer.h"

#include "marker-markdown.h"

char*
marker_markdown_to_html(const char*       markdown,
                        size_t            size,
                        MarkerMathJaxMode mathjax_mode,
                        const char*       stylesheet_location)
{
  char* html = NULL;
  char* mathjax_script;

  hoedown_renderer* renderer;
  hoedown_document* document;
  hoedown_buffer* buffer;
  
  renderer = hoedown_html_renderer_new(0,0);
  
  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK         |
                                  HOEDOWN_EXT_SPAN          |
                                  HOEDOWN_EXT_MATH          |
                                  HOEDOWN_EXT_MATH_EXPLICIT |
                                  HOEDOWN_EXT_FLAGS,
                                  16);
                                  
  buffer = hoedown_buffer_new(500); 
  
  switch (mathjax_mode) {
    case MATHJAX_OFF:
      mathjax_script = g_strdup(" ");
      break;
    
    case MATHJAX_NET:
      mathjax_script = g_strdup("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-MML-AM_CHTML\"></script>");
      break;
    
    case MATHJAX_LOCAL:
      mathjax_script = g_strdup_printf("<script src=\"%sMathJax/MathJax.js?config=TeX-MML-AM_CHTML\"></script>", SCRIPTS_DIR);
      break;
  }
  
  hoedown_buffer_printf(buffer,
                        "<!doctype html>"
                        "<html>\n"
                        "<head>\n"
                        "%s\n"
                        "<meta charset=\"utf-8\">\n",
                        mathjax_script);

  g_free(mathjax_script);

  if (stylesheet_location)
  {
    hoedown_buffer_printf(buffer, "<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", stylesheet_location);
  }
  
  hoedown_buffer_puts(buffer,
                      "</head>\n"
                      "<body>\n");
                      
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);
  
  hoedown_buffer_puts(buffer,
                      "</body>\n"
                      "</html>");
  
  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  
  html = strdup(buf_cstr);
  
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);
  
  return html;
}

char*
marker_markdown_to_html_with_css_inline(const char*       markdown,
                                        size_t            size,
                                        MarkerMathJaxMode mathjax_mode,
                                        const char*       stylesheet_location)
{
  char* html = NULL;
  char* mathjax_script;
  
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
  
  renderer = hoedown_html_renderer_new(0,0);
  
  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK         |
                                  HOEDOWN_EXT_SPAN          |
                                  HOEDOWN_EXT_MATH          |
                                  HOEDOWN_EXT_MATH_EXPLICIT |
                                  HOEDOWN_EXT_FLAGS,
                                  16);
 
  buffer = hoedown_buffer_new(500);
  
  switch (mathjax_mode) {
    case MATHJAX_OFF:
      mathjax_script = g_strdup(" ");
      break;
    
    case MATHJAX_NET:
      mathjax_script = g_strdup("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-MML-AM_CHTML\"></script>");
      break;
    
    case MATHJAX_LOCAL:
      mathjax_script = g_strdup_printf("<script src=\"%sMathJax/MathJax.js?config=TeX-MML-AM_CHTML\"></script>", SCRIPTS_DIR);
      break;
  }
  
  hoedown_buffer_printf(buffer,
                        "<!doctype html>\n"
                        "<html>\n"
                        "<head>\n"
                        "%s\n"
                        "<meta charset=\"utf-8\">\n",
                        mathjax_script);

  g_free(mathjax_script);

  if(inline_css)
  {
    hoedown_buffer_printf(buffer,
                         "<style>\n%s\n</style>\n",
                         inline_css);
    free(inline_css);
    inline_css = NULL;
  }
  
  hoedown_buffer_puts(buffer,
                      "</head>\n"
                      "<body>\n");
                      
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);
  
  hoedown_buffer_puts(buffer,
                      "</body>\n"
                      "</html>");
  
  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  html = strdup(buf_cstr);
  
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);

  
  return html;
}

void
marker_markdown_to_html_file(const char*       markdown,
                             size_t            size,
                             MarkerMathJaxMode mathjax_mode,
                             const char*       stylesheet_location,
                             const char*       filepath)
{
  char* html = marker_markdown_to_html(markdown, size, mathjax_mode, stylesheet_location);
  FILE* fp = fopen(filepath, "w");
  if (fp && html)
  {
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}

void
marker_markdown_to_html_file_with_css_inline(const char*       markdown,
                                             size_t            size,
                                             MarkerMathJaxMode mathjax_mode,
                                             const char*       stylesheet_location,
                                             const char*       filepath)
{
  char* html = marker_markdown_to_html_with_css_inline(markdown, size, mathjax_mode, stylesheet_location);
  FILE* fp = fopen(filepath, "w");
  printf("fp: %p\nfilepath: %s\n", fp, filepath);
  if (fp && html)
  {
    puts("two");
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}
