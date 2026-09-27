/*
 * marker-window.c
 *
 * Copyright (C) 2017-2020 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Marker; see the file LICENSE.md. If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include "marker.h"
#include "marker-prefs.h"
#include "marker-editor.h"
#include "marker-exporter.h"
#include "marker-sketcher-window.h"

#include "marker-window.h"

#include <glib.h>
#include <glib/gi18n.h>

struct _MarkerWindow
{
  AdwApplicationWindow  parent_instance;

  /* template children */
  AdwToolbarView       *toolbar_view;
  AdwHeaderBar         *header_bar;
  AdwWindowTitle       *window_title;
  GtkMenuButton        *menu_btn;
  GtkButton            *unfullscreen_btn;
  GtkBox               *zoom_box;
  GtkButton            *zoom_original_btn;
  AdwOverlaySplitView  *split_view;
  GtkListView          *documents_list;
  GtkStack             *editors_stack;

  /* state */
  GListStore           *documents;
  GtkSingleSelection   *selection;
  MarkerEditor         *active_editor;
  guint                 editors_counter;
  guint                 untitled_files;
  gboolean              is_fullscreen;
  gboolean              sidebar_visible;
  guint                 sidebar_tick_id;
};

G_DEFINE_FINAL_TYPE (MarkerWindow, marker_window, ADW_TYPE_APPLICATION_WINDOW)

typedef void (*MarkerDiscardFunc) (MarkerWindow *window,
                                   gpointer      data);


/* ------------------------------------------------------------------------- */
/* Helpers                                                                    */
/* ------------------------------------------------------------------------- */

static void
update_window_title (MarkerWindow *window)
{
  MarkerEditor *editor = window->active_editor;

  if (!editor)
  {
    adw_window_title_set_title (window->window_title, "Marker");
    adw_window_title_set_subtitle (window->window_title, "");
    gtk_window_set_title (GTK_WINDOW (window), "Marker");
    return;
  }

  g_autofree gchar *title = marker_editor_get_title (editor);
  g_autofree gchar *subtitle = marker_editor_get_subtitle (editor);

  adw_window_title_set_title (window->window_title, title);
  adw_window_title_set_subtitle (window->window_title, subtitle ? subtitle : "");
  gtk_window_set_title (GTK_WINDOW (window), title);
}

static void
update_zoom_label (MarkerWindow *window)
{
  if (!window->active_editor)
    return;

  MarkerPreview *preview = marker_editor_get_preview (window->active_editor);
  const gdouble zoom_percentage = 100 * webkit_web_view_get_zoom_level (WEBKIT_WEB_VIEW (preview));
  g_autofree gchar *zoom_level_str = g_strdup_printf ("%.0f%%", zoom_percentage);
  gtk_button_set_label (window->zoom_original_btn, zoom_level_str);
}

static gboolean
find_editor (MarkerWindow *window,
             MarkerEditor *editor,
             guint        *position)
{
  return g_list_store_find (window->documents, editor, position);
}

static void
save_window_geometry (MarkerWindow *window)
{
  gint width = gtk_widget_get_width (GTK_WIDGET (window));
  gint height = gtk_widget_get_height (GTK_WIDGET (window));

  if (width <= 0 || height <= 0)
    gtk_window_get_default_size (GTK_WINDOW (window), &width, &height);

  if (width > 0 && height > 0 && !window->is_fullscreen && !gtk_window_is_maximized (GTK_WINDOW (window)))
  {
    marker_prefs_set_window_width (width);
    marker_prefs_set_window_height (height);
  }

  if (window->active_editor)
  {
    guint editor_width = marker_editor_get_pane_width (window->active_editor);
    marker_prefs_set_editor_pane_width (editor_width);
  }
}

/* ------------------------------------------------------------------------- */
/* Unsaved-changes confirmation (async)                                       */
/* ------------------------------------------------------------------------- */

typedef struct
{
  MarkerWindow      *window;
  MarkerDiscardFunc  on_discard;
  gpointer           data;
} DiscardRequest;

static void
discard_response_cb (AdwAlertDialog *dialog,
                     const gchar    *response,
                     gpointer        user_data)
{
  DiscardRequest *req = user_data;

  if (g_strcmp0 (response, "discard") == 0 && req->on_discard)
    req->on_discard (req->window, req->data);

  g_free (req);
}

/**
 * confirm_discard:
 *
 * Asks the user whether unsaved changes of @editor may be discarded. If they
 * agree, @on_discard is invoked with @data. Nothing happens on cancel.
 */
