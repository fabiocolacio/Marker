#include <gtkmm/application.h>
#include <iostream>

#include "marker-window.hpp"

namespace Marker {

auto app = Gtk::Application::create(
    MARKER_ID,
    Gio::APPLICATION_HANDLES_OPEN);

void activate()
{
    std::cout << "Activated!" << std::endl;
    auto win = new Window;
    win->set_default_size(500, 500);
    win->show_all();
}

void open(const Gio::Application::type_vec_files &files, const Glib::ustring &hint)
{
    for (auto file : files) {
        auto filename = file->get_basename();
        std::cout << "Opening file '" << filename << "'\n";
    }
}

}

int main(int argc, char *argv[])
{
    Marker::app->signal_open().connect(sigc::ptr_fun(&Marker::open));
    Marker::app->signal_activate().connect(sigc::ptr_fun(&Marker::activate));
    return Marker::app->run(argc, argv);
}

