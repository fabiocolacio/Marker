#ifndef __MARKER_MARKDOWN_H__
#define __MARKER_MARKDOWN_H__

char*
marker_markdown_render(char*  markdown,
                       size_t size);
char*
marker_markdown_render_with_css(char*  markdown,
                                size_t size,
                                char*  href);

#endif

