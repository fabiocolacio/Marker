extern crate gtk;
use gtk::prelude::*;
use self::gtk::Application;
use self::gtk::ApplicationWindow;
use self::gtk::MenuBar;
use self::gtk::Builder;

extern crate gio;
use self::gio::prelude::*;
use self::gio::MenuModel;

use gui::session::Session;

pub struct Window {
    sessions: Vec<Session>,
    window: ApplicationWindow
}

impl Window {
    pub fn new(app: &Application) -> Window {
        let mut sessions = Vec::new();
        let empty_session = Session::new();
        sessions.push(empty_session);

        let window = ApplicationWindow::new(app);
        window.add(&sessions[0].as_widget());

        Window { sessions, window }
    }

    pub fn as_application_window(&self) -> ApplicationWindow {
        self.window.clone()
    }
}

