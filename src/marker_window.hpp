#ifndef MARKER_WINDOW_HPP
#define MARKER_WINDOW_HPP

#include <gtkmm/window.h>
#include <gtksourceviewmm/view.h>

using namespace Gtk;

class MarkerWindow : public Window
{
private:
    Gsv::View source_view;
public:
    MarkerWindow();
    ~MarkerWindow();
};

#endif

