extern crate gtk;
use gtk::prelude::*;
use gtk::Application;
use gtk::Builder;

extern crate gio;
use gio::prelude::*;
use  gio::MenuModel;

mod gui;

const menubar_ui: &str = include_str!("gui/ui/menubar.ui");

fn main() {
    let app = Application::new(
        "com.github.fabiocolacio.Marker",
        gio::ApplicationFlags::HANDLES_OPEN)
        .expect("Failed to initialize application");

    app.connect_startup(startup);
    app.connect_activate(activate);
    app.connect_open(open);

    app.run(&std::env::args().collect::<Vec<String>>().as_slice());
}

fn startup(app: &Application) {
    println!("startup!");
    let builder = Builder::new();
    builder.add_from_string(menubar_ui);
    let menumodel: MenuModel = builder.get_object("menubar").unwrap();
    app.set_menubar(&menumodel);
}

fn activate(app: &Application) {
    let window = gui::Window::new(app);
    let app_window = window.as_application_window();
    app_window.set_title("First Rust GTK+ Program");
    app_window.set_default_size(350, 70);
    app_window.show_all();
}

fn open(app: &Application, files: &[gio::File], hint: &str) {
    for file in files {
        let basename = file.get_basename().unwrap();
        let basename = basename.to_string_lossy();
        println!("Opening file '{}'", basename);
    }
}

