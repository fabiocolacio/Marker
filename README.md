# <img width="30" src="data/com.github.fabiocolacio.marker.svg"/>Marker

Marker is a markdown editor for linux made with GTK4 and libadwaita

**NOTE:** Issues regarding markdown parsing should go to the [scidown repo](https://github.com/mandarancio/scidown).

## Features

* View and edit markdown documents
* HTML and LaTeX conversion of markdown documents with [scidown](https://github.com/Mandarancio/scidown/)
  * Support for YAML headers
  * Document classes
  * Beamer/presentation mode (`class: beamer`)
  * Abstract sections
  * Table of Contents
  * External document inclusion
  * Equations, figures, table and listings with reference id and caption
  * Internal references
* Extra scientific syntax of SciDown on its [wiki](https://github.com/Mandarancio/scidown/wiki/)
* TeX math rendering with [KaTeX](https://khan.github.io/KaTeX/) or [MathJax](mathjax.org/)
* Support for [mermaid](https://mermaidjs.github.io/) diagrams
* Support for [charter](https://github.com/Mandarancio/charter/) for plotting
* Syntax highlighting for code blocks with [highlight.js](https://highlightjs.org/)
* Integrated sketch editor
* Flexible export options with [pandoc](https://pandoc.org/)
  * PDF
  * RTF
  * ODT
  * DOCX
* Custom CSS themes
* Custom syntax themes
* Native GTK4 / libadwaita application

## Screenshots

![scrot.png](help/C/figures/scrot.png)

![scrot1.png](help/C/figures/scrot1.png)

![scrot2.png](help/C/figures/scrot2.png)

![sketcher.png](help/C/figures/sketcher.png)

![slides.png](help/C/figures/slides.png)

## Packages

* [Fedora (thanks to @tim77)](https://src.fedoraproject.org/rpms/marker)
* [Flathub (thanks to @jsparber and @bertob)](https://flathub.org/apps/details/com.github.fabiocolacio.marker)
* [Arch Linux AUR (thanks to @mmetak)](https://aur.archlinux.org/packages/marker-git/)
* [Arch Linux official package (thanks to @City-busz)](https://archlinux.org/packages/extra/x86_64/marker/)
* [Ubuntu PPA Stable (thanks to @apandada1 and @olivierb2)](https://launchpad.net/~apandada1/+archive/ubuntu/marker)
* [Ubuntu PPA Daily builds (thanks to @apandada1 and @olivierb2)](https://launchpad.net/~apandada1/+archive/ubuntu/marker-daily)
* [![Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-black.svg)](https://snapcraft.io/marker)


## Installation From Source

### Dependencies

* meson >= 0.59 (install only)
* gtk4-devel >= 4.14
* libadwaita-devel >= 1.6
* gtksourceview5-devel
* webkitgtk6.0-devel
* libspelling-devel
* pandoc
* itstool

On Debian/Ubuntu: `libgtk-4-dev libadwaita-1-dev libgtksourceview-5-dev libwebkitgtk-6.0-dev libspelling-1-dev meson ninja-build itstool gettext pandoc`

### Build Instructions

**Note:** Marker no longer installs a WebKit web extension; if you are
updating from a GTK3 build you can remove the old
`<prefix>/lib/Marker.extensions` directory.

**Note:** For a more stable experience, users are recommended download
[release tarball](https://github.com/fabiocolacio/Marker/releases) rather
than cloning from master.

```bash
$ git clone https://github.com/fabiocolacio/Marker.git
$ cd Marker
$ git submodule update --init --recursive
$ mkdir build && cd build
$ meson .. --prefix /usr
$ ninja
$ sudo ninja install
```

## Donations/Tips

If you like Marker and would like to support the development of this project, please donate below!

[<img height="30" src="donate.png" alt="PayPal"/>](https://www.paypal.me/fabiocolacio)
