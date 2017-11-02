#ifndef __MARKER_EXPORTER_H__
#define __MARKER_EXPORTER_H__

#include <gtk/gtk.h>

typedef enum
{
  HTML, PDF, RTF, ODT, DOCX, LATEX
} MarkerExportFormat;

void
marker_exporter_show_export_dialog(MarkerEditorWindow* window);

void
marker_exporter_export(const char*        markdown,
                       const char*        stylesheet,
                       const char*        outfile,
                       MarkerExportFormat format);

#endif

