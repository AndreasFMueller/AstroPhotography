#! /bin/bash
#
#  buildimage.sh -- build the sky image for the current position
#
# (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
#
set -x

rightascension=${1}
declination=${2}
longitude=${3}
latitude=${4}

tmpfile=${TMPDIR}/sky$$.png
trap "rm -f $tmpfile" 0 1 2 3 15

ASTROSKY=/Users/afm/Projects/AstroPhotography/gui/snowgui/astrosky/astrosky.app/Contents/MacOS/astrosky

${ASTROSKY} --debug --size=640						\
	--rightascension=${rightascension} --declination=${declination}	\
	--longitude=${longitude} --latitude=${latitude}			\
	${tmpfile}

scp ${tmpfile} cesario.othello.ch:/var/www/html/telescope/telescopesky.png

rm ${tmpfile}

exit 0
