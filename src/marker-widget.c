#include <gtk/gtk.h>

#include <string.h>

#include "marker-string.h"

#include "marker-widget.h"

void
marker_widget_combo_box_set_string_model(GtkComboBox*  combo_box,
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
marker_widget_populate_combo_box_with_strings(GtkComboBox* combo_box,
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
  marker_widget_combo_box_set_string_model(combo_box, model);
}

char*
marker_widget_combo_box_get_active_str(GtkComboBox* combo_box)
{
  GtkTreeIter iter;
  if (gtk_combo_box_get_active_iter(combo_box, &iter))
  {
    GtkTreeModel* model = NULL;
    model = gtk_combo_box_get_model(combo_box);
    GValue value = G_VALUE_INIT;
    if (model)
    {
      gtk_tree_model_get_value(model, &iter, 0, &value);
      const char* str = g_value_get_string(&value);
      return marker_string_alloc(str);
    }
  }
  return NULL;
}

void
marker_widget_combo_box_set_active_str(GtkComboBox* combo_box,
                                       const char*  str,
                                       int          model_len)
{
  if (str)
  {
    gboolean found = FALSE;
    char* active_str = NULL;
    for (int i = 0; i < model_len; ++i)
    {
      gtk_combo_box_set_active(combo_box, i);
      active_str = marker_widget_combo_box_get_active_str(combo_box);
      if (strcmp(active_str, str) == 0 || active_str == NULL)
      {
        found = TRUE;
        break;
      }
    }
    if (!found)
    {
      printf("not found: %s\n", str);
      gtk_combo_box_set_active(combo_box, 0);
    }
  }
}

