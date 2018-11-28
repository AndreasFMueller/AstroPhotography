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

convert ${fitsimage} -combine -set colorspace sRGB ${tmpfile}

#rm -f ${fitsimage}

convert -size 2760x2080 xc:none -fill ${tmpfile} \
	-draw "circle 1380,1040 1,1040" \
	${jpegimage}

exit 0
