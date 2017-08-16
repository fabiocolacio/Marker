#include <iostream>
#include <gtkmm/application.h>

#include "marker_window.hpp"

using namespace std;

int
main(int argc,
     char** argv)
{
    int ret = 0;
    
    auto app = Gtk::Application::create(argc, argv,
        "com.github.fabiocolacio.marker");
    MarkerWindow window;
    ret = app->run(window);

    return ret;
}

