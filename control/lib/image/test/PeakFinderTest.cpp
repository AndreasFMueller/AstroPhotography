/*
 * PeakFinderTest.cpp -- find the peak in an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFilter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>
#include <math.h>

using namespace astro::image;
using namespace astro::image::filter;

namespace astro {
namespace test {

class PeakFinderTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testPeak();

	CPPUNIT_TEST_SUITE(PeakFinderTest);
	CPPUNIT_TEST(testPeak);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PeakFinderTest);

void	PeakFinderTest::setUp() {
}

void	PeakFinderTest::tearDown() {
}

void	PeakFinderTest::testPeak() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPeak() begin");
	Image<double>	image(200,100);
	Point	center(111.1,55.5);
	for (int x = 0; x < 200; x++) {
		for (int y = 0; y < 100; y++) {
			Point	p(x, y);
			double	r = (p - center).abs();
			image.pixel(x, y) = exp(-r * r / 2000);
		}
	}
	PeakFinder	pf(ImagePoint(0,0), 20);
	Point	peak = pf(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "peak at %s (should be %s): %f",
		peak.toString().c_str(), center.toString().c_str(), (peak - center).abs());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPeak() end");
}

} // namespace test
} // namespace astro
