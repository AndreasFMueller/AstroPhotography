/*
 * HSLTest.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>
#include <AstroIO.h>
#include <AstroDebug.h>
#include <AstroPixel.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <math.h>

using namespace astro::image;

namespace astro {
namespace test {

class HSLTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testRGB();

	CPPUNIT_TEST_SUITE(HSLTest);
	CPPUNIT_TEST(testRGB);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HSLTest);

void	HSLTest::setUp() {
}

void	HSLTest::tearDown() {
}

void	HSLTest::testRGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB() begin");
	// red
	HSL<double>	red(0, 1, 0.5);
	CPPUNIT_ASSERT(fabs(red.r() - 1.00) < 0.01);
	CPPUNIT_ASSERT(fabs(red.g() - 0.00) < 0.01);
	CPPUNIT_ASSERT(fabs(red.b() - 0.00) < 0.01);

	// green
	HSL<double>	green(2 * M_PI / 3, 1, 0.25);
	CPPUNIT_ASSERT(fabs(green.r() - 0.00) < 0.01);
	CPPUNIT_ASSERT(fabs(green.g() - 0.50) < 0.01);
	CPPUNIT_ASSERT(fabs(green.b() - 0.00) < 0.01);

	// blue
	HSL<double>	blue(4 * M_PI / 3, 1, 0.75);
	CPPUNIT_ASSERT(fabs(blue.r() - 0.50) < 0.01);
	CPPUNIT_ASSERT(fabs(blue.g() - 0.50) < 0.01);
	CPPUNIT_ASSERT(fabs(blue.b() - 1.00) < 0.01);

	// yellow
	HSL<double>	yellow(M_PI * 56.9 / 180, 0.991, 0.765);
	CPPUNIT_ASSERT(fabs(yellow.r() - 0.998) < 0.01);
	CPPUNIT_ASSERT(fabs(yellow.g() - 0.974) < 0.01);
	CPPUNIT_ASSERT(fabs(yellow.b() - 0.532) < 0.01);

	// pink
	HSL<double>	pink(M_PI * 283.7 / 180, 0.775, 0.542);
	CPPUNIT_ASSERT(fabs(pink.r() - 0.704) < 0.01);
	CPPUNIT_ASSERT(fabs(pink.g() - 0.187) < 0.01);
	CPPUNIT_ASSERT(fabs(pink.b() - 0.897) < 0.01);

	// bright green
	HSL<double>	bg(M_PI * 162.4 / 180, 0.779, 0.447);
	CPPUNIT_ASSERT(fabs(bg.r() - 0.099) < 0.01);
	CPPUNIT_ASSERT(fabs(bg.g() - 0.795) < 0.01);
	CPPUNIT_ASSERT(fabs(bg.b() - 0.591) < 0.01);
	

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGB() end");
}


} // namespace test
} // namespace astro
