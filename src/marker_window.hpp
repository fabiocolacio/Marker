#ifndef MARKER_WINDOW_HPP
#define MARKER_WINDOW_HPP

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/paned.h>
#include <gtksourceviewmm/view.h>
#include <webkit2/webkit2.h>

using namespace Gtk;

class MarkerWindow : public Window
{
private:
    HeaderBar headerbar;
    Button refresh_button;
    Paned paned;
    WebKitWebView* web_view;
    Gsv::View source_view;
public:
    MarkerWindow();
    ~MarkerWindow();
    void refresh_web_view();
};

#endif