static void
confirm_discard (MarkerWindow      *window,
                 MarkerEditor      *editor,
                 MarkerDiscardFunc  on_discard,
                 gpointer           data)
{
  g_assert (MARKER_IS_WINDOW (window));

  AdwDialog *dialog;
  GFile *file = editor ? marker_editor_get_file (editor) : NULL;

  if (G_IS_FILE (file))
  {
    g_autofree gchar *filename = g_file_get_basename (file);
    dialog = adw_alert_dialog_new (NULL, NULL);
    adw_alert_dialog_format_heading (ADW_ALERT_DIALOG (dialog),
                                     _("Discard changes to “%s”?"), filename);
  }
  else
  {
    dialog = adw_alert_dialog_new (_("Discard changes to the document?"), NULL);
  }

  adw_alert_dialog_set_body (ADW_ALERT_DIALOG (dialog),
                             _("The document has unsaved changes that will be lost if it is closed now."));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel", _("_Cancel"),
                                  "discard", _("_Discard"),
                                  NULL);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog), "discard", ADW_RESPONSE_DESTRUCTIVE);
  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  DiscardRequest *req = g_new0 (DiscardRequest, 1);
  req->window = window;
  req->on_discard = on_discard;
  req->data = data;

  g_signal_connect (dialog, "response", G_CALLBACK (discard_response_cb), req);
  adw_dialog_present (dialog, GTK_WIDGET (window));
}

/* ------------------------------------------------------------------------- */
/* Editor bookkeeping                                                         */
/* ------------------------------------------------------------------------- */

static void
destroy_window_now (MarkerWindow *window,
                    gpointer      data)
{
  guint n = g_list_model_get_n_items (G_LIST_MODEL (window->documents));
  for (guint i = 0; i < n; ++i)
  {
    g_autoptr (MarkerEditor) editor = g_list_model_get_item (G_LIST_MODEL (window->documents), i);
    marker_editor_closing (editor);
  }
  gtk_window_destroy (GTK_WINDOW (window));
}

static void
close_editor_now (MarkerWindow *window,
                  gpointer      data)
{
  MarkerEditor *editor = MARKER_EDITOR (data);
  guint position;

  if (!find_editor (window, editor, &position))
    return;

  marker_editor_closing (editor);

  g_object_ref (editor);
  g_list_store_remove (window->documents, position);
  gtk_stack_remove (window->editors_stack, GTK_WIDGET (editor));
  if (window->active_editor == editor)
    window->active_editor = NULL;
  g_object_unref (editor);

  window->editors_counter--;

  if (window->editors_counter < 1)
  {
    marker_window_try_close (window);
    return;
  }

  if (window->editors_counter == 1)
    marker_window_hide_sidebar (window);

  /* Make sure something is selected */
  if (gtk_single_selection_get_selected_item (window->selection) == NULL)
  {
    guint rows = g_list_model_get_n_items (G_LIST_MODEL (window->documents));
    if (rows > 0)
      gtk_single_selection_set_selected (window->selection, rows - 1);
  }
}

void
marker_window_close_editor (MarkerWindow *window,
                            MarkerEditor *editor)
{
  g_assert (MARKER_IS_WINDOW (window));
  g_return_if_fail (MARKER_IS_EDITOR (editor));

  if (marker_editor_has_unsaved_changes (editor))
    confirm_discard (window, editor, close_editor_now, editor);
  else
    close_editor_now (window, editor);
}

/* ------------------------------------------------------------------------- */
/* Actions                                                                    */
/* ------------------------------------------------------------------------- */

static void
action_fullscreen (GSimpleAction *action,
                   GVariant      *value,
                   gpointer       window)
{
  gboolean state = g_variant_get_boolean (value);

  g_simple_action_set_state (action, value);

  if (state)
    marker_window_fullscreen (MARKER_WINDOW (window));
  else
    marker_window_unfullscreen (MARKER_WINDOW (window));
}

static void
action_sidebar (GSimpleAction *action,
                GVariant      *value,
                gpointer       window)
{
  gboolean state = g_variant_get_boolean (value);

  g_simple_action_set_state (action, value);
  marker_prefs_set_show_sidebar (state);

  if (state)
    marker_window_show_sidebar (MARKER_WINDOW (window));
  else
    marker_window_hide_sidebar (MARKER_WINDOW (window));
}

static void
action_link (GSimpleAction *action,
             GVariant      *parameter,
             gpointer       window)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
  if (!editor) return;
  marker_source_view_insert_link (marker_editor_get_source_view (editor));
}

static void
action_monospace (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       window)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
  if (!editor) return;
  marker_source_view_surround_selection_with (marker_editor_get_source_view (editor), "``");
}

static void
action_italic (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       window)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
  if (!editor) return;
  marker_source_view_surround_selection_with (marker_editor_get_source_view (editor), "*");
}

static void
action_bold (GSimpleAction *action,
             GVariant      *parameter,
             gpointer       window)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
  if (!editor) return;
  marker_source_view_surround_selection_with (marker_editor_get_source_view (editor), "**");
}

static void
action_refresh (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       window)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (window));
  if (!editor) return;
  marker_editor_refresh_preview (editor);
}

