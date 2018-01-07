#ifndef __MARKER_H__
#define __MARKER_H__

#include <gtk/gtk.h>

/**
 * Gets and instance of the current GtkApplication instance.
 *
 * @return The current GtkApplication instance
 */
GtkApplication*
marker_get_app();

/**
 * Creates an empty editor window
 */
void
marker_create_new_window();

/**
 * Opens a file in a new editor window.
 *
 * @param file The file to open in a new window
 */
void
marker_create_new_window_from_file(GFile* file);

/**
 * Attempts to quit the application. If there are unsaved documents, a
 * dialog will appear asking the user if changes should be saved.
 */
void
marker_quit();

/**
 * A callback that is called whenever the preferences button in the
 * appmenu is pressed.
 */
void
marker_prefs_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data);

/**
 * A callback that is called whenever the about button in the
 * appmenu is pressed.
 */
void
marker_about_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data);

/**
 * A callback that is called whenever the quit button in the appmenu
 * is pressed.
 */
void
marker_quit_cb(GSimpleAction*  action,
               GVariant*       parameter,
               gpointer        user_data);

/**
 * A callback that is called whenever the shortcuts button in the appmenu
 * is pressed.
 */
void
marker_shortcuts_cb(GSimpleAction* action,
                    GVariant*      parameter,
                    gpointer       user_data);

/**
 * Indicates whether the appmenu exists or not.
 *
 * @return true if the appmenu exists, false otherwise.
 */
gboolean
marker_has_app_menu();

/**
 * An array containing all of the GActions used by the appmenu
 */
extern GActionEntry APP_MENU_ACTION_ENTRIES[];

#endif
