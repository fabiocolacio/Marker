#! /bin/sh
cd scalable/actions/
for n in $(ls *svg | sed 's/.svg//'); do gtk-encode-symbolic-svg -o ../../$1/actions/ $n.svg $1; done
