#!/bin/sh

prefix=${MESON_INSTALL_PREFIX}
datadir=$prefix"/share"

# Distro packagers define DESTDIR and they don't
# want/need us to do the below
if [ -z $DESTDIR ]; then
    echo "Compiling GSchema..."
    glib-compile-schemas $datadir"/glib-2.0/schemas"

    echo "Updating icon cache..."
    gtk-update-icon-cache -f -t $datadir"/icons/hicolor"

    echo "Updating desktop database..."
    update-desktop-database -q $datadir"/applications"

    chmod -R o=+rx $datadir"/com.github.fabiocolacio.marker/scripts"
    chmod -R g=+rx $datadir"/com.github.fabiocolacio.marker/scripts"
fi
