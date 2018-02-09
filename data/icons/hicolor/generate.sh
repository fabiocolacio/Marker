#! /bin/sh

for n in $(ls *svg | sed 's/.svg//'); do inkscape -z -e $n.png -w $1 -h $1 $n.svg; done