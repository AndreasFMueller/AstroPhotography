/*
 * PhaseCorrelatorTest.cpp -- test adapters to subwindows
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <iostream>
#include <sstream>
#include <includes.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace test {

class PhaseCorrelatorTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testInteger();
	void	testIntegerNegative();
	void	testHalf();
	void	testImage();
	void	testDisks();

	CPPUNIT_TEST_SUITE(PhaseCorrelatorTest);
	CPPUNIT_TEST(testInteger);
	CPPUNIT_TEST(testIntegerNegative);
	CPPUNIT_TEST(testHalf);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST(testDisks);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhaseCorrelatorTest);

void	PhaseCorrelatorTest::setUp() {
}

void	PhaseCorrelatorTest::tearDown() {
}

void	PhaseCorrelatorTest::testInteger() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start Integer test");
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
			r = hypot(x - 63, y - 74);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage).first;

	// display result
	std::ostringstream	out;
	out << translation;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end Integer test");
}

void	PhaseCorrelatorTest::testIntegerNegative() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start IntegerNegative test");
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
			r = hypot(x - 57, y - 66);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage).first;

	// display result
	std::ostringstream	out;
	out << translation;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end IntegerNegative test");
}

void	PhaseCorrelatorTest::testHalf() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start Half test");
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * cos(r / 8) / r;
			}
			r = hypot(x - 62.5, y - 66.5);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * cos(r / 8) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage).first;

	// display result
	std::ostringstream	out;
	out << translation;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end Half test");
}

void	PhaseCorrelatorTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "beginImage test");

	FITSin	imagefile("testimages/test-image.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<unsigned char>	*image = dynamic_cast<Image<unsigned char> *>(&*imageptr);
	TypeReductionAdapter<double, unsigned char>	doubleimage(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	FITSin	chartfile("testimages/test-chart.fits");
	ImagePtr	chartptr = chartfile.read();
	Image<unsigned char>	*chart = dynamic_cast<Image<unsigned char> *>(&*chartptr);
	TypeReductionAdapter<double, unsigned char>	doublechart(*chart);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test chart read");

	// create a phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(doubleimage, doublechart);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
		result.first.toString().c_str(), result.second);

	// expected result: (-15,26)
	Point	target(-15, 26);
	Point	effective(round(result.first.x()), round(result.first.y()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s ?= %s",
		target.toString().c_str(), effective.toString().c_str());
	CPPUNIT_ASSERT(target == effective);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "endImage test");
}

void	PhaseCorrelatorTest::testDisks() {
	int	N = 64;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 32, y - 32);
			if (r < 10) {
				fromimage.pixel(x, y) = 100;
			} else if (r > 12) {
				fromimage.pixel(x, y) = 0;
			} else {
				fromimage.pixel(x, y) = 100 * (12 - r) / 2;
			}
			r = hypot(x - 40, y - 48);
			if (r < 10) {
				toimage.pixel(x, y) = 100;
			} else if (r > 12) {
				toimage.pixel(x, y) = 0;
			} else {
				toimage.pixel(x, y) = 100 * (12 - r) / 2;
			}
		}
	}
	DerivativeNormAdapter<double>	from(fromimage);
	DerivativeNormAdapter<double>	to(toimage);

	// create a differential phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(from, to);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
		result.first.toString().c_str(), result.second);
}

} // namespace test
} // namespace astro
