#! /bin/bash
#
# convert a FITS image to JPEG
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
fitsimage=${1}
jpegimage=`echo ${fitsimage} | sed -e 's/-.*.fits/.jpeg/'`

tmpfile=/tmp/t$$.jpeg
trap "rm -f ${tmpfile}" 0 1 2 3 15

convert ${fitsimage} -rotate 90 -contrast-stretch 25%x4% ${tmpfile}

convert -size 1040x1040 xc:none -fill ${tmpfile} -draw "circle 520,520 1,520" \
	${jpegimage}

exit 0
