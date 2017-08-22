# Marker

Marker is a markdown editor for linux made with Gtk+-3.0

## Screenshots

![scrot.png](scrot.png)

## Installation From Source

### Dependencies

* gtk3-devel
* gtksourceview3-devel
* webkitgtk3-devel
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