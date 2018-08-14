extern crate gtk;

use gtk::prelude::*;
use session::Session;
use std::path::Path;
use std::io::Result;

pub struct Window {
    sessions: Vec<Session>,
    session_id: usize,
    window: gtk::ApplicationWindow
}

impl Window {
    pub fn new(app: &gtk::Application) -> Self {
        let mut sessions = Vec::new();
        let empty_session = Session::new();
        sessions.push(empty_session);
        let session_id = 0;

        let window = gtk::ApplicationWindow::new(app);
        window.add(&sessions[session_id].as_widget());

        Self { sessions, session_id, window }
    }

    pub fn new_from_file<P: AsRef<Path>>(app: &gtk::Application, path: P) -> Result<Self> {
        let mut window = Window::new(app);
        let session_id = window.session_id;
        window.close_session(session_id);
        window.add_session(Session::new_from_file(path)?);
        Ok(window)
    }

    pub fn as_application_window(&self) -> gtk::ApplicationWindow {
        self.window.clone()
    }

    pub fn add_session(&mut self, session: Session) {
        if !self.sessions.is_empty() {
            let active_session = &self.sessions[self.session_id];
            let active_widget = active_session.as_widget();
            self.window.remove(&active_widget);
        }

        self.window.add(&session.as_widget());
        self.session_id = 
            if self.sessions.is_empty() { 0 }
            else { self.sessions.len() - 1 };

        self.sessions.push(session);
    }

    pub fn close_session(&mut self, idx: usize) {
        if self.sessions.is_empty() {
            return;
        }

        let session = self.sessions.remove(idx);
        self.window.remove(&session.as_widget());

        if (self.session_id > 0) {
            self.session_id -= 1;
        }

        if !self.sessions.is_empty() {
            let active_widget = self.sessions[self.session_id].as_widget();
            self.window.add(&active_widget); 
        }
    }
}

