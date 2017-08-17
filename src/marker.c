#include <gtksourceview/gtksource.h>
#include <webkit2/webkit2.h>
#include <stdio.h>
#include <stdlib.h>

#include "marker.h"

struct Data{
    GtkWidget* source_view;
    GtkWidget* web_view;
};

static void
refresh_web_view(GtkWidget* widget,
                  gpointer   user_data)
{
    struct Data* views = user_data;
    printf("source: %p\n web: %p\n\n", views->source_view, views->web_view);
}

GtkWidget*
marker_window_new()
{
    GtkWidget* window;
    GtkWidget* paned;
    GtkWidget* source_view;
    GtkWidget* web_view;
    GtkWidget* header_bar;
    GtkWidget* more_items_btn;
    GtkWidget* refresh_btn;
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    source_view = gtk_source_view_new();
    web_view = webkit_web_view_new();
    header_bar = gtk_header_bar_new();
    more_items_btn = gtk_button_new_from_icon_name("view-more-symbolic",
                                                   GTK_ICON_SIZE_LARGE_TOOLBAR);
    refresh_btn = gtk_button_new_from_icon_name("view-refresh-symbolic",
                                                GTK_ICON_SIZE_LARGE_TOOLBAR);
    
    printf("source: %p\n web: %p\n\n", source_view, web_view);
    
    struct Data* views = malloc(sizeof(struct Data));
    views->source_view = source_view;
    views->web_view = web_view; 
                                              
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(refresh_web_view), views);
                                                
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Untitled.md");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), refresh_btn);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), more_items_btn);
    
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    
    gtk_paned_add1(GTK_PANED(paned), source_view);
    gtk_paned_add2(GTK_PANED(paned), web_view);
    
    gtk_container_add(GTK_CONTAINER(window), paned);
    
    return window;
}

