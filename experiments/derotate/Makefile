#
# Makefile -- build test programs for derotation
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
CPPFLAGS = -I/usr/local/include/opencv2
LDFLAGS = -lopencv_core -lopencv_highgui -lopencv_imgproc -llapack

all:	setup rotate radontransform motion

rotate:	rotate.cpp
	g++ $(CPPFLAGS) -o rotate -Wall -g -O2 rotate.cpp $(LDFLAGS)

TESTIMG = M11.jpg
ANGLE = 2.4

test:	rotate
	./rotate -l 64 -a $(ANGLE) -l 512 -x 22 -y 24  $(TESTIMG) rotated.png recovered.png

setup:	setup.cpp
	g++ $(CPPFLAGS) -o setup -Wall -g -O2 setup.cpp $(LDFLAGS)

EDGE = 800
test-setup:	setup
	./setup -a 37 -x 23 -y 12 -w $(EDGE) -h $(EDGE) poc.jpg after.jpg
	./setup -a 0 -x 0 -y 0 -w $(EDGE) -h $(EDGE) poc.jpg before.jpg

radon.o:	radon.cpp
	g++ $(CPPFLAGS) -c -o radon.o -Wall -g -O2 radon.cpp

radontransform.o:	radontransform.cpp
	g++ $(CPPFLAGS) -c -o radontransform.o -Wall -g -O2 radontransform.cpp

radontransform:	radontransform.o radon.o
	g++ -o radontransform radontransform.o radon.o $(LDFLAGS)

RADONRESOLUTION=512

test-radon:	radontransform
	./radontransform -w $(RADONRESOLUTION) -h $(RADONRESOLUTION) before.jpg before-radon.jpg
	./radontransform -w $(RADONRESOLUTION) -h $(RADONRESOLUTION) after.jpg after-radon.jpg

motion.o:	motion.cpp
	g++ $(CPPFLAGS) -c -o motion.o motion.cpp
motion:	motion.o radon.o
	g++ -o motion motion.o radon.o $(LDFLAGS)

test-motion:	motion
	./motion before-radon.jpg after-radon.jpg
