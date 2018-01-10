#ifndef __MARKER_H__
#define __MARKER_H__

#include <gtk/gtk.h>

GtkApplication*
marker_get_app();

void
marker_create_new_window();

void
marker_create_new_window_from_file(GFile* file);

void
marker_quit();

void
marker_prefs_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data);

void
marker_about_cb(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       user_data);

void
marker_quit_cb(GSimpleAction*  action,
               GVariant*       parameter,
               gpointer        user_data);

void
marker_shortcuts_cb(GSimpleAction* action,
                    GVariant*      parameter,
                    gpointer       user_data);

gboolean
marker_has_app_menu();

extern const int APP_MENU_ACTION_ENTRIES_LEN;

extern const GActionEntry APP_MENU_ACTION_ENTRIES[];

#endif
