#include <gtkmm/application.h>
#include <iostream>

auto app = Gtk::Application::create(
    MARKER_ID,
    Gio::APPLICATION_HANDLES_OPEN);

void activate()
{
    std::cout << "Activated!" << std::endl;
}

void open(const Gio::Application::type_vec_files &files, const Glib::ustring &hint)
{
    for (auto file : files) {
        auto filename = file->get_basename();
        std::cout << "Opening file '" << filename << "'\n";
    }
}

int main(int argc, char *argv[])
{
    app->signal_open().connect(sigc::ptr_fun(&open));
    app->signal_activate().connect(sigc::ptr_fun(&activate));
    return app->run(argc, argv);
}