static void
action_zoom_out (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (user_data));
  if (!editor) return;
  marker_preview_zoom_out (marker_editor_get_preview (editor));
}

static void
action_zoom_original (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (user_data));
  if (!editor) return;
  marker_preview_zoom_original (marker_editor_get_preview (editor));
}

static void
action_zoom_in (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  MarkerEditor *editor = marker_window_get_active_editor (MARKER_WINDOW (user_data));
  if (!editor) return;
  marker_preview_zoom_in (marker_editor_get_preview (editor));
}

static void
action_print (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor) return;
  marker_preview_run_print_dialog (marker_editor_get_preview (editor), GTK_WINDOW (window));
}

static void
reload_now (MarkerWindow *window,
            gpointer      data)
{
  marker_editor_reload_file (MARKER_EDITOR (data));
}

static void
action_reload (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  MarkerWindow *window = user_data;
  MarkerEditor *editor = marker_window_get_active_editor (window);

  g_return_if_fail (MARKER_IS_EDITOR (editor));

  if (marker_editor_has_unsaved_changes (editor))
    confirm_discard (window, editor, reload_now, editor);
  else
    reload_now (window, editor);
}

static void
set_view_mode (MarkerWindow   *window,
               MarkerViewMode  mode)
{
  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor) return;
  marker_editor_set_view_mode (editor, mode);
}

static void
action_editor_only_mode (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  set_view_mode (MARKER_WINDOW (user_data), EDITOR_ONLY_MODE);
}

static void
action_preview_only_mode (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  set_view_mode (MARKER_WINDOW (user_data), PREVIEW_ONLY_MODE);
}

static void
action_dual_pane_mode (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  set_view_mode (MARKER_WINDOW (user_data), DUAL_PANE_MODE);
}

static void
action_dual_window_mode (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  set_view_mode (MARKER_WINDOW (user_data), DUAL_WINDOW_MODE);
}

static void
action_export (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  marker_exporter_show_export_dialog (MARKER_WINDOW (user_data));
}

typedef struct
{
  const gchar   *name;
  GCallback      callback;
  const gchar   *accels[3];
  gboolean       swapped;
} WindowAction;

static void
setup_actions (MarkerWindow *window)
{
  GtkApplication *app = marker_get_app ();

  const WindowAction actions[] = {
    { "refresh",         G_CALLBACK (action_refresh),                    { "<Ctrl>r", NULL },          FALSE },
    { "sketcher",        G_CALLBACK (marker_window_open_sketcher),       { "<Ctrl>d", NULL },          TRUE  },
    { "find",            G_CALLBACK (marker_window_search),              { "<Ctrl>f", NULL },          TRUE  },
    { "link",            G_CALLBACK (action_link),                       { "<Ctrl>k", NULL },          FALSE },
    { "italic",          G_CALLBACK (action_italic),                     { "<Ctrl>i", NULL },          FALSE },
    { "bold",            G_CALLBACK (action_bold),                       { "<Ctrl>b", NULL },          FALSE },
    { "monospace",       G_CALLBACK (action_monospace),                  { "<Ctrl>m", NULL },          FALSE },
    { "new",             G_CALLBACK (marker_create_new_window),          { "<Shift><Ctrl>n", NULL },   TRUE  },
    { "neweditor",       G_CALLBACK (marker_window_new_editor),          { "<Ctrl>n", NULL },          TRUE  },
    { "closedocument",   G_CALLBACK (marker_window_close_current_document), { "<Ctrl>w", NULL },       TRUE  },
    { "close",           G_CALLBACK (marker_window_try_close),           { "<Shift><Ctrl>w", NULL },   TRUE  },
    { "zoomoriginal",    G_CALLBACK (action_zoom_original),              { "<Ctrl>equal", NULL },      FALSE },
    { "zoomin",          G_CALLBACK (action_zoom_in),                    { "<Ctrl>plus", NULL },       FALSE },
    { "zoomout",         G_CALLBACK (action_zoom_out),                   { "<Ctrl>minus", NULL },      FALSE },
    { "editoronlymode",  G_CALLBACK (action_editor_only_mode),           { "<Ctrl>1", NULL },          FALSE },
    { "previewonlymode", G_CALLBACK (action_preview_only_mode),          { "<Ctrl>2", NULL },          FALSE },
    { "dualpanemode",    G_CALLBACK (action_dual_pane_mode),             { "<Ctrl>3", NULL },          FALSE },
    { "dualwindowmode",  G_CALLBACK (action_dual_window_mode),           { "<Ctrl>4", NULL },          FALSE },
    { "open",            G_CALLBACK (marker_window_open_file),           { "<Ctrl>o", NULL },          TRUE  },
    { "openinnewwindow", G_CALLBACK (marker_window_open_file_in_new_window), { "<Ctrl><Shift>o", NULL }, TRUE },
    { "reload",          G_CALLBACK (action_reload),                     { "F5", "<Ctrl>r", NULL },    FALSE },
    { "save",            G_CALLBACK (marker_window_save_active_file),    { "<Ctrl>s", NULL },          TRUE  },
    { "saveas",          G_CALLBACK (marker_window_save_active_file_as), { "<Ctrl><Shift>s", NULL },   TRUE  },
    { "export",          G_CALLBACK (action_export),                     { "<Ctrl>e", NULL },          FALSE },
    { "print",           G_CALLBACK (action_print),                      { "<Ctrl>p", NULL },          FALSE },
  };

  for (gsize i = 0; i < G_N_ELEMENTS (actions); ++i)
  {
    GSimpleAction *action = g_simple_action_new (actions[i].name, NULL);
    if (actions[i].swapped)
      g_signal_connect_swapped (action, "activate", actions[i].callback, window);
    else
      g_signal_connect (action, "activate", actions[i].callback, window);

    g_autofree gchar *detailed = g_strdup_printf ("win.%s", actions[i].name);
    gtk_application_set_accels_for_action (app, detailed, actions[i].accels);
    g_action_map_add_action (G_ACTION_MAP (window), G_ACTION (action));
    g_object_unref (action);
  }

  GSimpleAction *action;

  action = g_simple_action_new_stateful ("fullscreen", NULL, g_variant_new_boolean (FALSE));
  g_signal_connect (action, "change-state", G_CALLBACK (action_fullscreen), window);
  const gchar *fullscreen_accels[] = { "F11", NULL };
  gtk_application_set_accels_for_action (app, "win.fullscreen", fullscreen_accels);
  g_action_map_add_action (G_ACTION_MAP (window), G_ACTION (action));
  g_object_unref (action);

  action = g_simple_action_new_stateful ("sidebar", NULL, g_variant_new_boolean (FALSE));
  g_signal_connect (action, "change-state", G_CALLBACK (action_sidebar), window);
  const gchar *sidebar_accels[] = { "F12", NULL };
  gtk_application_set_accels_for_action (app, "win.sidebar", sidebar_accels);
  g_action_map_add_action (G_ACTION_MAP (window), G_ACTION (action));
  g_object_unref (action);
}

