#!/bin/bash

outdir="$1"
module="$2"			# e.g. qtcharts
program="$3"		# e.g. donutbreakdown
filename="$4"		# e.g. mainslice.h

curl -s "https://doc.qt.io/archives/qt-5.11/$module-$program-$(echo "$filename" | tr . -).html" | sed -n "/<!-- \$\$\$${program}\/${filename}-description -->/,/<!-- @@@${program}\/${filename} -->/p" | sed 's/<[^>]*>//g' | sed 's/&lt;/</g' | sed 's/&gt;/>/g' | sed 's/&amp;/\&/g'  >  "$outdir/$filename"

