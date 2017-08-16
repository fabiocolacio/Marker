#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "marker_window.hpp"

using namespace Gtk;

MarkerWindow::MarkerWindow() :
    paned(ORIENTATION_HORIZONTAL)
{
    set_default_size(500, 600);
    set_titlebar(headerbar);
    
    headerbar.set_title("Editing: Untitled");
    headerbar.set_show_close_button(true);
    headerbar.pack_start(refresh_button);
    
    refresh_button.set_image_from_icon_name("view-refresh-symbolic");
    refresh_button.signal_clicked().connect(
        sigc::mem_fun(this, &MarkerWindow::refresh_web_view));
    
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

void MarkerWindow::refresh_web_view()
{
    Glib::RefPtr<const TextBuffer> buffer = source_view.get_buffer();
    if (buffer)
    {
        Glib::ustring text = buffer->get_text(false);
        std::ofstream tmpfile;
        tmpfile.open("tmp.md");
        tmpfile << text;
        tmpfile.close();
        system("pandoc -s -o tmp.html tmp.md");
        std::stringstream uri;
        char cwd[50];
        getcwd(cwd, 50);
        uri << "file://" << std::string(cwd) << "/tmp.html";
        webkit_web_view_load_uri(web_view, uri.str().c_str());
    }
}

