# <img width="30" src="data/com.github.fabiocolacio.marker.svg"/>Marker

Marker is a markdown editor for linux made with Gtk+-3.0

**Note:** Marker is still very new and under heavy development. Use at your own risk!
If you do try it out, please report any glitches or feature requests through the
issues tab on [Marker's github repository](https://github.com/fabiocolacio/Marker)

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

* gtk3-devel
* gtksourceview3-devel
* webkitgtk3-devel
* wkhtmltox c libraries
    * It is recomended to download this dependency from the
      [wkhtmltopdf website](https://wkhtmltopdf.org/downloads.html),
      as the version provided in some distro repositories will not
      work properly.
* pandoc
* cmake

### Build Instructions

```
$ git clone https://github.com/fabiocolacio/Marker.git
$ cd Marker
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
$ sudo make install
```