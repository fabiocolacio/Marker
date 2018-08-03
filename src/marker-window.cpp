#include "marker.hpp"

#include "marker-window.hpp"

using namespace Marker;

Window::Window() :
    Gtk::ApplicationWindow(Marker::app)
{
    init_ui();
}

Window::~Window()
{

}

void Window::init_ui()
{
    set_default_size(500, 500);
    add(editor);
}

