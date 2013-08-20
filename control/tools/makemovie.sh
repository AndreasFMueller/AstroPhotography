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

mpegname=${year}${month}${day}

cd ${directory}
files=`echo *.jpeg | wc -w`

echo producing ${mpegname} from ${files} JPEG images

if type mpeg2enc
then
	ls *.jpeg jpeg2yuv -f 25 -I p | \
		mpeg2enc -f 13 -F 3 -b 10000 -o ${2-test}.mpg
fi

exit 0
