#ifndef __MARKER_EDITOR_HPP__
#define __MARKER_EDITOR_HPP__

#include <gtkmm/box.h>
#include <gtkmm/paned.h>
#include <gtksourceviewmm/view.h>
#include <webkit2/webkit2.h>

namespace Marker {

class Editor : public Gtk::Box {
public:
    Editor();
    ~Editor();

private:
    void init_ui();

    std::string title = "Untitled.md";
    std::string subtitle = "";

    Gsv::View source_view;
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    Gtk::Paned paned = Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL);
};

}

#endif

