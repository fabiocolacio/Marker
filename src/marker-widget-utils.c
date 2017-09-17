#include <gtk/gtk.h>

#include "marker-widget-utils.h"

void
marker_widget_utils_combo_box_set_string_model(GtkComboBox*  combo_box,
                                               GtkTreeModel* model)
{
  gtk_combo_box_set_model(combo_box, model);
  GtkCellRenderer* cell_renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box), cell_renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box),
                                 cell_renderer,
                                 "text", 0,
                                 NULL);
  gtk_combo_box_set_active(combo_box, 0);
}

void
marker_widget_utils_populate_combo_box_with_strings(GtkComboBox* combo_box,
                                                    GList*       list)
{
  GtkListStore* list_store = gtk_list_store_new(1, G_TYPE_STRING);
  GtkTreeModel* model = GTK_TREE_MODEL(list_store);
  GtkTreeIter iter;
  for (GList* item = list; item != NULL; item = item->next)
  {
    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter, 0, item->data, -1);
  }
  marker_widget_utils_combo_box_set_string_model(combo_box, model);
}

