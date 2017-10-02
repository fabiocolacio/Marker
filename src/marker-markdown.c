#include <string.h>
#include <stdio.h>

#include "hoedown/html.h"
#include "hoedown/document.h"
#include "hoedown/buffer.h"

#include "marker-markdown.h"

char*
marker_markdown_to_html(const char*  markdown,
                        size_t size)
{
  hoedown_renderer* renderer;
  hoedown_document* document;
  hoedown_buffer* buffer;
  
  renderer = hoedown_html_renderer_new(0,0);
  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK |
                                  HOEDOWN_EXT_SPAN  |
                                  HOEDOWN_EXT_FLAGS,
                                  16);
  buffer = hoedown_buffer_new(500);
  
  hoedown_buffer_puts(buffer,
                      "<html>\n"
                      "<head>\n"
                      "</head>\n"
                      "<body>");
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);
  hoedown_buffer_puts(buffer,
                      "</body>\n"
                      "</html>");
  
  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  size_t buf_cstr_len = strlen(buf_cstr) + 1;
  
  char* html = NULL;
  html = malloc(buf_cstr_len);
  memcpy(html, buf_cstr, buf_cstr_len);
  
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);
  
  return html;
}

char*
marker_markdown_to_html_with_css(const char*  markdown,
                                 size_t size,
                                 const char*  href)
{
  hoedown_renderer* renderer;
  hoedown_document* document;
  hoedown_buffer* buffer;
  
  renderer = hoedown_html_renderer_new(0,0);
  document = hoedown_document_new(renderer,
                                  HOEDOWN_EXT_BLOCK |
                                  HOEDOWN_EXT_SPAN  |
                                  HOEDOWN_EXT_FLAGS,
                                  16);
  buffer = hoedown_buffer_new(500);
  
  hoedown_buffer_puts(buffer,
                      "<html>\n"
                      "<head>\n");
  
  if(href)
  {
    hoedown_buffer_printf(buffer,
                         "<link rel=\"stylesheet\" href=\"%s\" type=\"text/css\"/>\n",
                         href);
  }
  hoedown_buffer_puts(buffer,
                      "</head>\n"
                      "<body>\n");
  hoedown_document_render(document, buffer, (uint8_t*) markdown, size);
  hoedown_buffer_puts(buffer,
                      "</body>\n"
                      "</html>");
  
  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  size_t buf_cstr_len = strlen(buf_cstr) + 1;
  
  char* html = NULL;
  html = malloc(buf_cstr_len);
  memcpy(html, buf_cstr, buf_cstr_len);
  
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);
  
  return html;
}

char*
marker_markdown_to_html_with_css_inline(const char* markdown,
                                        size_t      size,
                                        const char* href)
{
  char* html = NULL;
  FILE* fp = NULL;
  fp = fopen(href, "r");
  if (fp)
  {
    char* inline_css = NULL;
    
    fseek(fp , 0 , SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    inline_css = (char*) malloc(sizeof(char) * size);
    fread(inline_css, 1, size, fp);
    
    fclose(fp);
    
    hoedown_renderer* renderer;
    hoedown_document* document;
    hoedown_buffer* buffer;
    
    renderer = hoedown_html_renderer_new(0,0);
    document = hoedown_document_new(renderer,
                                    HOEDOWN_EXT_BLOCK |
                                    HOEDOWN_EXT_SPAN  |
                                    HOEDOWN_EXT_FLAGS,
                                    16);
    buffer = hoedown_buffer_new(500);
    
    hoedown_buffer_puts(buffer,
                        "<html>\n"
                        "<head>\n");
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
    size_t buf_cstr_len = strlen(buf_cstr) + 1;
    
    html = NULL;
    html = malloc(buf_cstr_len);
    memcpy(html, buf_cstr, buf_cstr_len);
    
    hoedown_html_renderer_free(renderer);
    hoedown_document_free(document);
    hoedown_buffer_free(buffer);
  }
  
  return html;
}

void
marker_markdown_to_html_file(const char*  markdown,
                             size_t size,
                             const char*  filepath)
{
  char* html = marker_markdown_to_html(markdown, size);
  FILE* fp = fopen(filepath, "w");
  if (fp && html)
  {
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}
                               
void
marker_markdown_to_html_file_with_css(const char*  markdown,
                                      size_t size,
                                      const char* filepath,
                                      const char* css_filepath)
{
  char* html = marker_markdown_to_html_with_css(markdown, size, css_filepath);
  FILE* fp = fopen(filepath, "w");
  if (fp && html)
  {
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}

void
marker_markdown_to_html_file_with_css_inline(const char* markdown,
                                             size_t      size,
                                             const char* filepath,
                                             const char* href)
{
  char* html = marker_markdown_to_html_with_css_inline(markdown, size, href);
  FILE* fp = fopen(filepath, "w");
  if (fp && html)
  {
    fputs(html, fp);
    fclose(fp);
  }
  free(html);
}

