#include "marker_window.hpp"

using namespace Gtk;

MarkerWindow::MarkerWindow()
{
    set_default_size(500, 600);
    set_title("Editing: Untitled");
    show_all_children();
}

MarkerWindow::~MarkerWindow()
{

}

