#ifndef __MARKER_MARKDOWN_H__
#define __MARKER_MARKDOWN_H__

char*
marker_markdown_to_html(const char*  markdown,
                        size_t       size);
char*
marker_markdown_to_html_with_css(const char*  markdown,
                                 size_t       size,
                                 const char*  href);

void
marker_markdown_to_html_file(const char*  markdown,
                             size_t       size,
                             const char*  filepath);

void
marker_markdown_to_html_file_with_css(const char* markdown,
                                      size_t      size,
                                      const char* filepath,
                                      const char* href);

#endif

