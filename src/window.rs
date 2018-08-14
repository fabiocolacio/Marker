extern crate gtk;
use gtk::prelude::*;
use session::Session;

pub struct Window {
    sessions: Vec<Session>,
    window: gtk::ApplicationWindow
}

impl Window {
    pub fn new(app: &gtk::Application) -> Window {
        let mut sessions = Vec::new();
        let empty_session = Session::new();
        sessions.push(empty_session);

        let window = gtk::ApplicationWindow::new(app);
        window.add(&sessions[0].as_widget());

        Window { sessions, window }
    }

    pub fn as_application_window(&self) -> gtk::ApplicationWindow {
        self.window.clone()
    }
}

