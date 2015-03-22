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

using namespace astro::image::radon;
using namespace astro::image;

namespace astro {
namespace test {

class RadonTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testCircle();
	void	testCircleTransform();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(RadonTest);
	CPPUNIT_TEST(testCircle);
	CPPUNIT_TEST(testCircleTransform);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RadonTest);

void	RadonTest::setUp() {
}

void	RadonTest::tearDown() {
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
	circle	c(radius);

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
	io::FITSout	out("circle.fits");
	out.setPrecious(false);
        out.write(imageptr);

	// apply the circle transform with that circle
	CircleAdapter	ca(*image, c);
	Image<double>	*ctransform = new Image<double>(ca);
	ImagePtr	ctransformptr(ctransform);

	// write the circle transform
	io::FITSout	outt("circletransform.fits");
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
