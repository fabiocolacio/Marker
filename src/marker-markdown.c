#include <string.h>

#include "hoedown/html.h"
#include "hoedown/document.h"
#include "hoedown/buffer.h"

#include "marker-markdown.h"

char*
marker_markdown_render(char*  markdown,
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
  
  hoedown_document_render(document, buffer, markdown, size);
  const char* buf_cstr = hoedown_buffer_cstr(buffer);
  size_t buf_cstr_len = strlen(buf_cstr) + 1;
  
  char* html = malloc(buf_cstr_len);
  memcpy(html, buf_cstr, buf_cstr_len);
  
  hoedown_html_renderer_free(renderer);
  hoedown_document_free(document);
  hoedown_buffer_free(buffer);
  
  return html;
}

char*
marker_markdown_render_with_css(char*  markdown,
                                size_t size,
                                char*  href)
{
  return NULL;
}

