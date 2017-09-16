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
prefs_cb(GSimpleAction* action,
         GVariant*      parameter,
         gpointer       user_data);
         
void
about_cb(GSimpleAction* action,
         GVariant*      parameter,
         gpointer       user_data);

void
quit_cb(GSimpleAction*  action,
        GVariant*      parameter,
        gpointer       user_data);
        
extern GActionEntry APP_MENU_ACTION_ENTRIES[];


#endif

