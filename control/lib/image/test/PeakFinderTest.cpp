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
	void	testDoublePeak();
	void	testNoisePeak();

	CPPUNIT_TEST_SUITE(PeakFinderTest);
	CPPUNIT_TEST(testPeak);
	CPPUNIT_TEST(testDoublePeak);
	CPPUNIT_TEST(testNoisePeak);
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
	PeakFinder	pf(20);
	Point	peak = pf(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "peak at %s (should be %s): %f",
		peak.toString().c_str(), center.toString().c_str(),
		(peak - center).abs());
	CPPUNIT_ASSERT((peak - center).abs() < 0.5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPeak() end");
}

void	PeakFinderTest::testDoublePeak() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDoublePeak() begin");
	Image<double>	image(1000, 700);
	Point	c1(400.3,500.4);
	Point	c2(500.1,402.9);
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			Point	p(x, y);
			double	r1 = (p - c1).abs();
			double	r2 = (p - c2).abs();
			image.pixel(x, y) = 2 * exp(-r1 * r1 / 1000)
				+ 5 * exp(-r2 * r2 / 1000);;
		}
	}
	// first find the more pronounced peak
	PeakFinder	pf2(10);
	Point	peak2 = pf2(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s should be close to %s",
		peak2.toString().c_str(),
		c2.toString().c_str());
	CPPUNIT_ASSERT((peak2 - c2).abs() < 0.5);

	// now find the peak close to c1
	PeakFinder	pf1(ImagePoint(390, 510), 100);
	Point	peak1 = pf1(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s should be close to %s",
		peak1.toString().c_str(),
		c1.toString().c_str());
	CPPUNIT_ASSERT((peak1 - c1).abs() < 0.5);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDoublePeak() end");
}

void	PeakFinderTest::testNoisePeak() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNoisePeak() begin");
	int	w = 1024;
	Image<double>	image(2 * w, 2 * w);
	image.fill(0.);

	// where the peak is going to be
	int	x = (w/2) + random() % w;
	int	y = (w/2) + random() % w;
	Point	center(x, y);

	// fill the image
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			double	noise = 0.00001 * (random() % 1024);
			Point	p(x, y);
			double	r = (p - center).abs();
			image.pixel(x, y) = exp(-r * r / 10000) + noise;
		}
	}
	
	// find the peak
	PeakFinder	pf(100);
	Point	peak = pf(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s should be close to %s",
		peak.toString().c_str(), center.toString().c_str());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNoisePeak() end");
}

} // namespace test
} // namespace astro
