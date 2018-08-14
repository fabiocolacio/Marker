extern crate gtk;
extern crate gio;
extern crate webkit2gtk;
extern crate sourceview;

mod session;
mod window;

use gtk::prelude::*;
use gio::prelude::*;
use window::*;

const MENUBAR_UI: &str = include_str!("ui/menubar.ui");

fn main() {
    let app = gtk::Application::new(
        "com.github.fabiocolacio.Marker",
        gio::ApplicationFlags::HANDLES_OPEN)
        .expect("Failed to initialize application");

    app.connect_startup(startup);
    app.connect_activate(activate);
    app.connect_open(open);

    app.run(&std::env::args().collect::<Vec<String>>().as_slice());
}

fn startup(app: &gtk::Application) {
    let builder = gtk::Builder::new();
    builder.add_from_string(MENUBAR_UI).unwrap();
    let menumodel: gio::MenuModel = builder.get_object("menubar").unwrap();
    app.set_menubar(&menumodel);
}

fn activate(app: &gtk::Application) {
    let window = Window::new(app);
    let app_window = window.as_application_window();
    app_window.set_title("First Rust GTK+ Program");
    app_window.set_default_size(350, 70);
    app_window.show_all();
}

fn open(app: &gtk::Application, files: &[gio::File], hint: &str) {
    for file in files {
        let path = file.get_path().unwrap();
        let path = path.to_string_lossy().into_owned();
        println!("Opening file '{}'", &path);

        let window = Window::new_from_file(app, &path).unwrap();
        let app_window = window.as_application_window();
        app_window.set_title(&path);
        app_window.set_default_size(600, 600);
        app_window.show_all();
    }
}