/* ------------------------------------------------------------------------- */
/* Editor signals                                                             */
/* ------------------------------------------------------------------------- */

static void
title_changed_cb (MarkerEditor *editor,
                  const gchar  *title,
                  const gchar  *raw_title,
                  gpointer      user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  if (editor == window->active_editor)
    update_window_title (window);
}

static void
subtitle_changed_cb (MarkerEditor *editor,
                     const gchar  *subtitle,
                     gpointer      user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  if (editor == window->active_editor)
    update_window_title (window);
}

static void
preview_zoom_changed_cb (MarkerPreview *preview,
                         gpointer       user_data)
{
  MarkerWindow *window = user_data;
  if (window->active_editor && marker_editor_get_preview (window->active_editor) == preview)
    update_zoom_label (window);
}

/* ------------------------------------------------------------------------- */
/* Documents sidebar (GtkListView)                                            */
/* ------------------------------------------------------------------------- */

static void
row_close_clicked_cb (GtkButton *button,
                      gpointer   user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  MarkerEditor *editor = g_object_get_data (G_OBJECT (button), "editor");
  if (editor)
    marker_window_close_editor (window, editor);
}

static void
row_label_editing_cb (GtkEditableLabel *label,
                      GParamSpec       *pspec,
                      gpointer          user_data)
{
  if (gtk_editable_label_get_editing (label))
    return;

  MarkerEditor *editor = g_object_get_data (G_OBJECT (label), "editor");
  if (!editor)
    return;

  const gchar *new_text = gtk_editable_get_text (GTK_EDITABLE (label));
  g_autofree gchar *raw_title = marker_editor_get_raw_title (editor);

  if (new_text && *new_text && g_strcmp0 (new_text, raw_title) != 0)
    marker_editor_rename_file (editor, new_text);
  else
    gtk_editable_set_text (GTK_EDITABLE (label), raw_title);
}

static void
row_update (GtkListItem *item)
{
  GtkWidget *box = gtk_list_item_get_child (item);
  MarkerEditor *editor = gtk_list_item_get_item (item);
  GtkEditableLabel *label = g_object_get_data (G_OBJECT (box), "label");
  GtkWidget *dot = g_object_get_data (G_OBJECT (box), "dot");

  if (!editor)
    return;

  g_autofree gchar *raw_title = marker_editor_get_raw_title (editor);
  if (!gtk_editable_label_get_editing (label))
    gtk_editable_set_text (GTK_EDITABLE (label), raw_title);
  gtk_widget_set_visible (dot, marker_editor_has_unsaved_changes (editor));
}

