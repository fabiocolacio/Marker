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
marker_markdown_render(char*  markdown,
                       size_t size);
char*
marker_markdown_render_with_css(char*  markdown,
                                size_t size,
                                char*  href);

void
marker_markdown_render_to_file(char*  markdown,
                               size_t size,
                               char*  filepath);
                               
void
marker_markdown_render_to_file_with_css(char*  markdown,
                                        size_t size,
                                        char* filepath,
                                        char* css_filepath);

#ifdef PANDOC
void
marker_markdown_pandoc_export(char*                markdown,
                              MarkerExportSettings settings,
                              char*                format,
                              char*                out_file);
#endif

#endif

