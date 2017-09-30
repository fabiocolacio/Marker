#ifndef __MARKER_WIDGET_H__
#define __MARKER_WIDGET_H__

void
marker_widget_combo_box_set_string_model(GtkComboBox*  combo_box,
                                         GtkTreeModel* list);

void
marker_widget_populate_combo_box_with_strings(GtkComboBox* combo_box,
                                              GList*       list);

char*
marker_widget_combo_box_get_active_str(GtkComboBox* combo_box);

void
marker_widget_combo_box_set_active_str(GtkComboBox* combo_box,
                                       const char*  str,
                                       int          model_len);

#endif

