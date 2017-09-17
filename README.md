# <img width="30" src="data/com.github.fabiocolacio.marker.svg"/>Marker

Marker is a markdown editor for linux made with Gtk+-3.0

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

**Warning:** If you want a relatively stable experience, download a
release tarball rather than cloning the repository! Breaking changes and
unfinished/untested work is often pushed to the repository, as this is still
very early in the project's lifespan!

```
$ git clone https://github.com/fabiocolacio/Marker.git
$ cd Marker
$ mkdir build && cd build
# set -DWKHTMLTOX=OFF to disable wkhtmltox export options
# set -DPANDOC=OFF to disable pandoc export options
$ cmake -DCMAKE_INSTALL_PREFIX=/usr -DWKHTMLTOX=ON -DWKPANDOC=ON ..
$ make
$ sudo make install
```
