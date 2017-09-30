# <img width="30" src="data/com.github.fabiocolacio.marker.svg"/>Marker

Marker is a markdown editor for linux made with Gtk+-3.0

**Warning:** This app is still very early in development. There are many bugs and
unfinished features.

## Features

* Opening markdown documents
* HTML Preview of markdown documents
* Editing markdown documents
* Flexible export options through pandoc
  * HTML
  * PDF
  * RTF
  * ODT
  * DOCX
  * LaTeX
* Custom CSS themes
* Custom syntax themes
* Native Gtk3+ application with full theme support

## Screenshots

![scrot.png](scrot.png)

![scrot1.png](scrot1.png)

## Installation From Source

### Dependencies

* meson >= 0.37.0 (install only)
* gtk3-devel >= 3.20
* gtksourceview3-devel
* webkitgtk4-devel
* wkhtmltox c libraries
  * It is recomended to download this dependency from the
    [wkhtmltopdf website](https://wkhtmltopdf.org/downloads.html),
    as the version provided in some distro repositories will not
    work properly.
* pandoc

### Build Instructions

```
$ git clone https://github.com/fabiocolacio/Marker.git
$ cd Marker
$ mkdir build && cd build
$ meson ..
$ ninja
$ sudo ninja install
```
