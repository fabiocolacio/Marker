/*
 * marker-prefs.h
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

#ifndef __MARKER_PREFS_H__
#define __MARKER_PREFS_H__

#include "marker-editor.h"

typedef struct {
  GSettings *editor_settings;
  GSettings *preview_settings;
  GSettings *window_settings;
} MarkerPrefs;

gboolean             marker_prefs_get_use_dark_theme             (void);
void                 marker_prefs_set_use_dark_theme             (gboolean            state);
guint                marker_prefs_get_window_width               (void);
void                 marker_prefs_set_window_width               (guint               width);
guint                marker_prefs_get_window_height              (void);
void                 marker_prefs_set_window_height              (guint               height);
void                 marker_prefs_get_window_position            (gint               *pos_x,
                                                                  gint               *pos_y);
void                 marker_prefs_set_window_position            (gint                pos_x,
                                                                  gint                pos_y);
guint                marker_prefs_get_editor_pane_width          (void);
void                 marker_prefs_set_editor_pane_width          (guint               width);
gboolean             marker_prefs_get_show_sidebar               (void);
void                 marker_prefs_set_show_sidebar               (gboolean            state);
char                *marker_prefs_get_syntax_theme               (void);
void                 marker_prefs_set_syntax_theme               (const char         *theme);
gboolean             marker_prefs_get_use_syntax_theme           (void);
void                 marker_prefs_set_use_syntax_theme           (gboolean            state);
gboolean             marker_prefs_get_replace_tabs               (void);
void                 marker_prefs_set_replace_tabs               (gboolean            state);
guint                marker_prefs_get_tab_width                  (void);
void                 marker_prefs_set_tab_width                  (guint               width);
gboolean             marker_prefs_get_auto_indent                (void);
void                 marker_prefs_set_auto_indent                (gboolean            state);
gboolean             marker_prefs_get_show_spaces                (void);
void                 marker_prefs_set_show_spaces                (gboolean            state);
gboolean             marker_prefs_get_spell_check                (void);
void                 marker_prefs_set_spell_check                (gboolean            state);
gchar               *marker_prefs_get_spell_check_language       (void);
void                 marker_prefs_set_spell_check_language       (const gchar        *lang);
gboolean             marker_prefs_get_show_line_numbers          (void);
void                 marker_prefs_set_show_line_numbers          (gboolean            state);
gboolean             marker_prefs_get_highlight_current_line     (void);
void                 marker_prefs_set_highlight_current_line     (gboolean            state);
gboolean             marker_prefs_get_wrap_text                  (void);
void                 marker_prefs_set_wrap_text                  (gboolean            state);
gboolean             marker_prefs_get_show_right_margin          (void);
void                 marker_prefs_set_show_right_margin          (gboolean            state);
guint                marker_prefs_get_right_margin_position      (void);
void                 marker_prefs_set_right_margin_position      (guint               position);
gboolean             marker_prefs_get_auto_save                  (void);
void                 marker_prefs_set_auto_save                  (gboolean            state);
char                *marker_prefs_get_css_theme                  (void);
void                 marker_prefs_set_css_theme                  (const char         *theme);
gboolean             marker_prefs_get_use_css_theme              (void);
void                 marker_prefs_set_use_css_theme              (gboolean            state);
char                *marker_prefs_get_highlight_theme            (void);
void                 marker_prefs_set_highlight_theme            (const char         *theme);
gboolean             marker_prefs_get_use_mathjs                 (void);
void                 marker_prefs_set_use_mathjs                 (gboolean            state);
gdouble              makrer_prefs_get_zoom_level                 (void);
void                 marker_prefs_set_zoom_level                 (gdouble             val);
gboolean             marker_prefs_get_use_highlight              (void);
void                 marker_prefs_set_use_highlight              (gboolean            state);
gboolean             marker_prefs_get_use_mermaid                (void);
void                 marker_prefs_set_use_mermaid                (gboolean            state);
gboolean             marker_prefs_get_use_charter                (void);
void                 marker_prefs_set_use_charter                (gboolean            state);
gboolean             marker_prefs_get_gnome_appmenu              (void);
void                 marker_prefs_set_gnome_appmenu              (gboolean            state);
MarkerViewMode       marker_prefs_get_default_view_mode          (void);
void                 marker_prefs_set_default_view_mode          (MarkerViewMode      view_mode);
MarkerMathBackEnd    marker_prefs_get_math_backend               (void);
void                 marker_prefs_set_math_backend               (MarkerMathBackEnd   backend);
void                 marker_prefs_load                           (void);
void                 marker_prefs_show_window                    (void);
GList               *marker_prefs_get_available_stylesheets      (void);
GList               *marker_prefs_get_available_syntax_themes    (void);

#endif
