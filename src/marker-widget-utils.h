#ifndef __MARKER_WIDGET_UTILS_H__
#define __MARKER_WIDGET_UTILS_H__

void
marker_widget_utils_combo_box_set_string_model(GtkComboBox*  combo_box,
                                               GtkTreeModel* list);

void
marker_widget_utils_populate_combo_box_with_strings(GtkComboBox* combo_box,
                                                    GList*       list);

#endif

