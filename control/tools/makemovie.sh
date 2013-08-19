#! /bin/sh
#
# makemovie.sh -- produce an MPEG2 movie from all the JPEG images found in
#                 a directory
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
directory=${1}

day=`basename ${directory}`
dir=`dirname ${directory}`
month=`basename ${dir}`
dir=`dirname ${dir}`
year=`basename ${dir}`

mpegname=${year}${month}${day}.mpg

sleep 2

cd ${directory}
files=`echo *.jpeg | wc -w`

echo producing ${mpegname} from ${files} JPEG images

exit 0
