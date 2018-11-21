#! /bin/bash
#
# convert a FITS image to JPEG
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
fitsimage=${1}
jpegimage=`echo ${fitsimage} | sed -e 's/-.*.fits/.jpeg/'`

convert ${fitsimage} -set colorspace sRGB ${jpegimage}

exit 0