static void
row_title_changed_cb (MarkerEditor *editor,
                      const gchar  *title,
                      const gchar  *raw_title,
                      gpointer      user_data)
{
  row_update (GTK_LIST_ITEM (user_data));
}

static void
row_setup_cb (GtkSignalListItemFactory *factory,
              GtkListItem              *item,
              gpointer                  user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  GtkWidget *label = gtk_editable_label_new ("");
  GtkWidget *dot = gtk_label_new ("●");
  GtkWidget *close = gtk_button_new_from_icon_name ("window-close-symbolic");

  gtk_widget_set_hexpand (label, TRUE);
  gtk_widget_set_halign (label, GTK_ALIGN_FILL);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_add_css_class (dot, "dim-label");
  gtk_widget_set_visible (dot, FALSE);
  gtk_widget_add_css_class (close, "flat");
  gtk_widget_add_css_class (close, "circular");
  gtk_widget_set_valign (close, GTK_ALIGN_CENTER);
  gtk_widget_set_tooltip_text (close, _("Close document"));

  gtk_box_append (GTK_BOX (box), label);
  gtk_box_append (GTK_BOX (box), dot);
  gtk_box_append (GTK_BOX (box), close);

  g_object_set_data (G_OBJECT (box), "label", label);
  g_object_set_data (G_OBJECT (box), "dot", dot);
  g_object_set_data (G_OBJECT (box), "close", close);

  g_signal_connect (close, "clicked", G_CALLBACK (row_close_clicked_cb), window);
  g_signal_connect (label, "notify::editing", G_CALLBACK (row_label_editing_cb), window);

  gtk_list_item_set_child (item, box);
}

static void
row_bind_cb (GtkSignalListItemFactory *factory,
             GtkListItem              *item,
             gpointer                  user_data)
{
  GtkWidget *box = gtk_list_item_get_child (item);
  MarkerEditor *editor = gtk_list_item_get_item (item);
  GtkWidget *label = g_object_get_data (G_OBJECT (box), "label");
  GtkWidget *close = g_object_get_data (G_OBJECT (box), "close");

  g_object_set_data (G_OBJECT (label), "editor", editor);
  g_object_set_data (G_OBJECT (close), "editor", editor);

  gulong id = g_signal_connect (editor, "title-changed", G_CALLBACK (row_title_changed_cb), item);
  g_object_set_data (G_OBJECT (item), "title-handler", GSIZE_TO_POINTER (id));

  row_update (item);
}

static void
row_unbind_cb (GtkSignalListItemFactory *factory,
               GtkListItem              *item,
               gpointer                  user_data)
{
  GtkWidget *box = gtk_list_item_get_child (item);
  MarkerEditor *editor = gtk_list_item_get_item (item);
  GtkWidget *label = g_object_get_data (G_OBJECT (box), "label");
  GtkWidget *close = g_object_get_data (G_OBJECT (box), "close");

  gulong id = GPOINTER_TO_SIZE (g_object_get_data (G_OBJECT (item), "title-handler"));
  if (id && editor)
    g_signal_handler_disconnect (editor, id);
  g_object_set_data (G_OBJECT (item), "title-handler", NULL);

  g_object_set_data (G_OBJECT (label), "editor", NULL);
  g_object_set_data (G_OBJECT (close), "editor", NULL);
}

static void
selection_changed_cb (GtkSingleSelection *selection,
                      GParamSpec         *pspec,
                      gpointer            user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  MarkerEditor *editor = gtk_single_selection_get_selected_item (selection);

  if (!editor)
    return;

  window->active_editor = editor;
  gtk_stack_set_visible_child (window->editors_stack, GTK_WIDGET (editor));
  update_window_title (window);
  update_zoom_label (window);
  gtk_widget_grab_focus (GTK_WIDGET (marker_editor_get_source_view (editor)));
}

/* ------------------------------------------------------------------------- */
/* GObject                                                                    */
/* ------------------------------------------------------------------------- */

static gboolean
close_request_cb (GtkWindow *gtk_window,
                  gpointer   user_data)
{
  marker_window_try_close (MARKER_WINDOW (gtk_window));
  return TRUE; /* we destroy ourselves once the user has been asked */
}

