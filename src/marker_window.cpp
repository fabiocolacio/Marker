#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <gtkmm/filechooserdialog.h>

#include "marker_window.hpp"

using namespace Gtk;

MarkerWindow::MarkerWindow() :
    paned(ORIENTATION_HORIZONTAL)
{
    set_default_size(500, 600);
    set_titlebar(headerbar);
    
    headerbar.set_title("Editing: Untitled");
    headerbar.set_show_close_button(true);
    
    headerbar.pack_start(save_button);
    headerbar.pack_start(save_as_button);
    headerbar.pack_start(open_button);
    headerbar.pack_start(refresh_button);
    
    save_button.set_image_from_icon_name("document-save-symbolic");
    save_button.signal_clicked().connect(
        sigc::mem_fun(this, &MarkerWindow::save_document));
        
    save_as_button.set_image_from_icon_name("document-save-as-symbolic");
    save_as_button.signal_clicked().connect(
        sigc::mem_fun(this, &MarkerWindow::save_document_as));
    
    open_button.set_image_from_icon_name("document-open-symbolic");
    open_button.signal_clicked().connect(
        sigc::mem_fun(this, &MarkerWindow::open_document));
    
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

void MarkerWindow::save_document()
{

}

void MarkerWindow::save_document_as()
{
    FileChooserDialog dialog(*this,
                             "Save File",
                             FILE_CHOOSER_ACTION_SAVE);
    dialog.set_do_overwrite_confirmation(true);
    dialog.set_filename("Untitled.md");
    
    int res = dialog.run();
    if (res == RESPONSE_ACCEPT)
    {
        std::string filename;
        filename = dialog.get_filename();
        std::cout << filename << std::endl;
    }
}

void MarkerWindow::open_document()
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

