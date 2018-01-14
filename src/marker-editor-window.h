/*
 * marker-editor-window.h
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
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

#ifndef __MARKER_EDITOR_WINDOW_H__
#define __MARKER_EDITOR_WINDOW_H__

#include <gtk/gtk.h>

#include "marker-editor.h"
#include "marker-preview.h"

G_BEGIN_DECLS

#define MARKER_TYPE_EDITOR_WINDOW (marker_editor_window_get_type ())

G_DECLARE_FINAL_TYPE (MarkerEditorWindow, marker_editor_window, MARKER, EDITOR_WINDOW, GtkApplicationWindow)

MarkerEditorWindow  *marker_editor_window_new                    (GtkApplication     *app);
MarkerEditorWindow  *marker_editor_window_new_from_file          (GtkApplication     *app,
                                                                  GFile              *file);
void                 marker_editor_window_open_file              (MarkerEditorWindow *window,
                                                                  GFile              *file);
void                 marker_editor_window_save_file              (MarkerEditorWindow *window,
                                                                  GFile              *file);
gchar               *marker_editor_window_get_markdown           (MarkerEditorWindow *window);
void                 marker_editor_window_refresh_preview        (MarkerEditorWindow *window);
void                 marker_editor_window_try_close              (MarkerEditorWindow *window);
void                 marker_editor_window_set_syntax_theme       (MarkerEditorWindow *window,
                                                                  const char         *theme);
void                 marker_editor_window_set_use_syntax_theme   (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_show_line_numbers  (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_spell_check        (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_spell_lang         (MarkerEditorWindow *window,
                                                                  gchar              *lang);
void                 marker_editor_window_set_replace_tabs       (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_auto_indent        (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_tab_width          (MarkerEditorWindow *window,
                                                                  guint               value);
void                 marker_editor_window_set_highlight_current_line
                                                                 (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_wrap_text          (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_show_right_margin  (MarkerEditorWindow *window,
                                                                  gboolean            state);
void                 marker_editor_window_set_right_margin_position
                                                                 (MarkerEditorWindow *window,
                                                                  guint               value);
void                 marker_editor_window_apply_prefs            (MarkerEditorWindow *window);
void                 marker_editor_window_set_title_filename     (MarkerEditorWindow *window);
void                 marker_editor_window_set_title_filename_unsaved
                                                                 (MarkerEditorWindow *window);
void                 marker_editor_window_set_view_mode          (MarkerEditorWindow *window,
                                                                  MarkerViewMode      mode);
MarkerPreview       *marker_editor_window_get_preview            (MarkerEditorWindow *window);
void                 marker_editor_window_fullscreen             (MarkerEditorWindow *window);
void                 marker_editor_window_unfullscreen           (MarkerEditorWindow *window);
void                 marker_editor_window_set_fullscreen         (MarkerEditorWindow *window,
                                                                  gboolean            state);
gboolean             marker_editor_window_get_is_fullscreen      (MarkerEditorWindow *window);
void                 marker_editor_window_toggle_fullscreen      (MarkerEditorWindow *window);

G_END_DECLS

#endif
