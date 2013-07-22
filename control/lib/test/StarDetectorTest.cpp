/*
 * StarDetectorTest.cpp -- test the StarDetector class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroGuiding.h>
#include <AstroIO.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <debug.h>
#include <iostream>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::guiding;
using namespace astro::io;

namespace astro {
namespace test {

class StarDetectorTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testDetector();

	CPPUNIT_TEST_SUITE(StarDetectorTest);
	CPPUNIT_TEST(testDetector);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StarDetectorTest);

void	StarDetectorTest::setUp() {
}

void	StarDetectorTest::tearDown() {
}

void	StarDetectorTest::testDetector() {
std::cerr << "StarDetector" << std::endl;
	Point	p(480.1, 230.7);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test detect star at %s",
		p.toString().c_str());
	Image<unsigned short>	*imagep = new Image<unsigned short>(640, 480);
	ImagePtr	imageptr(imagep);
	ImageSize	size = imagep->getSize();
	for (unsigned int x = 0; x < size.width; x++) {
		for (unsigned int y = 0; y < size.height; y++) {
			double	r = hypot(x - p.x, y - p.y);
			unsigned short	v = 1000 * exp(-(r * r) / 8);
			imagep->pixel(x, y) = v;
		}
	}
	unlink("star.fits");
	FITSout	out("star.fits");
	out.write(imageptr);
	StarDetector<unsigned short>	detector(*imagep);
	ImageRectangle	rectangle(ImagePoint(400,200), ImageSize(100, 100));
	Point	p2 = detector(rectangle, 8);
std::cerr << p2 << std::endl;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "detected at: %s",
		p2.toString().c_str());
}


} // namespace test
} // namespace astro
