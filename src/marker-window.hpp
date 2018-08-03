#ifndef __MARKER_WINDOW_HPP__
#define __MARKER_WINDOW_HPP__

#include <gtkmm/applicationwindow.h>

#include "marker-editor.hpp"

namespace Marker {

class Window : public Gtk::ApplicationWindow {
public:
    Window();
    ~Window();

private:
    void init_ui();

    Editor editor;
};

}

#endif