static void
marker_window_init (MarkerWindow *window)
{
  gtk_widget_init_template (GTK_WIDGET (window));

  /* Bundled sketcher tool icons live in the resource bundle */
  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gdk_display_get_default ()),
                                    "/com/github/fabiocolacio/marker/icons");

  window->is_fullscreen = FALSE;
  window->sidebar_visible = FALSE;
  window->sidebar_tick_id = 0;
  window->editors_counter = 0;
  window->untitled_files = 0;
  window->active_editor = NULL;

  setup_actions (window);

  /* Zoom controls inside the gear menu */
  GtkPopoverMenu *popover = GTK_POPOVER_MENU (gtk_menu_button_get_popover (window->menu_btn));
  gtk_popover_menu_add_child (popover, GTK_WIDGET (window->zoom_box), "zoom");

  /* Documents list */
  window->documents = g_list_store_new (MARKER_TYPE_EDITOR);
  window->selection = gtk_single_selection_new (G_LIST_MODEL (g_object_ref (window->documents)));
  gtk_single_selection_set_autoselect (window->selection, TRUE);
  gtk_single_selection_set_can_unselect (window->selection, FALSE);

  GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
  g_signal_connect (factory, "setup", G_CALLBACK (row_setup_cb), window);
  g_signal_connect (factory, "bind", G_CALLBACK (row_bind_cb), window);
  g_signal_connect (factory, "unbind", G_CALLBACK (row_unbind_cb), window);

  gtk_list_view_set_factory (window->documents_list, factory);
  gtk_list_view_set_model (window->documents_list, GTK_SELECTION_MODEL (window->selection));
  g_object_unref (factory);

  g_signal_connect (window->selection, "notify::selected-item", G_CALLBACK (selection_changed_cb), window);

  /* Window geometry */
  guint width = marker_prefs_get_window_width ();
  guint height = marker_prefs_get_window_height ();
  if (width == 0) width = 900;
  if (height == 0) height = 600;
  gtk_window_set_default_size (GTK_WINDOW (window), width, height);

  adw_overlay_split_view_set_show_sidebar (window->split_view, FALSE);
  if (marker_prefs_get_show_sidebar ())
  {
    /* toggles the stateful action, which shows the sidebar */
    g_action_group_activate_action (G_ACTION_GROUP (window), "sidebar", NULL);
  }

  g_signal_connect (window, "close-request", G_CALLBACK (close_request_cb), NULL);

  update_window_title (window);
}

static void
marker_window_dispose (GObject *object)
{
  MarkerWindow *window = MARKER_WINDOW (object);

  if (window->sidebar_tick_id)
  {
    gtk_widget_remove_tick_callback (GTK_WIDGET (window), window->sidebar_tick_id);
    window->sidebar_tick_id = 0;
  }

  gtk_widget_dispose_template (GTK_WIDGET (window), MARKER_TYPE_WINDOW);

  g_clear_object (&window->selection);
  g_clear_object (&window->documents);

  G_OBJECT_CLASS (marker_window_parent_class)->dispose (object);
}

static void
marker_window_class_init (MarkerWindowClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  object_class->dispose = marker_window_dispose;

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/fabiocolacio/marker/ui/marker-window.ui");
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, toolbar_view);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, window_title);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, menu_btn);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, unfullscreen_btn);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, zoom_box);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, zoom_original_btn);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, documents_list);
  gtk_widget_class_bind_template_child (widget_class, MarkerWindow, editors_stack);
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                 */
/* ------------------------------------------------------------------------- */

static void
marker_window_add_editor (MarkerWindow *window,
                          MarkerEditor *editor)
{
  gtk_stack_add_child (window->editors_stack, GTK_WIDGET (editor));
  g_list_store_append (window->documents, editor);

  guint n = g_list_model_get_n_items (G_LIST_MODEL (window->documents));
  gtk_single_selection_set_selected (window->selection, n - 1);

  /* selection_changed_cb may not fire if the index did not change */
  window->active_editor = editor;
  gtk_stack_set_visible_child (window->editors_stack, GTK_WIDGET (editor));
  update_window_title (window);

  if (window->editors_counter >= 1)
    marker_window_show_sidebar (window);

  g_signal_connect (editor, "title-changed", G_CALLBACK (title_changed_cb), window);
  g_signal_connect (editor, "subtitle-changed", G_CALLBACK (subtitle_changed_cb), window);
  g_signal_connect (marker_editor_get_preview (editor), "zoom-changed",
                    G_CALLBACK (preview_zoom_changed_cb), window);
  update_zoom_label (window);

  window->editors_counter++;

  marker_editor_refresh_preview (editor);
  gtk_widget_grab_focus (GTK_WIDGET (marker_editor_get_source_view (editor)));
}

void
marker_window_new_editor (MarkerWindow *window)
{
  MarkerEditor *editor = marker_editor_new ();
  if (window->untitled_files)
  {
    g_autofree gchar *name = g_strdup_printf ("Untitled_%u.md", window->untitled_files);
    marker_editor_rename_file (editor, name);
  }
  window->untitled_files++;
  marker_window_add_editor (window, editor);
}

