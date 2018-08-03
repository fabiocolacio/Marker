#include "marker-editor.hpp"

using namespace Marker;

Editor::Editor() : Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
    init_ui();
}

Editor::~Editor()
{

}

void Editor::init_ui()
{
    pack_start(paned);

    auto preview = Glib::wrap(GTK_WIDGET(web_view));

    paned.add1(source_view);
    paned.add2(*preview);
}

