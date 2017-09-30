#ifndef __MARKER_EXPORTER_H__
#define __MARKER_EXPORTER_H__

#include <gtk/gtk.h>

typedef enum
{
  HTML, PDF, RTF, ODT, DOCX, LATEX
} MarkerExportFormat;

MarkerExportFormat
marker_exporter_str_to_fmt(const char* str);

void
marker_exporter_show_export_dialog(GtkWindow*  parent,
                                   const char* markdown,
                                   const char* stylesheet_path);

void
marker_exporter_export(const char*        markdown,
                       const char*        stylesheet,
                       const char*        outfile,
                       MarkerExportFormat format);

#endif

