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

LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARYPATH
export LD_LIBRARY_PATH

jpeg2yuv -f 25 -I p -j %05d.jpg | mpeg2enc -f 13 -F 3 -b 10000 -o ${mpegname}

exit 0
