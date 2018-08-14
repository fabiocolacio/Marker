extern crate gtk;
extern crate sourceview;
extern crate webkit2gtk;

use gtk::prelude::*;
use gtk::Widget;
use gtk::Paned;
use gtk::Orientation;
use gtk::TextViewExt;
use gtk::TextBufferExt;
use sourceview::prelude::*;
use sourceview::View;
use webkit2gtk::WebView;
use std::path::Path;
use std::fs::File;
use std::io::Result;
use std::io::Read;

pub struct Session {
    sourceview: View,
    webview: WebView,
    paned: Paned
}

impl Session {
    pub fn new() -> Self {
        let paned = Paned::new(Orientation::Horizontal);
        paned.show();

        let sourceview = View::new();
        sourceview.show();
        paned.add1(&sourceview);

        let webview = WebView::new();
        webview.show();
        paned.add2(&webview);

        Self {
            sourceview,
            webview,
            paned
        }
    }

    pub fn new_from_file<P: AsRef<Path>>(path: P) -> Result<Self> {
        let mut file = File::open(path)?;
        let mut contents = String::new();
        file.read_to_string(&mut contents)?;

        let session = Self::new();
        let buffer = session.sourceview.get_buffer().unwrap();
        buffer.set_text(&contents);

        Ok(session)
    }

    pub fn as_widget(&self) -> Widget {
        self.paned.clone().upcast::<Widget>()
    }

    pub fn get_md(&self) -> Option<String> {
        let buffer = self.sourceview.get_buffer()?;
        let start = buffer.get_start_iter();
        let end = buffer.get_end_iter();
        buffer.get_text(&start, &end, false)
    }
}