void
marker_window_new_editor_from_file (MarkerWindow *window,
                                    GFile        *file)
{
  guint n = g_list_model_get_n_items (G_LIST_MODEL (window->documents));

  /* Already open? Then just show it. */
  for (guint i = 0; i < n; ++i)
  {
    g_autoptr (MarkerEditor) editor = g_list_model_get_item (G_LIST_MODEL (window->documents), i);
    GFile *editor_file = marker_editor_get_file (editor);
    if (editor_file != NULL && g_file_equal (editor_file, file))
    {
      gtk_single_selection_set_selected (window->selection, i);
      return;
    }
  }

  /* Replace an empty untitled document instead of keeping it around */
  MarkerEditor *active = marker_window_get_active_editor (window);
  if (active != NULL && marker_editor_get_file (active) == NULL)
  {
    g_autofree gchar *md = marker_source_view_get_text (marker_editor_get_source_view (active), FALSE);
    if (g_strcmp0 (md, "") == 0)
    {
      window->editors_counter++;
      close_editor_now (window, active);
      window->editors_counter--;
    }
  }

  MarkerEditor *editor = marker_editor_new_from_file (file);
  marker_window_add_editor (window, editor);
}

MarkerWindow *
marker_window_new (GtkApplication *app)
{
  MarkerWindow *window = g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
  marker_window_new_editor (window);
  return window;
}

MarkerWindow *
marker_window_new_from_file (GtkApplication *app,
                             GFile          *file)
{
  MarkerWindow *window = g_object_new (MARKER_TYPE_WINDOW, "application", app, NULL);
  marker_window_new_editor_from_file (window, file);
  return window;
}

static GListStore *
markdown_filters (void)
{
  GListStore *filters = g_list_store_new (GTK_TYPE_FILE_FILTER);

  GtkFileFilter *md = gtk_file_filter_new ();
  gtk_file_filter_set_name (md, _("Markdown files"));
  gtk_file_filter_add_mime_type (md, "text/markdown");
  gtk_file_filter_add_mime_type (md, "text/x-markdown");
  gtk_file_filter_add_suffix (md, "md");
  gtk_file_filter_add_suffix (md, "markdown");
  gtk_file_filter_add_suffix (md, "txt");
  g_list_store_append (filters, md);
  g_object_unref (md);

  GtkFileFilter *all = gtk_file_filter_new ();
  gtk_file_filter_set_name (all, _("All files"));
  gtk_file_filter_add_pattern (all, "*");
  g_list_store_append (filters, all);
  g_object_unref (all);

  return filters;
}

static void
open_file_cb (GObject      *source,
              GAsyncResult *result,
              gpointer      user_data)
{
  MarkerWindow *window = MARKER_WINDOW (user_data);
  g_autoptr (GFile) file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (source), result, NULL);
  if (file)
    marker_window_new_editor_from_file (window, file);
  g_object_unref (window);
}

void
marker_window_open_file (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  g_autoptr (GtkFileDialog) dialog = gtk_file_dialog_new ();
  g_autoptr (GListStore) filters = markdown_filters ();
  gtk_file_dialog_set_title (dialog, _("Open"));
  gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));
  gtk_file_dialog_open (dialog, GTK_WINDOW (window), NULL, open_file_cb, g_object_ref (window));
}

static void
open_file_in_new_window_cb (GObject      *source,
                            GAsyncResult *result,
                            gpointer      user_data)
{
  g_autoptr (GFile) file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (source), result, NULL);
  if (file)
    marker_create_new_window_from_file (file);
}

void
marker_window_open_file_in_new_window (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  g_autoptr (GtkFileDialog) dialog = gtk_file_dialog_new ();
  g_autoptr (GListStore) filters = markdown_filters ();
  gtk_file_dialog_set_title (dialog, _("Open in New Window"));
  gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));
  gtk_file_dialog_open (dialog, GTK_WINDOW (window), NULL, open_file_in_new_window_cb, NULL);
}

static void
save_file_as_cb (GObject      *source,
                 GAsyncResult *result,
                 gpointer      user_data)
{
  MarkerEditor *editor = MARKER_EDITOR (user_data);
  g_autoptr (GFile) file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (source), result, NULL);
  if (file)
    marker_editor_save_file_as (editor, file);
  g_object_unref (editor);
}

void
marker_window_save_active_file_as (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
    return;

  g_autoptr (GtkFileDialog) dialog = gtk_file_dialog_new ();
  g_autoptr (GListStore) filters = markdown_filters ();
  gtk_file_dialog_set_title (dialog, _("Save As"));
  gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));

  GFile *current = marker_editor_get_file (editor);
  if (current)
  {
    gtk_file_dialog_set_initial_file (dialog, current);
  }
  else
  {
    g_autofree gchar *name = marker_editor_get_raw_title (editor);
    gtk_file_dialog_set_initial_name (dialog, name);
  }

  gtk_file_dialog_save (dialog, GTK_WINDOW (window), NULL, save_file_as_cb, g_object_ref (editor));
}

void
marker_window_save_active_file (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));
  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
    return;

  if (marker_editor_get_file (editor))
    marker_editor_save_file (editor);
  else
    marker_window_save_active_file_as (window);
}

