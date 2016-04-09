/*
 * RadonTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <Radon.h>
#include <AstroIO.h>
#include <math.h>

using namespace astro::image::radon;
using namespace astro::image;

namespace astro {
namespace test {

class RadonTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testRadon();
	void	testCircle();
	void	testCircleTransform();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(RadonTest);
	CPPUNIT_TEST(testRadon);
//	CPPUNIT_TEST(testCircle);
//	CPPUNIT_TEST(testCircleTransform);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RadonTest);

void	RadonTest::setUp() {
}

void	RadonTest::tearDown() {
}

class Circle {
	ImagePoint	_center;
	int	_radius;
public:
	Circle(const ImagePoint& center, int radius) : _center(center), _radius(radius) { }
	bool	contains(const ImagePoint& p) const {
		return (_center.distance(p) <= _radius);
	}
};

void	RadonTest::testRadon() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRadon() begin");
	int	width = 1280;
	int	height = 1024;
	Image<double>	image(width, height);
	ImageRectangle	rectangle(ImagePoint(60, 20), ImageSize(10,15));
	std::list<ImageRectangle>	rectangles;
	std::list<Circle>	circles;
	for (int counter = 0; counter < 30; counter++) {
		int	w = random() % (width - 100);
		int	h = random() % (height - 100);
		ImagePoint	origin(w, h);
		int	x = 10 + random() % 90;
		int	y = 10 + random() % 90;
		ImageSize	size(x, y);
		rectangles.push_back(ImageRectangle(origin, size));
		x = 50 + random() % (width - 100);
		y = 50 + random() % (height - 100);
		ImagePoint	center(x, y);
		int	radius = 10 + random() % 90;
		circles.push_back(Circle(center, radius));
	}
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			double	sum = 1;
			ImagePoint	p(x, y);
			std::list<ImageRectangle>::const_iterator	i;
			for (i = rectangles.begin(); i != rectangles.end(); i++) {
				if (i->contains(p)) {
					sum += 1;
				}
			}
			std::list<Circle>::const_iterator	j;
			for (j = circles.begin(); j != circles.end(); j++) {
				if (j->contains(p)) {
					sum += 1;
				}
			}
			image.pixel(x, y) = sum;
		}
	}
	astro::io::FITSoutfile<double>	outimage("image.fits");
	outimage.setPrecious(false);
	outimage.write(image);
	RadonAdapter	radon(ImageSize(1600,800), image);
	Image<double>	r(radon);
	astro::io::FITSoutfile<double>	out("radon.fits");
	out.setPrecious(false);
	out.write(r);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRadon() end");
}

void	RadonTest::testCircle() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCircle() begin");
	double	radius = 1000.5001;
	circle	c1(radius);
	double	length = c1.length();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "length = %f, exact = %f",
		length, 2 * M_PI * radius);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCircle() end");
}

void	RadonTest::testCircleTransform() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCircleTransform() begin");
	// create a circle object
	double	radius = 100;
	double	sigma = 10;
	circle	c(radius - 4);

	// create an image with a circle
	Image<double>	*image = new Image<double>(300, 300);
	ImagePtr	imageptr(image);

	// draw a circle of the same radius
	for (int x = 0; x < 300; x++) {
		for (int y = 0; y < 300; y++) {
			double	r = hypot(x - 150, y - 150) - radius;
			image->pixel(x, y) = 1000 * exp(-r * r / sigma);
		}
	}

	// save this image
	io::FITSout	out("tmp/circle.fits");
	out.setPrecious(false);
        out.write(imageptr);

	// apply the circle transform with that circle
	CircleAdapter	ca(*image, c);
	Image<double>	*ctransform = new Image<double>(ca);
	ImagePtr	ctransformptr(ctransform);

	// write the circle transform
	io::FITSout	outt("tmp/circletransform.fits");
	outt.setPrecious(false);
        outt.write(ctransformptr);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCircleTransform() end");
}

#if 0
void	RadonTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
