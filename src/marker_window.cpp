#include "marker_window.hpp"

using namespace Gtk;

MarkerWindow::MarkerWindow() :
    paned(ORIENTATION_HORIZONTAL)
{
    set_default_size(500, 600);
    set_titlebar(headerbar);
    
    headerbar.set_title("Editing: Untitled");
    headerbar.set_show_close_button(true);
    headerbar.pack_start(preview_button);
    
    preview_button.set_image_from_icon_name("view-dual-symbolic");
    
    web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    Gtk::Widget* web_view_widget = Glib::wrap(GTK_WIDGET(web_view));
    
    add(paned);
    
    paned.add1(source_view);
    paned.add2(*web_view_widget);
    
    show_all_children();
}

MarkerWindow::~MarkerWindow()
{

}

