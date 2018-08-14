extern crate gtk;
extern crate sourceview;
extern crate webkit2gtk;

use self::gtk::prelude::*;
use self::gtk::Widget;
use self::gtk::Paned;
use self::gtk::Orientation;
use self::gtk::TextViewExt;
use self::gtk::TextBufferExt;
use self::sourceview::View;
use self::webkit2gtk::WebView;

pub struct Session {
    sourceview: View,
    webview: WebView,
    paned: Paned
}

impl Session {
    pub fn new() -> Session {
        let paned = Paned::new(Orientation::Horizontal);
        paned.show();

        let sourceview = View::new();
        sourceview.show();
        paned.add1(&sourceview);

        let webview = WebView::new();
        webview.show();
        paned.add2(&webview);

        Session {
            sourceview,
            webview,
            paned
        }
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

