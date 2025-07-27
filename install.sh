#!/usr/bin/bash

rm -rf build
mkdir build
cd build
meson .. --prefix ~/.local
ninja
ninja install
