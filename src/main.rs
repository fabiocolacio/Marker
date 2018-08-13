extern crate gtk;
extern crate gio;

mod gui;

use gtk::prelude::*;
use gtk::{ Application, Button, ApplicationWindow, WindowType };
use gio::prelude::*;

fn main() {
    let app = Application::new(
        "com.github.fabiocolacio.Marker",
        gio::ApplicationFlags::HANDLES_OPEN)
        .expect("Failed to initialize application");

    app.connect_activate(activate);
    app.connect_open(open);

    app.run(&std::env::args().collect::<Vec<String>>().as_slice());
}

fn activate(app: &Application) {
    let window = ApplicationWindow::new(app);
    window.set_title("First Rust GTK+ Program");
    window.set_default_size(350, 70);
    
    let session = gui::Session::new();
    window.add(&session.as_widget());
    window.show_all();
}

fn open(app: &Application, files: &[gio::File], hint: &str) {
    for file in files {
        let basename = file.get_basename().unwrap();
        let basename = basename.to_string_lossy();
        println!("Opening file '{}'", basename);
    }
}

