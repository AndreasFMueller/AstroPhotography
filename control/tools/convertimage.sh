#! /bin/sh
#
# convert a FITS image to JPEG
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
fitsimage=${1}
jpegimage=`dirname ${fitsimage}`/`basename ${fitsimage} .fits`.jpeg

convert ${fitsimage} -rotate 90 -contrast-stretch 20000x65535 ${jpegimage}
exit 0