void
marker_window_fullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  if (window->is_fullscreen)
    return;

  window->is_fullscreen = TRUE;
  gtk_window_fullscreen (GTK_WINDOW (window));
  gtk_widget_set_visible (GTK_WIDGET (window->unfullscreen_btn), TRUE);
}

void
marker_window_unfullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  if (!window->is_fullscreen)
    return;

  window->is_fullscreen = FALSE;
  gtk_window_unfullscreen (GTK_WINDOW (window));
  gtk_widget_set_visible (GTK_WIDGET (window->unfullscreen_btn), FALSE);
}

void
marker_window_toggle_fullscreen (MarkerWindow *window)
{
  g_return_if_fail (MARKER_IS_WINDOW (window));
  g_action_group_change_action_state (G_ACTION_GROUP (window), "fullscreen",
                                      g_variant_new_boolean (!window->is_fullscreen));
}

gboolean
marker_window_is_fullscreen (MarkerWindow *window)
{
  return window->is_fullscreen;
}

MarkerEditor *
marker_window_get_active_editor (MarkerWindow *window)
{
  g_return_val_if_fail (MARKER_IS_WINDOW (window), NULL);
  return window->active_editor;
}

gboolean
marker_window_try_close (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  save_window_geometry (window);

  guint n = g_list_model_get_n_items (G_LIST_MODEL (window->documents));
  for (guint i = 0; i < n; ++i)
  {
    g_autoptr (MarkerEditor) editor = g_list_model_get_item (G_LIST_MODEL (window->documents), i);
    if (marker_editor_has_unsaved_changes (editor))
    {
      gtk_single_selection_set_selected (window->selection, i);
      confirm_discard (window, editor, destroy_window_now, NULL);
      return FALSE;
    }
  }

  destroy_window_now (window, NULL);
  return TRUE;
}

void
marker_window_close_current_document (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
  {
    marker_window_try_close (window);
    return;
  }

  marker_window_close_editor (window, editor);
}

static void
apply_sidebar_state (MarkerWindow *window)
{
  adw_overlay_split_view_set_show_sidebar (window->split_view, window->sidebar_visible);
}

static gboolean
apply_sidebar_when_allocated (GtkWidget     *widget,
                              GdkFrameClock *clock,
                              gpointer       user_data)
{
  MarkerWindow *window = MARKER_WINDOW (widget);

  if (gtk_widget_get_width (GTK_WIDGET (window->split_view)) <= 0)
    return G_SOURCE_CONTINUE;

  window->sidebar_tick_id = 0;
  apply_sidebar_state (window);
  return G_SOURCE_REMOVE;
}

/*
 * AdwOverlaySplitView animates the sidebar reveal with a spring whose initial
 * velocity is divided by the sidebar width. Before the first allocation that
 * width is 0, the velocity becomes NaN and the split view allocates its
 * content with a garbage (G_MININT-based) width, which in turn corrupts the
 * editor's GtkPaned position. So the split view is only touched once it has a
 * real allocation; until then we just remember the wanted state.
 */
static void
set_sidebar_state (MarkerWindow *window,
                   gboolean      visible)
{
  window->sidebar_visible = visible;

  if (gtk_widget_get_width (GTK_WIDGET (window->split_view)) > 0)
    apply_sidebar_state (window);
  else if (window->sidebar_tick_id == 0)
    window->sidebar_tick_id = gtk_widget_add_tick_callback (GTK_WIDGET (window),
                                                            apply_sidebar_when_allocated,
                                                            NULL, NULL);

  GAction *action = g_action_map_lookup_action (G_ACTION_MAP (window), "sidebar");
  if (action)
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (visible));
}

void
marker_window_toggle_sidebar (MarkerWindow *window)
{
  set_sidebar_state (window, !window->sidebar_visible);
}

void
marker_window_hide_sidebar (MarkerWindow *window)
{
  set_sidebar_state (window, FALSE);
}

void
marker_window_show_sidebar (MarkerWindow *window)
{
  set_sidebar_state (window, TRUE);
}

void
marker_window_open_sketcher (MarkerWindow *window)
{
  g_assert (MARKER_IS_WINDOW (window));

  MarkerEditor *editor = marker_window_get_active_editor (window);
  if (!editor)
    return;

  GFile *file = marker_editor_get_file (editor);
  MarkerSourceView *source_view = marker_editor_get_source_view (editor);

  marker_sketcher_window_show (GTK_WINDOW (window), file, source_view);
}

void
marker_window_search (MarkerWindow *window)
{
  if (window->active_editor)
    marker_editor_toggle_search_bar (window->active_editor);
}

void
marker_window_refresh_all_preview (MarkerWindow *window)
{
  guint n = g_list_model_get_n_items (G_LIST_MODEL (window->documents));
  for (guint i = 0; i < n; ++i)
  {
    g_autoptr (MarkerEditor) editor = g_list_model_get_item (G_LIST_MODEL (window->documents), i);
    marker_editor_refresh_preview (editor);
  }
}

