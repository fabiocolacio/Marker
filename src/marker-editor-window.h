#ifndef __MARKER_EDITOR_WINDOW_H__
#define __MARKER_EDITOR_WINDOW_H__

#include <gtk/gtk.h>

#include "marker-preview.h"

G_BEGIN_DECLS

#define MARKER_TYPE_EDITOR_WINDOW (marker_editor_window_get_type ())

G_DECLARE_FINAL_TYPE(MarkerEditorWindow,
                     marker_editor_window,
                     MARKER,
                     EDITOR_WINDOW,
                     GtkApplicationWindow)

typedef enum
{
  EDITOR_ONLY_MODE,
  PREVIEW_ONLY_MODE,
  DUAL_PANE_MODE,
  DUAL_WINDOW_MODE
} MarkerEditorWindowViewMode;

/**
 * Constructs a new MarkerEditorWindow for the given GtkApplication
 *
 * @param app The parent application for this window
 *
 * @return A newly constructed MarkerEditorWindow
 */
MarkerEditorWindow*
marker_editor_window_new(GtkApplication* app);

/**
 * Constructs a new MarkerEditorWindow for the given GtkApplication, with
 * the given file open.
 *
 * @param app The parent application for this window
 * @param file The file to open in the new window
 *
 * @return A newly constructed MarkerEditorWindow with the contents of file
 */
MarkerEditorWindow*
marker_editor_window_new_from_file(GtkApplication* app,
                                   GFile*          file);

/**
 * Opens a file in a an existing MarkerEditorWindow
 *
 * @param window The window to open the file into
 * @param file The file to open into the window
 */
void
marker_editor_window_open_file(MarkerEditorWindow* window,
                               GFile*              file);

/**
 * Saves the file being edited in an existing MarkerEditorWindow
 *
 * @param window The window from which to save
 * @param file The file to save to
 */
void
marker_editor_window_save_file(MarkerEditorWindow* window,
                               GFile*              file);

/**
 * Get the markdown from the editor pane of a MarkerEditorWindow
 *
 * @param window The window from which to retrieve the markdown
 *
 * @return An allocated, null-terminated, UTF-8 encoded string of Markdown text
 */
gchar*
marker_editor_window_get_markdown(MarkerEditorWindow* window);

/**
 * Refresh the HTML preview of a MarkerEditorWindow
 *
 * @param window The window to refresh the preview for
 */
void
marker_editor_window_refresh_preview(MarkerEditorWindow* window);

/**
 * Attempt to close a MarkerEditorWindow. If there are unsaved changes in the
 * document, a confirmation dialog will appear.
 *
 * @param window The window to close
 */
void
marker_editor_window_try_close(MarkerEditorWindow* window);

/**
 * Set the syntax theme for a MarkerEditorWindow
 *
 * @param window The window to set the syntax theme for
 * @param theme The name of the theme to set
 */
void
marker_editor_window_set_syntax_theme(MarkerEditorWindow* window,
                                      const char*         theme);

/**
 * Enable or disable syntax theme for a MarkerEditorWindow
 *
 * @param window The window to set the syntax theme for
 * @param state TRUE if the syntax theme should be enabled, FALSE otherwise
 */
void
marker_editor_window_set_use_syntax_theme(MarkerEditorWindow* window,
                                          gboolean            state);

/**
 * Enable or disable line numbers for a MarkerEditorWindow
 *
 * @param window The window to set line numbers for
 * @param state The state to set line numbers to
 */
void
marker_editor_window_set_show_line_numbers(MarkerEditorWindow* window,
                                           gboolean            state);

/**
 * Enable or disable spell check a MarkerEditorWindow
 *
 * @param window The window to set line numbers for
 * @param state The state to set the spell checker to
 */
void
marker_editor_window_set_spell_check(MarkerEditorWindow* window,
                                     gboolean            state);

/**
 * Set spell check language a MarkerEditorWindow
 *
 * @param window The window to set line numbers for
 * @param lang The language to set the spell checker to
 */
void marker_editor_window_set_spell_lang(MarkerEditorWindow* window,
                                    gchar*            lang);

/**
 * Enable or disable the replacement of the the tabs by spaces.
 * 
 * @param window The window to change the setting for
 * @param state The state of the setting to
 **/
void marker_editor_window_set_replace_tabs(MarkerEditorWindow*  window,
                                           gboolean             state);

/**
 * Enable or disable the auto indentation.
 * 
 * @param window The window to change the setting for
 * @param state The state of the setting to
 **/
void marker_editor_window_set_auto_indent(MarkerEditorWindow*  window,
                                           gboolean             state);
                                           
void marker_editor_window_set_tab_width(MarkerEditorWindow*   window,
                                        guint                 value);
/**
 * Enable or disable highlighting the current line for a MarkerEditorWindow
 *
 * @param window The window to set the 
 */
void
marker_editor_window_set_highlight_current_line(MarkerEditorWindow* window,
                                                gboolean            state);

void
marker_editor_window_set_wrap_text(MarkerEditorWindow* window,
                                   gboolean            state);

void
marker_editor_window_set_show_right_margin(MarkerEditorWindow* window,
                                           gboolean            state);

void
marker_editor_window_set_right_margin_position(MarkerEditorWindow* window,
                                               guint               value);

void
marker_editor_window_apply_prefs(MarkerEditorWindow* window);

void
marker_editor_window_set_title_filename(MarkerEditorWindow* window);

void
marker_editor_window_set_title_filename_unsaved(MarkerEditorWindow* window);

void
marker_editor_window_set_view_mode(MarkerEditorWindow*        window,
                                   MarkerEditorWindowViewMode mode);

MarkerPreview*
marker_editor_window_get_preview(MarkerEditorWindow* window);

G_END_DECLS

#endif

