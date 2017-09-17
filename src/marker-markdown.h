#ifndef __MARKER_MARKDOWN_H__
#define __MARKER_MARKDOWN_H__

#define TMP_MD   ".marker_tmp.md"
#define TMP_HTML ".marker_tmp.html"

typedef enum
{
    HTML,
    PDF,
    RTF,
    EPUB,
    ODT,
    DOCX,
    LATEX
} MarkerFileFormat;

typedef struct
{
    MarkerFileFormat file_type;
    char* style_sheet;
} MarkerExportSettings;

char*
marker_markdown_to_html(char*  markdown,
                        size_t size);
char*
marker_markdown_to_html_with_css(char*  markdown,
                                 size_t size,
                                 char*  href);

void
marker_markdown_to_html_file(char*  markdown,
                             size_t size,
                             char*  filepath);
                               
void
marker_markdown_to_html_file_with_css(char*  markdown,
                                      size_t size,
                                      char* filepath,
                                      char* css_filepath);

#endif

