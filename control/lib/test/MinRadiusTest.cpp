/*
 * MinRadiusTest.cpp -- test minimum radius function
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

class MinRadiusTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testMin();

	CPPUNIT_TEST_SUITE(MinRadiusTest);
	CPPUNIT_TEST(testMin);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MinRadiusTest);

void	MinRadiusTest::setUp() {
}

void	MinRadiusTest::tearDown() {
}

void	MinRadiusTest::testMin() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() begin");
	std::list<ImagePoint>	points;
	points.push_back(ImagePoint(0, 0));
	points.push_back(ImagePoint(1, 0));
	points.push_back(ImagePoint(0, 1));
	points.push_back(ImagePoint(1, 1));
	double	radius = MinRadius(points);
	CPPUNIT_ASSERT(fabs(radius - sqrt(2)/2) < 0.0001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() end");
}

} // namespace test
} // namespace astro
